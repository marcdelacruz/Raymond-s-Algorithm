/*
 *  Singleton.h
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#ifndef SINGLETON_H
#define SINGLETON_H

template<class T> 
class Singleton
{
public:
    static T& getInstance()
    {
        static T objInstance;
        return objInstance;
    }
};

#endif //SINGLETON_H


