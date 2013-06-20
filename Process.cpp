/*
 *  Process.cpp
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#include "Process.h"

using namespace std;

Process::Process() : mrTimerHandler(Singleton<TimedActionHandler>::getInstance()),
                     mShouldRun(true), mHasInvalidArg(false), 
                     mHasDebugsOn(false)
{
}

void Process::initialize()
{
    intializeArgProcessorMap();
}

void Process::deInitialize()
{
    if (mShouldRun)
        mShouldRun = false;
    STATUS("EXITING...");
    //close all sockets
    for (unsigned int i = 0; i < mFdList.size(); ++i)
    {
        Fd& rFd = *mFdList.at(i);
        if (rFd.isOpen())
            rFd.close();
    }
    deInitializeArgProcessorMap();
}

bool Process::readArguments(int argc, char* argv[])
{
    bool hasUnknownArgs = false;
    bool hasReqArgsNotFound = false;
    mProcessName = argv[0];
    DEBUG("readArguments");
    if (argc > 1)
    {
        //for each entry in the arg table, //check for a match and call the callback   
        for (int i = 1; i < argc; ++i)
        {   
            string argStr(argv[i]);
            bool hasFoundMatch = false;
            for (PROCESS_ARG_MAP_ITR itr = mProcessArgTable.begin();
                 itr != mProcessArgTable.end(); ++itr)
            {
                string prefix = argStr.substr(0 , itr->first.length());
                if (itr->first.compare(prefix) == 0)
                {
                    //process
                    itr->second.performCallBack(argStr);
                    hasFoundMatch = true;
                    break;
                }
            }
            if (!hasFoundMatch)
                hasUnknownArgs = true;
        }
        //loop through the table, check if there are required args not found, set error flag

        for (PROCESS_ARG_MAP_ITR itr = mProcessArgTable.begin();
             itr != mProcessArgTable.end(); ++itr)
        {
            if (itr->second.isRequired() && !itr->second.wasArgProcessed())
            {
                hasReqArgsNotFound = true; 
                break;
            }
        }

        if (hasUnknownArgs || hasReqArgsNotFound)
        {
            if (hasUnknownArgs)
            {
                //log something
                DEBUG("Some unknown arguments detected");
            }
            else
            {
            }
            printArgHelp();
        }
    }//end if arguments passed
    else
    {
        for (PROCESS_ARG_MAP_ITR itr = mProcessArgTable.begin();
             itr != mProcessArgTable.end(); ++itr)
        {
            if (itr->second.isRequired())
            {
                printArgHelp();
                hasReqArgsNotFound = true;
                break;
            }
        }
    }
    DEBUG("printArgHelp END");

    return (!hasReqArgsNotFound && !mHasInvalidArg);
}//end readArguments

void Process::run(int argc, char* argv[])
{
    initialize();
    //read args setup stuff
    if (readArguments(argc, argv)) //true if allowed to run; args are OK
    {
        //actually run
        run();
    }
    deInitialize();
}

bool Process::setSignalHandler(int sigNum, 
                               int sigFlags,
                               void (*pFunc)(int, 
                                             siginfo_t*, 
                                             void *) )
{
    bool isSigHandlerSet = true;
    struct sigaction sigAction;
    memset(&sigAction, 0, sizeof(sigAction));

    sigAction.sa_sigaction = pFunc;
    sigfillset(&sigAction.sa_mask); //mask off all signals
    sigdelset(&sigAction.sa_mask, sigNum); //process only the given signal
    sigAction.sa_flags = sigFlags;
    if (sigaction(sigNum, &sigAction, 0) == -1)
        isSigHandlerSet = false; 
    return isSigHandlerSet;
}

void Process::deInitializeArgProcessorMap()
{
    //clean up the map
    //for each item in map, call delete
    for(unsigned int i = 0; i < mArgCallBackTable.size(); ++i)
    {
        CallBack<string>* pCallBack = mArgCallBackTable.at(i);
        if (pCallBack != 0)
        {
            delete pCallBack;
            mArgCallBackTable[i] = 0;
        }
    }
}

void Process::addProcessArg(string argPrefix, 
                            string helpString, 
                            void (*pFunc)(string), 
                            bool isRequired,  
                            bool hasNoInputRequired,
                            string paramName)
{
    CallBack<string>* pCallBack = (CallBack<string>*)new StaticCallBack<string>(pFunc);
    mArgCallBackTable.push_back(pCallBack);
    mProcessArgTable[argPrefix] = ProcessArg(argPrefix, helpString, pCallBack,
                                             isRequired, hasNoInputRequired, paramName);
}

void Process::addProcessArg(string argPrefix, string helpString, 
                            CallBack<string>* pCallBack, 
                            bool isRequired,  
                            bool hasNoInputRequired,
                            string paramName)
{
    mArgCallBackTable.push_back(pCallBack);
    mProcessArgTable[argPrefix] = ProcessArg(argPrefix, helpString, pCallBack,
                                             isRequired, hasNoInputRequired, 
                                             paramName);
}

void Process::printArgHelp() const
{
    cerr << "Usage: " << mProcessName << " -<Arg1>='<ArgVal1>'" 
         << " -<Arg2>='<ArgVal2>'... -<ArgN>='<ArgValN>'" << endl;
    cerr << "Arguments are: " << endl;
    for (PROCESS_ARG_MAP_CONST_ITR itr = mProcessArgTable.begin();
         itr != mProcessArgTable.end(); ++itr)
    {
        cerr << "  " << itr->second.getArgPrefix();
        if (itr->second.hasNoInputRequired())
        {
            cerr << " ";
        }
        else
        {
            cerr << "='" << itr->second.getParamName() << "' : '"
                 << itr->second.getParamName() << "' is the ";
        }
        cerr << itr->second.getHelpString() 
             << (itr->second.isRequired() ? " [REQUIRED]": " [OPTIONAL]") 
             << endl;
    }
}

void Process::run()
{
    //do the select loop
    while (mShouldRun)
    {
        int width = 0;
        fd_set readFds;
        fd_set writeFds;
        timeval timeout;
        memset(&timeout, 0, sizeof(timeout));
        timeout.tv_sec = 2; 
        timeout.tv_usec = 0;
        FD_ZERO(&readFds);
        FD_ZERO(&writeFds);

        //process timers here
        mrTimerHandler.processTimedActions();

        //loop through each FD, do FD_SET if selectable, the call select
        for (unsigned int i = 0; i < mFdList.size(); ++i)
        {
            Fd& rFd = *mFdList.at(i);
            if (rFd.isSelectable() && !rFd.isOpen())
            {
                rFd.open();
            }
            if (rFd.isOpen())
            {
                //update width
                width = ((rFd.getWidth() > width) ? 
                          rFd.getWidth() : width);
                if (rFd.isReadable())
                {
                    FD_SET(rFd.getFd(), &readFds);
                }
                if (rFd.hasBytesToWrite())
                {
                    FD_SET(rFd.getFd(), &writeFds);
                }
            }
        }//end for each socket
        if (width >= 1)
        {
            int selectRetVal = select(width, &readFds, &writeFds, 0, &timeout);
            if (selectRetVal > 0)
            {
                for (unsigned int i = 0; i < mFdList.size(); ++i)
                {
                    Fd& rFd = *mFdList.at(i);
                    if (rFd.isOpen() && FD_ISSET(rFd.getFd(), &readFds))
                        rFd.read();
                    if (rFd.isOpen() && FD_ISSET(rFd.getFd(), &writeFds))
                        rFd.write();
                }
            }
        }

        for (unsigned int i = 0; i < mFdList.size(); ++i)
        {

            if (mFdList.at(i)->shouldRemoveFromList())
            {
                mFdList.erase(mFdList.begin() + i);
                //may need to also delete this socket
            }
        }
        processPostLoopAction();
    }// end while true
}// end Process::run

void Process::addToList(Fd* pFd)
{
    bool isInList = false;
    for (unsigned int i = 0; i < mFdList.size(); ++i)
    {
        Fd& rFd = *mFdList.at(i);
        if (pFd == &rFd || rFd.getId() == pFd->getId())
        {
            isInList = true;
            break;
        }
    }
    if (!isInList)
        mFdList.push_back(pFd);        
}

void Process::removeFromList(int fdId)
{
    for (unsigned int i = 0; i < mFdList.size(); ++i)
    {
        if (mFdList.at(i)->getId() == fdId)
        {
            mFdList.at(i)->setShouldRemoveFromList(true);
            break;
        }
    }
}

string Process::getArgSubStr(string& rArgString)
{
    return (rArgString.substr(rArgString.find_first_of("=")+1));
}

void Process::showPrompt()
{
    cerr << mPrompt;
}

void Process::setHasInvalidArg()
{
    mHasInvalidArg = true;
}
