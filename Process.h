/*
 *  Process.h
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <csignal>
#include <map>
#include <vector>
#include <strings.h>
#include <iostream>
#include <sys/select.h>
#include <cstdlib>
#include <cstdio>

#include "TimedActionHandler.h"
#include "Singleton.h"
#include "ProcessArg.h"
#include "CallBack.h"
#include "Fd.h"

using std::map;
using std::string;
using std::vector;
#define ALERT_LOG(str) (cerr << "\n<<<<< " << str << " >>>>>\n" << endl)
#define STATUS(str) (cerr << str << endl)
#define DEBUG(str)  if (hasDebugsOn()) cerr << str << endl

class Process
{
public:
    virtual void initialize() = 0;
    virtual void deInitialize() = 0;
    virtual void run(int argc, char* argv[]);

    virtual bool setSignalHandler(int sigNum,
                                  int sigFlags, 
                                  void (*pFunc)(int, siginfo_t*, 
                                                void *) );
    virtual bool hasDebugsOn() const { return mHasDebugsOn; }
protected:
    Process();
    virtual ~Process() {}

    virtual void intializeArgProcessorMap() = 0;
    virtual void deInitializeArgProcessorMap() = 0;
    virtual bool readArguments(int argc, char* argv[]);


    virtual void addProcessArg(string argPrefix, 
                               string helpString, 
                               void (*pFunc)(string), 
                               bool isRequired,  
                               bool hasNoInputRequired, string paramName);

    virtual void addProcessArg(string argPrefix, string helpString, 
                               CallBack<string>* pCallBack, 
                               bool isRequired,  
                               bool hasNoInputRequired, string paramName);

    virtual void printArgHelp() const;

    virtual void run();
    virtual void addToList(Fd* pFd);
    virtual void removeFromList(int fdId);
    virtual string getArgSubStr(string& rArgString);
    virtual void showPrompt();
    virtual void setHasInvalidArg();
    virtual bool hasInvalidArg() const { return mHasInvalidArg; }
    virtual void processPostLoopAction() = 0;

    //map of filedescriptors
    typedef map<string, ProcessArg> PROCESS_ARG_MAP;
    typedef map<string, ProcessArg>::iterator PROCESS_ARG_MAP_ITR;
    typedef map<string, ProcessArg>::const_iterator PROCESS_ARG_MAP_CONST_ITR;
    PROCESS_ARG_MAP mProcessArgTable; //list/map of args to listen to/check
    vector< CallBack<string>* > mArgCallBackTable;
    //timed actions
    TimedActionHandler& mrTimerHandler;
    //add signal handlers for termination.
    string mProcessName;
    vector<Fd*> mFdList;
public:
    bool mShouldRun;
protected:
    string mPrompt;
    bool mHasInvalidArg;
    bool mHasDebugsOn;
};//end class Process

#endif //PROCESS_H


