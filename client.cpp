//
// client.cpp
//

// System includes.
#include <string.h>

// Engine includes.
#include "EventNetwork.h"
#include "GameManager.h"
#include "LogManager.h"
#include "NetworkManager.h"
#include "utility.h"
#include "NetMessages.h"

// Game includes.
#include "game.h"
#include "client.h"


// Constructor, connecting to server.
Client::Client(std::string server_name) {
    // Set as network client.
    setType("Client");
    NM.setServer(false);

    // Register for network events.
    registerInterest(df::NETWORK_EVENT);

    // Connect.
    std::string server_port = df::DRAGONFLY_PORT;
    LM.writeLog("Client::Client(): Connecting to %s at port %s.",
	    server_name.c_str(), server_port.c_str());
    if (NM.connect(server_name, server_port) < 0) {
        LM.writeLog("Client::Client(): Error! Unable to connect.");
        exit(-1);
    }
    LM.writeLog("Client::Client(): Client started.");
}
// Handle event.
int Client::eventHandler(const df::Event *p_e) {
    if (p_e->getType() == df::NETWORK_EVENT) {
        const df::EventNetwork *p_ne = (const df::EventNetwork *) p_e;
        if(p_ne->getLabel() == df::NetworkEventLabel::CONNECT){
            LM.writeLog("Client::eventHandler(): accepted connection (total %d)",
		    NM.getNumConnections());
            return 1;
        }
        if(p_ne->getLabel() == df::NetworkEventLabel::CLOSE){
            LM.writeLog("Client::eventHandler(): closed connection");
            GM.setGameOver();
            return 1;
        }
        if(p_ne->getLabel() == df::NetworkEventLabel::DATA){
            return handleData(p_ne);
        }
    }
    return 0;
}

int Client::handleData(const df::EventNetwork *p_en) {\
//read header
    int msg_size = p_en->getBytes();
    char *buff = (char *) malloc(msg_size);

    memcpy(buff, p_en->getMessage(), msg_size);

    NetHeader header;
    memcpy(&header, buff, sizeof(NetHeader));

    //switch message based on type
    switch(header.type) {
        case MessageType::SYNC_OBJECT:
            
        case MessageType::DELETE_OBJECT:
            
        case MessageType::GAME_OVER:
            
    } 
    free(buff);
    return 1;
}