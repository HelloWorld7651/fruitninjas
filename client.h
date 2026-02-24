//
// Client.h 
//

#ifndef CLIENT_H
#define CLIENT_H

// System includes.
#include <string>

// Engine includes.
#include "EventNetwork.h"
#include "Object.h"
#include "NetworkManager.h"

namespace df {
    class EventNetwork;
}

class Client : public df::Object {

 private:
  // Handle data.
  int handleData(const df::EventNetwork *p_e);

 public:
  // Constructor, connecting to server.
  Client(std::string server_name);

  // Handle events
  int eventHandler(const df::Event *p_e) override;
};

#endif