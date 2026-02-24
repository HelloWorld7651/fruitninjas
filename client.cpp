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
#include "Event.h"
#include "WorldManager.h"
#include "Sword.h"
#include "Fruit.h"
#include "bomb.h"
#include "EventMouse.h"

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
    // Register for mouse events
    registerInterest(df::MSE_EVENT);

    // Connect.
    std::string server_port = df::DRAGONFLY_PORT;
    LM.writeLog("Client::Client(): Connecting to %s at port %s.", server_name.c_str(), server_port.c_str());
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
            LM.writeLog("Client::eventHandler(): accepted connection (total %d)", NM.getNumConnections());
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
    
    //checks for mouse event
    if (p_e->getType() == df::MSE_EVENT) {
        const df::EventMouse *p_mouse_event = dynamic_cast <const df::EventMouse *> (p_e);
        if (p_mouse_event->getMouseAction() == df::MOVED) {
            // Send mouse request to server
            NetMouseMovement msg;
            msg.header.size = sizeof(NetMouseMovement);
            msg.header.type = MessageType::MOUSE_MOVEMENT;
            msg.mouse_x = p_mouse_event->getMousePosition().getX();
            msg.mouse_y = p_mouse_event->getMousePosition().getY();
            
            NM.send(&msg, sizeof(NetMouseMovement),-1);
            return 1;
        }
    }
    return 0;
}int Client::handleData(const df::EventNetwork *p_en) {
    int msg_size = p_en->getBytes();
    const char *buff = (const char *) p_en->getMessage();

    int offset = 0;
    while (offset < msg_size) {
        if (offset + sizeof(NetHeader) > msg_size) break; 

        NetHeader header;
        memcpy(&header, buff + offset, sizeof(NetHeader));

        if (header.size <= 0 || offset + header.size > msg_size) break; 

        const char *msg_buff = buff + offset;

        if (header.type == MessageType::SYNC_OBJECT) {
            NetSyncObject msg;
            memcpy(&msg, msg_buff, sizeof(NetSyncObject));

            // 1. Extract the type string cleanly from memory
            std::string type_string(msg_buff + sizeof(NetSyncObject), msg.object_type_len);
            
            // 2. Extract the uncorrupted serialization data
            int data_len = header.size - sizeof(NetSyncObject) - msg.object_type_len;
            if (data_len > 0) {
                std::string serialize_data(msg_buff + sizeof(NetSyncObject) + msg.object_type_len, data_len);
                std::stringstream ss(serialize_data);

                int id = msg.id;

                df::Object *p_o = WM.objectWithId(id);
                if(p_o == NULL) {
                    if(type_string == "Sword") {
                        p_o = new Sword();
                    } else if(type_string == "bomb" || type_string == "Bomb") {
                        p_o = new Bomb();
                    } else if(type_string == "pear" || type_string == "grapes" || 
                              type_string == "apple" || type_string == "banana" || 
                              type_string == "blueberries" || type_string == "watermelon") {
                        p_o = new Fruit(type_string);
                    }
                    
                    if (p_o != NULL) p_o->setId(id);
                }
                
                // 3. Deserialize the pristine stream!
                if (p_o != NULL) p_o->deserialize(&ss);
            }
        } 
        else if (header.type == MessageType::DELETE_OBJECT) {
            NetDeleteObject msg;
            memcpy(&msg, msg_buff, sizeof(NetDeleteObject));
            df::Object *p_o = WM.objectWithId(msg.id);
            if (p_o != NULL) {
                WM.markForDelete(p_o);
            }
        }
        
        offset += header.size;
    } 
    return 1;
}