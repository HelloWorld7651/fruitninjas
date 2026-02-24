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

// Game includes.
#include "game.h"
#include "Sword.h"
#include "server.h"

Server::Server() {

  // Set as network server.
  setType("Server");
  NM.setServer(true);
  //NM.setMaxConnections(5);

  // Register for network events.
  registerInterest(df::NETWORK_EVENT);

    //register so we can sync frames
  registerInterest(df::STEP_EVENT);
  
  LM.writeLog("Server::Server(): Server started.");
}

// Handle event.
// when player join or leave
int Server::eventHandler(const df::Event *p_e) {
    if(p_e->getType()==df::NETWORK_EVENT){
        const df::EventNetwork *p_ne = (const df::EventNetwork *) p_e;
        //for new clients
        if(p_ne->getLabel() == df::NetworkEventLabel::ACCEPT){
            LM.writeLog("Server::eventHandler(): accepted connection (total %d)",
		    NM.getNumConnections());
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
        //finding all objects
        df::ObjectList all_objects = WM.getAllObjects();
        //storing in iterator
        df::ObjectListIterator list_object(&all_objects);
        //loop to get total amount of objects
        std::stringstream ss;
        unsigned int mask = UINT_MAX;
        for(list_object.first(); !list_object.isDone(); list_object.next()){
            df:: Object *p_o = list_object.currentObject();
            //if it is modified and is sword
            if(p_o->isModified() && p_o -> getType() == "Sword"){
                //turn to string
                std::string serialize_data; 
                p_o->serialize(&ss, mask);
                serialize_data = ss.str();
                //size of object
                int msg_size = sizeof(NetSyncObject) + serialize_data.length();
                char *buff = (char*)malloc(msg_size);
                //NetSync header
                NetSyncObject msg;
                msg.header.size = msg_size;
                msg.header.type = MessageType::SYNC_OBJECT;
                msg.id = p_o->getId();
                msg.object_type_len = p_o->getType().length();  

                //pack struct
                memcpy(buff, &msg, sizeof(NetSyncObject));
                memcpy(buff+sizeof(NetSyncObject), serialize_data.c_str(), serialize_data.length());

                //send to all clients
                NM.send(buff, msg_size, -1);
                free(buff);
            }
        }
        return 1;
    }
    return 0;
}

int Server::handleData(const df::EventNetwork *p_en) {
    //read header
    int msg_size = p_en->getBytes();
    char *buff = (char *) malloc(msg_size);
    memcpy(buff, p_en->getMessage(), msg_size);

    NetHeader header;
    memcpy(&header, buff, sizeof(NetHeader));

    //switch message based on type
    switch(header.type) {
        case MessageType::MOUSE_MOVEMENT:
        //read mouse
            NetMouseMovement msg;
            memcpy(&msg, buff, sizeof(NetMouseMovement));

            //sword for client
            int client_socket = p_en->getSocket();
            int sword_id = 100+client_socket;

            //update sword for client
            df::Object *p_o = WM.objectWithId(sword_id);
            p_o->setPosition(df::Vector(msg.mouse_x, msg.mouse_y));
            break;

        case MessageType::EXIT:
            break;
            
    } 
    free(buff);
    return 1;
}