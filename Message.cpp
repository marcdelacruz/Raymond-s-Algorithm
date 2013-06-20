/*
 *  Message.cpp
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#include "Message.h"

int Message::smSrcId = -1;

void Message::setSrcId(int srcId)
{
    smSrcId = srcId;
}

Message::Message(MessageTag& rMsg) 
{ 
    mMessage = rMsg; 
}

Message::Message(MsgType msgType, short destNodeId) 
{
    //this is for msgs w/o a msg body
    memset(&mMessage, 0, MAX_MSG_SIZE);
    setMsgType(msgType); 
    setDestNodeId(destNodeId);
    mMessage.hdr.payloadSize = 0;
    mMessage.hdr.senderId = smSrcId;
}

Message::Message() 
{
    memset(&mMessage, 0, MAX_MSG_SIZE); 
}

Message::~Message()
{
}

void Message::createNodeIdMsg(short nodeId)
{
    memset(&mMessage, 0, MAX_MSG_SIZE);
    setMsgType(NODE_ID); 
    mMessage.nodeIdMsg.nodeId = nodeId;
    mMessage.hdr.payloadSize = sizeof(mMessage.nodeIdMsg);
    mMessage.hdr.senderId = smSrcId;
}

short Message::getNodeId() const
{
    return mMessage.nodeIdMsg.nodeId;
}

MsgType Message::getMsgType() const 
{
    return (MsgType)mMessage.hdr.msgType;
}

short Message::getDestNodeId() const
{
    return mMessage.hdr.destNodeId;
}

short Message::getSize() const
{
    return (mMessage.hdr.payloadSize + sizeof(mMessage.hdr)); 
}

const MessageTag& Message::getMsg() 
{
    return mMessage; 
}

short Message::getPayloadSize() const
{
    return mMessage.hdr.payloadSize;
}

void Message::setMsgType(MsgType msgType)
{
    mMessage.hdr.msgType = msgType; 
}

void Message::setDestNodeId(short nodeId)
{
    mMessage.hdr.destNodeId = nodeId; 
}

void Message::setSrcNode(int nodeId)
{
    mMessage.hdr.senderId = nodeId;
}
int Message::getSrcNode()
{
    return mMessage.hdr.senderId;
}
