/*
 *  ClientSocket.cpp
 * Marc DelaCruz
 * CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#include "ClientSocket.h"

ClientSocket::ClientSocket() : ConnSocket(sizeof(MessageTag), true/*selectable*/, 
                            false/*isReadable*/), mServerAddrStr(""),
                            mServerPortNumber(0), mpOpenCallBack(0)
{
}

ClientSocket::ClientSocket(const char* pServerAddr, 
                           unsigned short serverPortNumber) : 
                           ConnSocket(sizeof(MessageTag), true/*selectable*/, 
                                      false/*isReadable*/), 
                                      mServerAddrStr(pServerAddr),
                                      mServerPortNumber(serverPortNumber),
                                      mpOpenCallBack(0)
{
}

ClientSocket::~ClientSocket() 
{
    deleteOpenCallBack();
    clearMsgQ();
}

void ClientSocket::setServerAddr(const char* pServerAddr) 
{
    mServerAddrStr = pServerAddr; 
}

void ClientSocket::setServerPortNumber(unsigned short serverPortNumber) 
{
    mServerPortNumber = serverPortNumber; 
}

void ClientSocket::open()
{
    sockaddr_in servAddr;
    hostent* pHostent = 0;
    bool hasFoundValidAddr = false;
    bzero(&servAddr, sizeof(servAddr));
    int ret = inet_pton(AF_INET, mServerAddrStr.c_str(), 
                        &servAddr.sin_addr); 
    if(ret != 1)// returns -1 if not in dot notation
    {
        //treat as DNS address
        if (ret == 0)
        {
            pHostent = gethostbyname(mServerAddrStr.c_str());
            if (pHostent)
            {
                memcpy(&servAddr.sin_addr.s_addr, *(pHostent->h_addr_list),
                        pHostent->h_length);
                hasFoundValidAddr = true;
            }
        }
    } // ret != 1
    else
    {
        hasFoundValidAddr = true;
    }

    if (hasFoundValidAddr)
    {
        if ((mFd = socket(AF_INET, SOCK_STREAM, 0)) >= 0)
        {
            servAddr.sin_family = AF_INET;
            servAddr.sin_port = htons(mServerPortNumber);
            setNonBlocking();
            int retVal = connect(mFd, (sockaddr*)&servAddr, 
                                    sizeof(servAddr));
            int errNum = errno;
            if (retVal >= 0)
            {
                //successful
                //cerr << "Connect successful!" << endl;
                mIsConnectPending = false;
                setNonBlocking();
                mIsReadable = true;
                setSendKeepAlive(true);
                if (mpOpenCallBack != 0)
                    mpOpenCallBack->doCallBack(getId());
            }
            else
            {
                if (errNum != EINPROGRESS)
                {
                    //cerr << "Connect error; Error is " << errNum 
                    //     << " " << strerror(errNum) << endl;
                    ::close(mFd);
                    mFd = -1;
                }
                else
                {
                    mIsConnectPending = true;
                    //cerr << "Connection pending..." << endl;
                }
            }
        }
        /*else
            cerr << "Socket creation error" << endl;*/
    }//end if found a valid address
    /*else
        cerr << "Can't find host " << mServerAddrStr 
             << ":" << mServerPortNumber << endl;*/
} //end open()

void ClientSocket::write()
{
    if (mIsConnectPending)
    {
        int opt = 0;
        socklen_t socklen = sizeof(int);
        int sockOptRet = getsockopt(mFd, SOL_SOCKET, SO_ERROR,
                                    (void*)(&opt), &socklen);
            
        if (sockOptRet >= 0 && opt == 0)
        {
            mIsConnectPending = false;
            //cerr << "Connect Successful" << endl;
            mIsReadable = true;
            setSendKeepAlive(true);
            if (mpOpenCallBack != 0)
                mpOpenCallBack->doCallBack(getId());
        }
        else
        {
            /*if (sockOptRet <= -1)
                cerr << "Error no getsockopt for connect()" << endl;
            else //if opt != 0
                cerr <<"Error on connect" << endl;*/
            ::close(mFd);
            mFd = -1;
        }
    }//end mIsConnectPending
    else
    {
        ConnSocket::write();
    }
}//end void write

bool ClientSocket::hasBytesToWrite() const
{
    return (ConnSocket::hasBytesToWrite() || mIsConnectPending);
}

void ClientSocket::setSelectable(bool shouldBeSelectable) 
{
    Fd::setSelectable(shouldBeSelectable); 
}

void ClientSocket::setOpenCallBack(CallBack<int>* pOpenCallBack)
{
    deleteOpenCallBack();
    mpOpenCallBack = pOpenCallBack;
}

void ClientSocket::deleteOpenCallBack()
{
    if (mpOpenCallBack != 0)
        delete mpOpenCallBack;
    mpOpenCallBack = 0;
}

