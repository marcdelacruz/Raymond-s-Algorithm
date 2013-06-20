/*
 *  ConnSocket.cpp
 * Marc DelaCruz
 * CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#include "ConnSocket.h"

using namespace std;

ConnSocket::ConnSocket(int maxBytesToRead, bool isSelectable, 
                       bool isReadable) : Fd(maxBytesToRead, 
                                             isSelectable, isReadable), 
                                             mpCloseCallBack(0),
                                             mpReadCallBack(0)
{
}

ConnSocket::ConnSocket(int fd, int maxBytesToRead, bool isSelectable, 
           bool isReadable) : Fd(maxBytesToRead, isSelectable, isReadable),
                              mpCloseCallBack(0), mpReadCallBack(0)

{
    setFd(fd);
}

ConnSocket::~ConnSocket() 
{
    clearMsgQ();
    deleteReadCallBack();
    deleteCloseCallBack();
}

void ConnSocket::setFd(int fd) 
{
    mFd = fd;
    setNonBlocking();
    setSendKeepAlive(true);
}

void ConnSocket::read() 
{
    //do a buffered read, attempt to read the header, 
    //then read the read the payload
    if ((unsigned int)mNumBytesRead < sizeof(Header))
    {       
        char* pMsg = (char*)&mMsgBuffer;
        int readBytes = ::read(getFd(), (void*)&pMsg[mNumBytesRead], 
                           sizeof(Header) - mNumBytesRead);
        int errNum = errno;
        if (readBytes > 0)
        {
            mNumBytesRead += readBytes;
            if ((unsigned int)mNumBytesRead >= sizeof(Header))
            {
                mBytesToRead = (sizeof(Header) + 
                                mMsgBuffer.hdr.payloadSize - mNumBytesRead);
                if ((unsigned int)mNumBytesRead == sizeof(Header) && 
                    mBytesToRead == 0)
                {
                    if (mpReadCallBack != 0)
                    {
                        ReadMessage readMsg(getId(), mMsgBuffer);
                        mpReadCallBack->doCallBack(&readMsg);
                        resetReadVariables();
                    }
                }
            }//end else if mNumBytesRead >= sizeof(header)
            else
            {
                mBytesToRead = sizeof(Header) - mNumBytesRead;
            }
        }//end if read > 0 bytes
        else if (readBytes == -1 && errNum != EAGAIN)
        {
            //do call back with an error
            mMsgBuffer.hdr.payloadSize = -1; //indicate error
            if (mpReadCallBack != 0)
            {
                ReadMessage readMsg(getId(), mMsgBuffer);
                mpReadCallBack->doCallBack(&readMsg);
            }
            //cerr << "ReadBytes == -1 && errNum != EAGAIN "
            //     << strerror(errNum) << endl;
            close();
        }
    }
    else //mNumBytesRead >= sizeof(Message.hdr)
    {
        char* pMsg = (char*)&mMsgBuffer;
        int readBytes = ::read(getFd(),(void*) &pMsg[mNumBytesRead], 
                               mBytesToRead );
        int errNum = errno;
        if (readBytes > 0)
        {
            mNumBytesRead += readBytes;
            if ((unsigned int)mNumBytesRead == mMsgBuffer.hdr.payloadSize + sizeof(Header))
            {
                if (mpReadCallBack != 0)
                {
                    ReadMessage readMsg(getId(), mMsgBuffer);
                    mpReadCallBack->doCallBack(&readMsg);
                }
                //clean up
                resetReadVariables();
            }
        }
        else if (readBytes == -1 && errNum != EAGAIN)
        {
            //do call back with an error
            mMsgBuffer.hdr.payloadSize = -1; //indicate error
            if (mpReadCallBack != 0)
            {
                ReadMessage readMsg(getId(), mMsgBuffer);
                mpReadCallBack->doCallBack(&readMsg);
            }
            //cerr << "ReadBytes == -1 && errNum != EAGAIN "
            //     << strerror(errNum) << endl;
            close();
        }
    }
} //end read()

void ConnSocket::close() 
{ 
    Fd::close(); 
    resetReadVariables();
    clearMsgQ();
    mShouldCloseAfterWrite = false;
    if (mpCloseCallBack != 0)
        mpCloseCallBack->doCallBack(getId());
}

void ConnSocket::resetReadVariables()
{
    mNumBytesRead = 0; 
    bzero(&mMsgBuffer, sizeof(mMsgBuffer));
    mBytesToRead = sizeof(Header);
}

bool ConnSocket::hasBytesToWrite() const 
{
    return (!mMsgQ.empty());
}

void ConnSocket::write()
{
    if (!mMsgQ.empty() && isOpen())
    {
        MessageTag* pMsg = mMsgQ.front();
        int size = pMsg->hdr.payloadSize+sizeof(Header);
        if (mWriteMsgPos < size)
        {
            int nBytesWritten = ::write(getFd(), (void*)pMsg, size - mWriteMsgPos);
            int errNum = errno;
            if (nBytesWritten > 0 )
            {
                mWriteMsgPos += nBytesWritten;
                if (mWriteMsgPos == size)
                {
                    mWriteMsgPos = 0;
                    mMsgQ.pop_front();
                    free(pMsg);
                    if (mShouldCloseAfterWrite && mMsgQ.empty())
                        close();
                }
            }//end wrote bytes > 0
            else if (nBytesWritten == -1 && errNum != EAGAIN)
            {
                //should close because this socket's probably dead/bad
                std::cerr << " BAD WRITE" << std::endl;
                close();
            }
        }//wrote less than size of the Message
    } // if (!mMsgQ.empty() && isOpen())
}//end write()

void ConnSocket::clearMsgQ()
{
    while(!mMsgQ.empty())
    {
        MessageTag* pMsg = mMsgQ.front();
        free(pMsg);
        mMsgQ.pop_front();
    }
    mWriteMsgPos = 0;
}

int ConnSocket::writeMsg(Message& rMessage)
{
    //put Message in a queue; set the flag
    //could check if queue is too big here
    int mallocSize = rMessage.getSize();
    int retVal = 0;
    if (!mShouldCloseAfterWrite)
    {
        MessageTag* pMsg = (MessageTag*)(malloc(mallocSize));
        if (pMsg != 0)
        {
            memcpy((void*)pMsg, (void*)&rMessage.getMsg(), rMessage.getSize());
            mMsgQ.push_back(pMsg);
            retVal = mallocSize;
            write();//immediately try a write
        }
        else
            cerr << "Malloc failed"<< endl;
    }
    return retVal;
}//end int write(Message&)

void ConnSocket::setReadCallBack(CallBack<ReadMessage* >* pReadCallBack)
{
    deleteReadCallBack();
    mpReadCallBack = pReadCallBack;
}

void ConnSocket::setCloseCallBack(CallBack<int>* pCloseCallBack)
{
    deleteCloseCallBack();
    mpCloseCallBack = pCloseCallBack;
}

void ConnSocket::setCloseAfterLastWrite(bool shouldClose)
{
    mShouldCloseAfterWrite = shouldClose;
}

void ConnSocket::setSendKeepAlive(bool shouldSendKeepAlive)
{
    int opt = shouldSendKeepAlive;
    setsockopt (mFd, SOL_SOCKET, SO_KEEPALIVE, (void*)&opt, 
                sizeof(opt));
}

void ConnSocket::deleteReadCallBack()
{
    if (mpReadCallBack != 0)
        delete mpReadCallBack;
    mpReadCallBack = 0;
}

void ConnSocket::deleteCloseCallBack()
{
    if (mpCloseCallBack != 0)
        delete mpCloseCallBack;
    mpCloseCallBack = 0;
}
