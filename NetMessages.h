//
// NetMessages.h
//

#ifndef NET_MESSAGES_H
#define NET_MESSAGES_H

//all possible type of messages 
enum class MessageType {
    SYNC_OBJECT,
    DELETE_OBJECT,
    MOUSE_MOVEMENT,
    GAME_OVER,
    EXIT
};
//front of all messages
struct NetHeader {
    int size;           
    MessageType type;   
};
//players mouse coordinates to send to server
struct NetMouseMovement {
    NetHeader header;
    float mouse_x;
    float mouse_y;
};
//tells client to delete object
struct NetDeleteObject {
    NetHeader header;
    int id;
};
//send data to update object on client
struct NetSyncObject {
    NetHeader header;
    int id;
    int object_type_len; 
};
#endif