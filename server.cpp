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
#include "EventStep.h"
#include "EventNetwork.h"
#include "Event.h"
#include "Grocer.h"

// Game includes.
#include "game.h"
#include "Sword.h"
#include "server.h"

Server::Server() {
    // Set as network server.
    df::NetworkManager::getInstance().setServer(true);
    NM.setServer(true);

    // Register for network events.
    registerInterest(df::NETWORK_EVENT);
    // register so we can sync frames
    registerInterest(df::STEP_EVENT);

    // spawn grocer
    new Grocer();
  
    LM.writeLog("Server::Server(): Server started.");
}

int Server::eventHandler(const df::Event *p_e) {
    if(p_e->getType()==df::NETWORK_EVENT){
        const df::EventNetwork *p_ne = (const df::EventNetwork *) p_e;
        //for new clients
        if(p_ne->getLabel() == df::NetworkEventLabel::ACCEPT){
            LM.writeLog("Server::eventHandler(): accepted connection (total %d)", NM.getNumConnections());
            int socket_index = p_ne->getSocket();
            Sword *p_sword = new Sword;
            p_sword->setId(100+socket_index);
            return 1;
        }
        //if new client disconnected
        if(p_ne->getLabel() == df::NetworkEventLabel::CLOSE){
            LM.writeLog("Server::eventHandler(): closed connection");
            GM.setGameOver();
            return 1;
        }
        //for client sent data
        if(p_ne->getLabel() == df::NetworkEventLabel::DATA){
            return handleData(p_ne);
        }
    }
    
//sync objects, swords for example, send sword info to all clients
    if(p_e->getType() == df::STEP_EVENT){
        df::ObjectList all_objects = WM.getAllObjects();
        df::ObjectListIterator list_object(&all_objects);
        
        for(list_object.first(); !list_object.isDone(); list_object.next()){
            df::Object *p_o = list_object.currentObject();
            
            // Check for type, specifically the exact fruit sprite names
            std::string t = p_o->getType();
            if(t == "Sword" || t == "bomb" || t == "Bomb" || t == "pear" || 
               t == "grapes" || t == "apple" || t == "banana" || 
               t == "blueberries" || t == "watermelon") {
                
                // BUG FIX: Stop TCP Flooding!
                // Only send a sync packet if the object is NEW or if a Sword moved.
                bool needs_sync = false;
                
                // isModified(ID) is true ONLY on the exact frame the object is spawned
                if (p_o->isModified(df::ObjectAttribute::ID)) {
                    needs_sync = true;
                }
                // isModified(POSITION) tracks if the Sword actually moved this frame
                else if (t == "Sword" && p_o->isModified(df::ObjectAttribute::POSITION)) {
                    needs_sync = true;
                }

                // Only do the heavy serialization and networking if something changed
                if (needs_sync) {
                    std::stringstream ss;
                    unsigned int mask = UINT_MAX;
                    
                    // serialize type for the spawning
                    ss << t << " ";
                    
                    std::string serialize_data; 
                    p_o->serialize(&ss, mask);
                    serialize_data = ss.str();
                    
                    int msg_size = sizeof(NetSyncObject) + serialize_data.length();
                    char *buff = (char*)malloc(msg_size);
                    
                    NetSyncObject msg;
                    msg.header.size = msg_size;
                    msg.header.type = MessageType::SYNC_OBJECT;
                    msg.id = p_o->getId();
                    msg.object_type_len = t.length();  

                    //pack struct
                    memcpy(buff, &msg, sizeof(NetSyncObject));
                    memcpy(buff+sizeof(NetSyncObject), serialize_data.c_str(), serialize_data.length());

                    // Broadcast to all clients
                    for (int i = 0; i < 5; i++) {
                        NM.send(buff, msg_size, i);
                    }
                    
                    free(buff);
                }
            }
        }
        return 1;
    }
    return 0;
}
int Server::handleData(const df::EventNetwork *p_en) {
    int msg_size = p_en->getBytes();
    const char *buff = (const char *) p_en->getMessage();

    int offset = 0;
    // Loop through the buffer until all bytes are processed
    while (offset < msg_size) {
        NetHeader header;
        memcpy(&header, buff + offset, sizeof(NetHeader));

        // Safety check to prevent reading past the buffer
        if (offset + header.size > msg_size) {
            break; 
        }

        // Pointer to the start of the current message
        const char *msg_buff = buff + offset;

        // Switch message based on type
        switch(header.type) {
            case MessageType::MOUSE_MOVEMENT: {
                NetMouseMovement msg;
                memcpy(&msg, msg_buff, sizeof(NetMouseMovement));

                // Identify which client moved their mouse
                int client_socket = p_en->getSocket();
                int sword_id = 100 + client_socket;

                // Update sword for client
                df::Object *p_o = WM.objectWithId(sword_id);
                if (p_o != NULL) {
                    p_o->setPosition(df::Vector(msg.mouse_x, msg.mouse_y));
                }
                break;
            }

            case MessageType::EXIT: {
                // If you need to handle client disconnects explicitly here, do so.
                // Otherwise, the NetworkEventLabel::CLOSE handles the drop.
                break;
            }
            default:
                break;    
        } 
        
        // Move offset forward by the size of the current message
        offset += header.size;
    }
    
    return 1;
}