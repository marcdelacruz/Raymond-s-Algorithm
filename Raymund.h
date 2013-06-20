/*
 *  Raymund.h
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 *
 *  Created by Marc on 6/13/10.
 *
 */

#ifndef __RAYMUND_H_
#define __RAYMUND_H_

#include <queue>
#include <vector>
#include "Process.h"
#include "ClientSocket.h"
#include "ServerSocket.h"

#define STD_IN_BUFFER_SIZE  1024
class StdIn : public Fd
{
    public:
    StdIn() : Fd(STD_IN_BUFFER_SIZE, true, true)
    {
        setId(0);
    }
    virtual ~StdIn() { }

    //define the interface for the Process class
    void open() { mFd = 0; }
    void read();
    void write() {}
    void close() {} //don't close
    bool hasBytesToWrite() const { return false; }
};

class Raymund : public Process, public Singleton<Raymund>
{
    public:
        static void handleTermSignal(int sigNum, siginfo_t* pSigInfo, void* pContext);
        virtual void initialize();
        virtual void deInitialize();
        virtual void processPostLoopAction();
        virtual void read();
    protected:
        friend class Singleton<Raymund>;
        Raymund();
        virtual ~Raymund();
        virtual void intializeArgProcessorMap();
        virtual void deInitializeArgProcessorMap();

        void handleNumArg(std::string argString);
        void handleDebugArg(string argString);
        void handleConfigFileArg(string argString);
        void readConfig();
        void processServerAccept(int fdId);
        void addProcessArg(string argPrefix, string helpString,
                           void (Raymund::*pFunc)(string), 
                           bool isRequired,  
                           bool hasNoInputRequired, string paramName);
        void processClientOpen(int nodeId);
        void processRead(ReadMessage* pMsg);

        void processTimeToLive();
        void processUnknownRead(ReadMessage* pMsg);
        void processUnknownClose(int nodeId);
        void processClientClose(int nodeId);
        void processKnownClose(int nodeId);
        void checkSystemReady();
        void sendMsg(Message& rMsg, int destNode);
        
        int mId;
        bool mIsSystemReady;
        int mCurTime;
        StdIn mCmdLine;
        std::string mConfigFile;
        ServerSocket mServerSocket;
        Timer mTimeToLiveTimer;
        VoidObjectCallBack<Raymund> mTtlCallBack;
        std::map<int, ConnSocket*> mUnknownNodeList;
        std::vector<ConnSocket*> mRemoveList;
        std::map<int, ConnSocket*> mConnList;
        std::map<int, ClientSocket*> mClientList; // never clean up until the end
        int mNumNodes;
        std::map<int, int > mFwdTable;
        int mNumNeighbors;
        int mTokenDirection;
        bool mHasToken;
        bool mIsIdle;
        std::queue<int> mRequestQueue;
        bool mHasRequest;
};//end Raymund

#endif// __RAYMUND_H_


