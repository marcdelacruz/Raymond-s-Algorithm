/*
 *  MessageDefs.h
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */

//define message structures
#ifndef MESSAGE_DEFS_H
#define MESSAGE_DEFS_H
 
#include <string>
#include <cstdlib>

using namespace std;

struct Header
{
    unsigned char msgType;
    unsigned char destNodeId;
    //int timestamp;
    //int msgId;
    int senderId;
    short payloadSize;
};

#define HDR_SIZE           sizeof(Header)
#define MAX_PAYLOAD_SIZE   1024//2048

enum MsgType
{
    MIN_MSG_TYPE         = 70,
    NODE_ID              = MIN_MSG_TYPE, //denotes the client's unique ID is
    TOKEN_REQUEST        = 71,
    TOKEN_MSG            = 72,
    MAX_MSG_TYPE_EX,
    MAX_MSG_TYPE         = MAX_MSG_TYPE_EX-1
};

struct NodeIdTag
{
    unsigned char nodeId;
};

struct MessageTag
{
    Header hdr;
    union
    {
        NodeIdTag nodeIdMsg;
        char textMsg[MAX_PAYLOAD_SIZE];
    };
};

#define MAX_MSG_SIZE sizeof(Message)


#endif // MESSAGE_DEFS_H


