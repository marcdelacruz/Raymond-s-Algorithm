/*
 *  ServerSocket.cpp
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#include "ServerSocket.h"

extern "C"
{
#include <strings.h>
}

using namespace std;

ServerSocket::ServerSocket() : Fd(sizeof(MessageTag), true/*selectable*/, 
                               true/*isReadable*/), mPortNumber(0), 
                               mpAcceptCallBack(0)
{
}

ServerSocket::ServerSocket(unsigned short portNumber) : Fd(0, true/*selectable*/, 
                               true/*isReadable*/), mPortNumber(portNumber), 
                               mpAcceptCallBack(0)
{
}

ServerSocket::~ServerSocket() 
{
    deleteAcceptCallBack();
}

void ServerSocket::setPortNumber(unsigned short portNumber) 
{
    mPortNumber = portNumber;
}

void ServerSocket::open() 
{
    //create, bind, listen, set to non-blocking
    if ((mFd = socket(AF_INET, SOCK_STREAM, 0)) >= 0)
    {
        bzero(&mServAddr, sizeof(mServAddr));
        mServAddr.sin_family = AF_INET;
        mServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        mServAddr.sin_port = htons(mPortNumber);
        int opt = 1;
        setsockopt (mFd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, 
                    sizeof(opt));
        if (bind(mFd, (sockaddr*)&mServAddr, sizeof(mServAddr)) >= 0 )
        {
            //need to convert to non-blocking somewhere here
            if (listen(mFd, 1) < 0)
            {
                //cerr << "Error on listen()" << endl;
                mFd = -1;
            }
            else
            {
                setNonBlocking();
                setShouldAcceptConnections(true);
                cerr <<"Listening on port["<<mPortNumber<<"]...."<< endl;
            }
        }//end if bind succeeded
        else 
        {
            //cerr << "Error on bind" << endl;
        }
    }//end if socket creation successful
    else 
    {
        //cerr << "Error on socket creation" << endl; 
    }
}// end void open()

void ServerSocket::read() 
{
    //do the accept instead of the read
    socklen_t clientLen = sizeof(mClientAddr);
    int connFd = accept(getFd(), (sockaddr*) &mClientAddr, &clientLen);
    if (connFd >= 0)
    { 
        //cerr << "Accepting client socket " << mClientAddr.sin_addr.s_addr << endl;
        if (mpAcceptCallBack != 0)
            mpAcceptCallBack->doCallBack(connFd);// call the call back
    }
    else if (connFd < 0) //error
    {
        //cerr << "Error on accept()" << endl;
    }
}//end void read()

void ServerSocket::setAcceptCallBack(CallBack<int>* pAcceptCallBack)
{
    deleteAcceptCallBack();
    mpAcceptCallBack = pAcceptCallBack;
}

void ServerSocket::deleteAcceptCallBack()
{
    if (mpAcceptCallBack != 0)
    {
        delete mpAcceptCallBack;
        mpAcceptCallBack = 0;
    }
}
