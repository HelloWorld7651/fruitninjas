//
// Server.cpp
//

// System includes.
#include <string.h>

// Engine includes.
#include "LogManager.h"
#include "GameManager.h"
#include "NetworkManager.h"
#include "WorldManager.h"
#include "utility.h"
#include "NetMessages.h"

// Game includes.
#include "game.h"
#include "server.h"

Server::Server() {

  // Set as network server.
  setType("Server");
  NM.setServer(true);
  NM.setMaxConnections(5);

  // Register for network events.
  registerInterest(df::NETWORK_EVENT);
  
  LM.writeLog("Server::Server(): Server started.");
}

// Handle event.
// when player join or leave
int Server::eventHandler(const df::Event *p_e) {
    if(p_e->getType()==df::NETWORK_EVENT){
        const df::EventNetwork *p_ne = (const df::EventNetwork *) p_e;
        if(p_ne->getLabel() == df::NetworkEventLabel::ACCEPT){
            LM.writeLog("Server::eventHandler(): accepted connection (total %d)",
		    NM.getNumConnections());
            return 1;
        }
        if(p_ne->getLabel() == df::NetworkEventLabel::CLOSE){
            LM.writeLog("Server::eventHandler(): closed connection");
            GM.setGameOver();
            return 1;
        }
        if(p_ne->getLabel() == df::NetworkEventLabel::DATA){
            return handleData(p_ne);
        }

    }
    return 0;
}

int Server::handleData(const df::EventNetwork *p_en) {\
//read header
    int msg_size = p_en->getBytes();
    char *buff = (char *) malloc(msg_size);

    memcpy(buff, p_en->getMessage(), msg_size);

    NetHeader header;
    memcpy(&header, buff, sizeof(NetHeader));

    //switch message based on type
    switch(header.type) {
        case MessageType::MOUSE_MOVEMENT:

        case MessageType::EXIT:
            
    } 
    free(buff);
    return 1;
}