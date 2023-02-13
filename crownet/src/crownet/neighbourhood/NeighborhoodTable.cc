/*
 * NeighborhoodTable.cc
 *
 *  Created on: Jan 23, 2021
 *      Author: sts
 */

#include "crownet/neighbourhood/NeighborhoodTable.h"

#include <omnetpp/simtime_t.h>
#include <omnetpp/cstlwatch.h>
#include <omnetpp/cwatch.h>
#include "crownet/crownet.h"
#include "crownet/common/GlobalDensityMap.h"
#include "crownet/common/util/FileWriterRegister.h"
#include "inet/common/ModuleAccess.h"
#include "crownet/common/IDensityMapHandler.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "crownet/crownet.h"

using namespace omnetpp;
using namespace inet;

namespace
{

const simsignal_t neighborhoodTableChangedSignal = cComponent::registerSignal("neighborhoodTableChanged");

}

namespace crownet {

Define_Module(NeighborhoodTable);

NeighborhoodTable::~NeighborhoodTable(){
    if (ttl_msg != nullptr)
        cancelAndDelete(ttl_msg);
}

// cSimpleModule
void NeighborhoodTable::initialize(int stage){
    MobilityProviderMixin<cSimpleModule>::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        maxAge = par("maxAge");
        ttl_msg = new cMessage("NeighborhoodTable_ttl");
        scheduleAt(simTime() + maxAge, ttl_msg);
        WATCH_PTRMAP(_table);
        WATCH(maxAge);
        WATCH(tableSize);
    } else if (stage == INITSTAGE_APPLICATION_LAYER){
        cStringTokenizer t(par("fileWriterRegister").stringValue(), ";");
        auto t_value = t.asVector();
        if (t_value.size() == 2){
            auto fRegister = dynamic_cast<IFileWriterRegister*>(findModuleByPath(t_value[0].c_str()));
            if (fRegister && fRegister->hasWriter(t_value[1])) {
                auto l = fRegister->getWriterAs<NeighborhoodEntryListner>(t_value[1]);
                registerEntryListner(l);
            }
        }
        auto converter = inet::getModuleFromPar<OsgCoordConverterProvider>(
                        par("coordConverterModule"), this)
                        ->getConverter();
        cellKeyProvider = std::make_shared<GridCellIDKeyProvider>(converter);
    }
}

void NeighborhoodTable::handleMessage(cMessage *msg){
    if (msg == ttl_msg){
        checkAllTimeToLive();
        scheduleAt(simTime() + maxAge, ttl_msg);
    } else {
        throw cRuntimeError("Unknown Message received");
    }
}

void NeighborhoodTable::saveInfo(BeaconReceptionInfo* info) {
    if (_table.find(info->getNodeId()) == _table.end()){
        // node info object from this source found. Add to table
        _table[info->getNodeId()] = info;
        tableSize = _table.size();
        setLastUpdatedAt(simTime());
        emit(neighborhoodTableChangedSignal, this);
    }
    // if key exist do nothing. Info objects is the same, the get reused.
    // todo test this?
}

const BeaconReceptionInfo* NeighborhoodTable::find(int sourceId) const {
    auto it = _table.find(sourceId);
    if (it == _table.end()){
        return nullptr;
    }
    return it->second;
}

void NeighborhoodTable::removeInfo(BeaconReceptionInfo* info){
    auto it = _table.find(info->getNodeId());
    if (it != _table.end()){
        emitRemoved(it->second);
        it->second->clearApplicationData();
        it = _table.erase(it);
        //only erase, do not delete object is managed elsewere.
        setLastUpdatedAt(simTime());
        emit(neighborhoodTableChangedSignal, this);
    } else {
        // do nothing object not in map
    }
}

bool NeighborhoodTable::processInfo(BeaconReceptionInfo *info){
    /*
     * Case1 New info object (never seen node) ttl not reached: post change will increment map
     * Case2 New info object (never seen node) ttl reached: drop will never change map (count or timestamp)
     *       There was no preChange thus the map was never touched during the processing of this beacon info.
     * Case3 Old info object (already seen node, preChange was called getOrCreate) ttl not reached: post change
     *       will increment map at new location. If it was in the same cell the -1 / +1 will cancel out put
     *       timestamp is updated
     * Case4 Old info object (already seen node, preChange was called getOrCreate) ttl reached:
     *       info will be dropped. No call to map. The preChange already decrement the map no increment necessary.
     */
    //ensure info object is in table.
    Enter_Method_Silent();

    EV_INFO << "processInfo[id: " << ownerId << "] " << info->logShort() << endl;
    if (!info->isUpdated()){
        // out of order packet do not process again
        return true;
    }
    // only save correctly ordered info objects into neighborhood table
    saveInfo(info);

    if (ttlReached(info)){
        // information to old do not propagate to density map
        if (info->hasPrio()){
            // received packet is to old at reception but a previous packet still exists.
            // Because packets are created in a strict order the previous packet must have
            // reached the ttl also. Map cleanup needed because previous packet exists.
            emitRemoved(info);
        } else {
            // no previous packet exits just drop it. No map cleanup needed.
            emitDropped(info);
        }
        removeInfo(info);
    } else {
        // new info, already seen and cell change or already seen and same cell
        if (!info->hasPrio()){
            // new beacon. Node was not seen before or last beacon was sent more than TTL seconds ago.
            // (1) enter cell
            emitEnterCell(info);
        } else if (cellKeyProvider->changedCell(
                info->getCurrentData()->getPosition(),
                info->getPrioData()->getPosition()))
        {
            // node moved! (1) decrement in old cell and (2) increment in new cell
            // (1) leave old cell
            emitLeaveCell(info);
            // (2) enter new cell
            emitEnterCell(info);
        } else {
            // node staed in cell! Only update time stampes.
            // (1) stay in cell (only time stamp update)
            emitStayInCell(info);
        }
    }
    return true;
}

bool NeighborhoodTable::ttlReached(BeaconReceptionInfo* info){
    return info->checkCurrentTtlReached(maxAge);
}

void NeighborhoodTable::checkAllTimeToLive(){
    Enter_Method_Silent();

    simtime_t now = simTime();
    // remove old entries
    for( auto it=_table.cbegin(); it !=_table.cend();){
        // Received + maxAge := time at which entry must be removed.
        if (ttlReached(it->second)){
            emitRemoved(it->second);
            it->second->clearApplicationData();
            // not manged by NT. just clear application data and remove not delete
            it = _table.erase(it);
            setLastUpdatedAt(now);
        } else {
            ++it;
        }
    }
    lastCheck = now;
    tableSize = _table.size();
    emit(neighborhoodTableChangedSignal, this);
}

const int NeighborhoodTable::getSize(){
    Enter_Method_Silent();
    checkAllTimeToLive();
    return _table.size();
}


Register_ResultFilter("tableSize", NeighborhoodTableSizeFilter);
void NeighborhoodTableSizeFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                            cObject *object, cObject *details) {
    if (auto table = dynamic_cast<NeighborhoodTable*>(object)){
        fire(this, t, (double)table->_table.size(), details);
    }
};


} /* namespace crownet */
