#pragma once
#include <omnetpp.h>
#include "inet/common/geometry/common/Coord.h"
#include <mutex>
#include "json.hpp"

using namespace omnetpp;
using namespace nlohmann;

class UnityClient : public cSimpleModule{

private:
    std::mutex m_mutex;
    static int serverSocket;
    static UnityClient* instance;

protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage*msg) override;
    virtual void finish() override;

public:
    void sendMessage(const std::string& id,const std::string& path,const std::string& instruction,inet::Coord coord);
    UnityClient(){}
    static UnityClient* getInstance() {
        if (instance == nullptr) {
            instance = new UnityClient();
           // Create the instance if it doesn't exist
        }
        static UnityClient instance;  // Static instance of UnityClient
        return &instance;
    }
};


