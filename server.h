//
// server.h
//

#ifndef SERVER_H
#define SERVER_H

#include "Object.h"
#include "EventNetwork.h"
#include "NetworkManager.h"

#include <unordered_map>

namespace df {
    class EventNetwork;
}

class Server : public df::Object{
    std::unordered_map<int,int> m_sock_to_sword;
private:
    //handle data
    int handleData(const df:: EventNetwork *p_en);
public:
    //handle event
    Server();
    int eventHandler(const df::Event *p_e) override;
};
#endif