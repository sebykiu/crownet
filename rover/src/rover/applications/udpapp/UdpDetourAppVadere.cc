/*
 * UdpDetourAppVadere.cpp
 *
 *  Created on: Feb 12, 2020
 *      Author: sts
 */

#include "UdpDetourAppVadere.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "rover/applications/udpapp/DetourAppPacket_m.h"

using namespace inet;
using omnetpp::cStringTokenizer;

namespace rover {
Define_Module(UdpDetourAppVadere);

UdpDetourAppVadere::UdpDetourAppVadere()
    : mobility(nullptr), traci(nullptr) {}

UdpDetourAppVadere::~UdpDetourAppVadere() { }

void UdpDetourAppVadere::initialize(int stage) {
  UdpDetourApp::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {
    // nothing here
  } else if (stage == INITSTAGE_APPLICATION_LAYER) {
    // traci
    mobility =
        veins::VeinsInetMobilityAccess().get<veins::VaderePersonMobility*>(
            getParentModule());
    traci = mobility->getCommandInterface();
  }
}

void UdpDetourAppVadere::actOnIncident(
    IntrusivePtr<const DetourAppPacket> pkt) {

  veins::VaderePersonItfc* traciPerson = mobility->getPersonCommandInterface();
  std::string blocked = std::string(pkt->getClosedTarget());
  std::vector<std::string> targetLists = traciPerson->getTargetList();
  if (std::find(targetLists.begin(), targetLists.end(), blocked) !=
      targetLists.end()) {
    // blocked target found on traget list.
    std::vector<std::string> newTargetlist{};
    for (int i = 0; i < pkt->getAlternativeRouteArraySize(); i++) {
      newTargetlist.push_back(pkt->getAlternativeRoute(i));
    }
    traciPerson->setTargetList(newTargetlist);
  }
}

} /* namespace rover */