/*
 *  ClientSocket.h
 * Marc DelaCruz
 * CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include "ConnSocket.h"

class ClientSocket : public ConnSocket
{
public:
    ClientSocket();
    ClientSocket(const char* pServerAddr, unsigned short serverPortNumber);
    virtual ~ClientSocket();
    virtual void setFd(int fd) {}
    virtual void setServerAddr(const char* pServerAddr);
    virtual void setServerPortNumber(unsigned short serverPortNumber);
    virtual void open();
    virtual void write();
    virtual bool hasBytesToWrite() const;
    virtual void setSelectable(bool shouldBeSelectable);
    virtual void setOpenCallBack(CallBack<int>* pOpenCallBack);
    virtual bool isClientSock() const { return true; }

protected:
    void deleteOpenCallBack();
    
    string mServerAddrStr;
    unsigned short mServerPortNumber;
    CallBack<int>* mpOpenCallBack;
    int mIsConnectPending;
};

#endif //CLIENT_SOCKET_H
