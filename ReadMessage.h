/*
 *  ReadMessage.h
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */
#ifndef READ_MESSAGE_H
#define READ_MESSAGE_H

#include "MessageDefs.h"

class ReadMessage
{
public:
    ReadMessage(int nodeId, MessageTag& rMessage) : 
                mNodeId(nodeId), mrReadMessage(rMessage)
    {
    }

    ~ReadMessage() {}
    int getNodeId() const { return mNodeId; }
    MessageTag& getMessage() const { return mrReadMessage; }
    
protected:
    int mNodeId;
    MessageTag& mrReadMessage;
};

#endif //READ_MESSAGE_H

