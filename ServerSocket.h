/*
 *  ServerSocket.h
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */
#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "Fd.h"
#include "CallBack.h"
#include <iostream>

class ServerSocket : public Fd
{
public:
    ServerSocket();
    ServerSocket(unsigned short portNumber);
    
    virtual ~ServerSocket();

    virtual void setPortNumber(unsigned short portNumber);

    virtual void open();
    virtual void read();
    virtual void write() {} //not writable

    virtual void setAcceptCallBack(CallBack<int>* pAcceptCallBack);
    
    virtual void setShouldAcceptConnections(bool shouldAccept) 
    {
        mIsReadable = shouldAccept;
    }

protected:
    ServerSocket(const ServerSocket&);

    void deleteAcceptCallBack();
    
    unsigned short mPortNumber;
    sockaddr_in mClientAddr;
    sockaddr_in mServAddr;
    CallBack<int>* mpAcceptCallBack;
};

#endif //SERVER_SOCKET_H
