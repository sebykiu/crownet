#include "UnityNetworkVisualizer.h"

namespace crownet {



Define_Module(UnityNetworkVisualizer);



void UnityNetworkVisualizer::initialize(int stage){

    LinkVisualizerBase::initialize(stage);
    printf("Hello Network Visualizer!");

    if (stage == inet::INITSTAGE_LOCAL){
        unityClient = UnityClient::getInstance();

    }

}



bool UnityNetworkVisualizer::isLinkStart(cModule *module) const
{
    return false;
}

bool UnityNetworkVisualizer::isLinkEnd(cModule *module) const
{
    return false;
}

const UnityNetworkVisualizer::LinkVisualizerBase::LinkVisualization *UnityNetworkVisualizer::createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const
{
}

void UnityNetworkVisualizer::setAlpha(const LinkVisualizerBase::LinkVisualization *linkVisualization, double alpha) const
{
}

void UnityNetworkVisualizer::subscribe() {
    LinkVisualizerBase::subscribe();
}

void UnityNetworkVisualizer::unsubscribe(){
    LinkVisualizerBase::unsubscribe();
}

void UnityNetworkVisualizer::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details){
    //LinkVisualizerBase::receiveSignal(source, signal, object, details);

std::cout << signal;

    // this doesn't support arrows, as we loose the information of who is connected?


//    if(LinkVisualizerBase::isLinkEnd(static_cast<cModule*> (source)))){
//        auto module = check_and_cast<cModule *>(source);
//        // send to unity that is receiver
//
//    }
//    else if (LinkVisualizerBase::isLinkStart(static_cast<cModule*> (source)))){
//        auto module = check_and_cast<cModule *>(source);
//        // send to unity that is sender
//    }


}


} // namespace inet
