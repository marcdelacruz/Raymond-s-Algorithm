/*
 *  Fd.h (File Descriptor)
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */
#ifndef _FD_H
#define _FD_H

extern "C"
{
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stropts.h>
#include <sys/filio.h>
#include <unistd.h>
#include <strings.h>
}

#include <iostream>
#include <deque>
#include <cerrno>
#include <cstring>

#include "CallBack.h"
#include "MessageDefs.h"
#include "Message.h"

using namespace std;

class Fd //this defines the abstract socket class
{
public:
    Fd();

    Fd(int maxBytesToRead, bool isSelectable, bool isReadable);
    virtual ~Fd();

    //define the interface for the Process class
    virtual void open() = 0;
    virtual void read() = 0;
    virtual void write() = 0;
    virtual void close();
    virtual bool isSelectable() const;
    virtual bool isOpen() const;
    virtual bool isReadable() const;
    virtual bool hasBytesToWrite() const;
    virtual int getWidth();
    virtual int getFd() const;
    virtual void setNonBlocking();
    virtual void setId(int uniqueId) { mId = uniqueId; }
    virtual bool shouldRemoveFromList() const { return mShouldRemoveFromList; }
    
    virtual void setSelectable (bool shouldBeSelectable);
    virtual void setShouldRemoveFromList(bool shouldRemove);
    virtual int getId() const { return mId; }
    virtual int getSecondsToLive() const { return mSecondsToLive; }
    virtual void setSecondsToLive(int s) { mSecondsToLive = s; }

protected:
    size_t mBytesToRead;
    int mFd; //the actual file descriptor
    bool mIsSelectable; //true =  should automatically open in process loop
    bool mIsReadable;
    int mId;
    bool mShouldRemoveFromList;
    int mSecondsToLive;
};

#endif //_FD_H
