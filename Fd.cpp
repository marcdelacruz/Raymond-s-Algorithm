/*
 *  Fd.cpp
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#include "Fd.h"

Fd::Fd() : mBytesToRead(0), mFd(-1), mIsSelectable(false), 
           mIsReadable(false), mShouldRemoveFromList(false), 
           mSecondsToLive(-1)
                   
{
}

Fd::Fd(int maxBytesToRead, bool isSelectable, bool isReadable) : 
       mBytesToRead(maxBytesToRead), mFd(-1),
       mIsSelectable(isSelectable), mIsReadable(isReadable), 
       mShouldRemoveFromList(false), mSecondsToLive(-1)
{
}

Fd::~Fd() 
{
    close(); 
}

void Fd::close() 
{ 
    if (mFd >= 0) 
    { 
        ::close(mFd);
    } 
    mFd = -1; 
}

bool Fd::isSelectable() const 
{
    return mIsSelectable;
}

bool Fd::isOpen() const 
{
    return (mFd > -1);
}

bool Fd::isReadable() const 
{
    return (isOpen() and mIsReadable); 
}

bool Fd::hasBytesToWrite() const 
{
    return false;
}

int Fd::getWidth() 
{
    return (isOpen() ? (mFd+1) : 0); 
}

int Fd::getFd() const 
{
    return mFd;
}

void Fd::setNonBlocking() 
{ 
    int ioArg = 1;
    if (isOpen())
    {
        ioctl(getFd(), FIONBIO, &ioArg);
    }
}
    
void Fd::setSelectable (bool shouldBeSelectable) 
{
    mIsSelectable = shouldBeSelectable; 
}

void Fd::setShouldRemoveFromList(bool shouldRemove)
{
    mShouldRemoveFromList = shouldRemove;
    mIsSelectable = false;
}
