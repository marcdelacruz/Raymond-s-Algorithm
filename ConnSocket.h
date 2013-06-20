/*
 *  ConnSocket.h
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */
#ifndef CONN_SOCKET_H
#define CONN_SOCKET_H

#include "Fd.h"
#include "ReadMessage.h"

class ConnSocket : public Fd
{
public:
    ConnSocket(int maxBytesToRead, bool isSelectable, 
                 bool isReadable);
    ConnSocket(int fd, int maxBytesToRead, bool isSelectable, 
                 bool isReadable);

    virtual ~ConnSocket();
    virtual void open() {} //nothing to do
    virtual void setFd(int fd);
    virtual void read();
    virtual void close();
    virtual void resetReadVariables();
    virtual bool hasBytesToWrite() const;
    virtual void write();
    virtual void clearMsgQ();
    virtual int writeMsg(Message& rMessage);

    virtual void setReadCallBack(CallBack<ReadMessage* >* pReadCallBack);

    virtual void setCloseCallBack(CallBack<int>* pCloseCallBack);

    virtual void setCloseAfterLastWrite(bool shouldClose);
    virtual void setSendKeepAlive(bool shouldSendKeepAlive);

protected:
    virtual void deleteReadCallBack();
    virtual void deleteCloseCallBack();
    
    CallBack<int>* mpCloseCallBack;
    MessageTag mMsgBuffer;
    int mNumBytesRead;
    CallBack<ReadMessage* >* mpReadCallBack;

    std::deque<MessageTag*> mMsgQ;
    int mWriteMsgPos;
    bool mShouldCloseAfterWrite;
};

#endif //CONN_SOCKET_H
