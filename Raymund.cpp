/*
 *  Raymund.cpp
 *  
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 *  Created by Marc on 6/13/10.
 *
 */
#include <algorithm>
#include <fstream>
#include <queue>
#include <stack>

#include "Raymund.h"
#include "ConnSocket.h"
#include "Utilities.h"

#define TTL_TIME           15
#define TTL_CHECK_INTERVAL 1
enum SEARCH_STATUS { SEARCH_WHITE, SEARCH_GRAY, SEARCH_BLACK };

typedef std::map<int, ConnSocket*>::iterator FdMapItr;


void StdIn::read()
{
     //get the Singleton 
     Singleton<Raymund>::getInstance().read();
}

void Raymund::handleTermSignal(int sigNum, siginfo_t* pSigInfo, void* pContext)
{
    if (sigNum == SIGINT || sigNum == SIGTERM || 
        sigNum == SIGHUP || sigNum == SIGQUIT)
    {
        Singleton<Raymund>::getInstance().mShouldRun = false;
    }
    //ignore SIGPIPE
}//end Router::handleTermSignal

Raymund::Raymund() : mId(-1), mIsSystemReady(false),
                               mCurTime(0),
                               mConfigFile(),
                               mTimeToLiveTimer(),
                               mTtlCallBack(this, &Raymund::processTimeToLive),
                               mNumNodes(0),
                               mNumNeighbors(0),
                               mTokenDirection(1), //always starts with node 1
                               mHasToken(false),
                               mIsIdle(true),
                               mRequestQueue(),
                               mHasRequest(false)
{
    mPrompt = "Enter CMD> " ;
}

Raymund::~Raymund()
{
}

void Raymund::initialize()
{
    setSignalHandler(SIGINT, SA_SIGINFO|SA_RESTART, &Raymund::handleTermSignal);
    setSignalHandler(SIGTERM, SA_SIGINFO|SA_RESTART, &Raymund::handleTermSignal);
    setSignalHandler(SIGHUP, SA_SIGINFO|SA_RESTART, &Raymund::handleTermSignal);
    setSignalHandler(SIGQUIT, SA_SIGINFO|SA_RESTART, &Raymund::handleTermSignal);
    setSignalHandler(SIGPIPE, SA_SIGINFO|SA_RESTART, &Raymund::handleTermSignal);
    Process::initialize();
}

void Raymund::deInitialize()
{
    Process::deInitialize();
    FdMapItr itr = mUnknownNodeList.begin();
    for (; itr != mUnknownNodeList.end(); ++itr)
    {
        ConnSocket* pConnSock = itr->second;
        if (pConnSock != 0)
            delete pConnSock;
    }
    mUnknownNodeList.clear();
    itr = mConnList.begin();
    for (; itr != mConnList.end(); ++itr)
    {
        ConnSocket* pConnSock = itr->second;
        if (pConnSock != 0)
            delete pConnSock;
    }
    mConnList.clear();
    std::map<int, ClientSocket*>::iterator cItr = mClientList.begin();
    for (; cItr != mClientList.end(); ++cItr)
    {
        ClientSocket* pClientSock = cItr->second;
        if (pClientSock != 0)
            delete pClientSock;
    }
    mClientList.clear();
    for (size_t i = 0; i < mRemoveList.size(); ++i)
    {
        ConnSocket* pConn = mRemoveList.at(i);
        delete pConn;
        mRemoveList.erase(mRemoveList.begin() + i);
    }
}

void Raymund::processPostLoopAction()
{
    for (size_t i = 0; i < mRemoveList.size(); ++i)
    {
        ConnSocket* pConn = mRemoveList.at(i);
        delete pConn;
        mRemoveList.erase(mRemoveList.begin() + i);
    }
}

void Raymund::read()
{
    char cBuffer[STD_IN_BUFFER_SIZE];
    bzero(&cBuffer, sizeof(cBuffer));
    cin.getline(cBuffer, sizeof(cBuffer));
    string buffer(cBuffer);
    if (mHasRequest)
        return;
    if (buffer.length() > 0)
    {
        //DEBUG("got message: " << buffer );
        std::vector<std::string> tokens;
        parse(buffer, tokens, '>');

        if (tokens.size() > 0)
        {
            std::string& rCmd = tokens.at(0);

            if (rCmd.compare("exit") == 0 or rCmd.compare("quit") == 0 or 
                     rCmd.compare("q") == 0 or rCmd.compare("x") == 0)
            {
                 mShouldRun = false;
            }
            else if (rCmd.compare("cs-enter") == 0 or rCmd.compare("enter") == 0)
            {
                if (mHasToken and mIsIdle)
                {
                    mIsIdle = false;
                    DEBUG("Has Token...");
                }
                else if (!mHasToken)
                {
                    //wait till ya have it
                    //send request to where the token is
                    if (mRequestQueue.empty())
                    {
                        STATUS("Sending request to " << mTokenDirection);
                        Message requestMsg(TOKEN_REQUEST, mTokenDirection);
                        sendMsg(requestMsg, mTokenDirection);
                    }
                    mRequestQueue.push(mId);
                    mHasRequest = true;

                    return;
                }
                else if (mHasToken and !mIsIdle)
                    DEBUG("Still in critical section...ignoring");
            }
            else if (rCmd.compare("cs-leave") == 0 or rCmd.compare("leave") == 0)
            {
                if (mHasToken and !mIsIdle)
                {
                    STATUS("<<<<<    Leaving critical section...    >>>>>");
                    mIsIdle = true;
                    
                    if (!mRequestQueue.empty())
                    {
                        //pop the first entry
                        //send to neighbor
                        mHasToken = false;
                        int request = mRequestQueue.front();
                        mRequestQueue.pop();
                        STATUS("Sending token to " << request);
                        Message token(TOKEN_MSG, request);
                        sendMsg(token, request);
                        mTokenDirection = request;
                        //send another
                        int anotherRequest = 0;
                        if (!mRequestQueue.empty())
                        {
                            anotherRequest = mRequestQueue.front();
                            STATUS("Sending request for next in queue " << 
                                   anotherRequest );
                            Message requestMsg(TOKEN_REQUEST, anotherRequest);
                            sendMsg(requestMsg, mTokenDirection);
                        }
                    }
                }
            }
            else if (rCmd.compare("debug") == 0)
            {
                if (tokens.size() > 1)
                    mHasDebugsOn = atoi(tokens.at(1).c_str());
                else
                    mHasDebugsOn = !mHasDebugsOn;
            }
            else
                STATUS("Unknown Command: " << rCmd);
        }//end if (tokens.size() > 0)
    }//end buffer.length() > 0
    showPrompt();
}//end read()

void Raymund::intializeArgProcessorMap()
{
    addProcessArg(string("-n"), 
                  string("process ID (positive integer)"),
                  &Raymund::handleNumArg, REQUIRED_ARG, INPUT_NEEDED,
                  string("process ID"));
    addProcessArg(string("-d"), 
                  string("enable debugs"),
                  &Raymund::handleDebugArg, OPTIONAL_ARG, NO_INPUT_NEEDED,
                  string("debugs"));
    addProcessArg(string("-c"), 
                  string("config file name"),
                  &Raymund::handleConfigFileArg, REQUIRED_ARG, INPUT_NEEDED,
                  string("config file"));
}

void Raymund::deInitializeArgProcessorMap()
{
}

void Raymund::handleNumArg(std::string argString)
{
    string processNumStr = getArgSubStr(argString);
    mId = atoi(processNumStr.c_str());
    DEBUG("Process Number=" << mId );
    Message::setSrcId(mId);
    if (mId == 1)
        mHasToken = true;
    if (mConfigFile.length() > 0)
        readConfig();
}

void Raymund::handleDebugArg(string argString)
{
    mHasDebugsOn = true;
    DEBUG("Debugs On...");
}

void Raymund::handleConfigFileArg(string argString)
{
    mConfigFile = getArgSubStr(argString);
    DEBUG("ConfigFile=" << mConfigFile);
    if (mId > 0)
        readConfig();
}

void Raymund::readConfig()
{
    std::ifstream configFile;
    configFile.open(mConfigFile.c_str());
    std::map<int, std::vector<int> > adjacencyList;//adjacency list
    std::map<int, std::pair<std::string/*address*/, int/*port num*/> > serverInfo;
    /*
    the file is in the form
    ID address serverport
    1 net24 25571 2 3
    2 net34 25572 1 3
    3 net25 25573 1 2
    */
    if (configFile.is_open())
	{
        STATUS("Reading ConfigFile...");

		while (!configFile.eof() )
        {
            std::string line;
            std::getline (configFile, line);
            std::vector<std::string> tokens;
            parse(line, tokens, ' ');
            if (tokens.size() <= 0)
            {
                continue;
            }
            else if (tokens.size() < 3)
            {
                DEBUG("Not enough tokens=" << tokens.size());
                mShouldRun = false;
                break;
            }
            else
            {
                int num = atoi(tokens.at(0).c_str());
                int port = atoi(tokens.at(2).c_str());
                std::string serverAddr = tokens.at(1);
                DEBUG("num=" << num << 
                       " serverAddr=" << serverAddr << 
                       " port=" << port);
                if (num <= 0)
                    continue;
                mNumNodes++;
                serverInfo[num].first = serverAddr;
                serverInfo[num].second = port;
                for (size_t i = 3; i < tokens.size(); ++i)
                {
                    int adjNode = atoi(tokens.at(i).c_str());
                    if (adjNode > 0 and adjNode != num)
                    {
                        DEBUG("adjacent nodes=" << adjNode);
                        adjacencyList[num].push_back(adjNode);
                    }
                }
            }//end else valid param count
        }//end while
        //
        configFile.close();
    }//end if configFile.is_open()
    else
        DEBUG("config file not open");
        
    /*
    now actually build the tree
    and do the routing
    */
    std::map<int, SEARCH_STATUS> visitMap;
    
    std::queue<int> bfsQueue;
    std::map<int, std::vector<int> > tree;
    //run BFS
    std::map<int, std::vector<int> >::iterator itr = adjacencyList.begin();
    if (itr != adjacencyList.end())
    {
        bfsQueue.push(itr->first);//start with the first entry
        while (!bfsQueue.empty())
        {
            //pop at the head
            int curNode = bfsQueue.front();
            DEBUG("Visiting node=" << curNode);
            visitMap[curNode] = SEARCH_GRAY;
            bfsQueue.pop();
            std::map<int, std::vector<int> >::iterator nItr = adjacencyList.find(curNode);
            if (nItr != adjacencyList.end())
            {
                for (size_t n = 0; n < nItr->second.size(); ++n)
                {
                    int neighbor = nItr->second.at(n);
                    if (visitMap[neighbor] == SEARCH_WHITE)
                    {
                        //mark neighbors as gray
                        visitMap[neighbor] = SEARCH_GRAY;
                        bfsQueue.push(neighbor);
                        DEBUG("Adding neighbor "<< neighbor << " for " << curNode);
                        tree[curNode].push_back(neighbor);
                        tree[neighbor].push_back(curNode);
                    }
                }//for each neighbor
            }//if node exists.
            visitMap[curNode] = SEARCH_BLACK;
        }//end while bfsQueue is not empty
    }//end 
    else
        mShouldRun = false;
    //dump the trimmed adjacency list
    for (std::map<int, std::vector<int> >::iterator xItr = tree.begin();
         xItr != tree.end(); ++xItr)
    {
        DEBUG("Node " << xItr->first);
        for (size_t n = 0; n < xItr->second.size(); ++n)
            DEBUG("\t\t->" << xItr->second.at(n));
    }

    //do DFS this time!!!
    STATUS("Building Forwarding Table");
    
    //add root to stack
    //save current route
    std::stack< std::pair<int /*node*/,int /*ancestor*/> > dfsStack;
    
    dfsStack.push(std::make_pair(mId, mId));
    visitMap.clear();
    while (!dfsStack.empty())
    {
        //pop at the head
        int curNode = dfsStack.top().first;
        int parent = dfsStack.top().second;
        DEBUG("Visting node=" << curNode);
        visitMap[curNode] = SEARCH_GRAY;
        dfsStack.pop();
        std::map<int, std::vector<int> >::iterator nItr = tree.find(curNode);
        if (nItr != tree.end())
        {
            for (size_t n = 0; n < nItr->second.size(); ++n)
            {
                int neighbor = nItr->second.at(n);
                if (visitMap[neighbor] == SEARCH_WHITE)
                {
                    //mark neighbors as gray
                    visitMap[neighbor] = SEARCH_GRAY;
                    //if parent == mId, use current
                    //else use parent's parent
                    int ancestor = ((parent == mId) ? neighbor : parent);
                    dfsStack.push(std::make_pair(neighbor, ancestor ));
                    DEBUG("Adding neighbor "<< neighbor << " for " << curNode);
                    mFwdTable[neighbor] = ancestor;
                }
            }//for each neighbor
        }//if node exists.
        visitMap[curNode] = SEARCH_BLACK;
    }//end while dfsStack is not empty

    DEBUG("Dumping Forwarding Table");
    for (std::map<int, int >::iterator xItr = mFwdTable.begin();
         xItr != mFwdTable.end(); ++xItr)
    {
        DEBUG("Node " << xItr->first << " route=" << xItr->second);
    }

    DEBUG("Creating connections...");
    bool hasServer = false;
    std::map<int, std::vector<int> >::iterator nItr = tree.find(mId);
    for (size_t n = 0; n < nItr->second.size(); ++n)
    {
        int num = nItr->second.at(n);
        std::string serverAddr = serverInfo[num].first;
        short port = serverInfo[num].second;
        mNumNeighbors++;
        if (mId < num)
        {
            DEBUG("Creating Client sock");
            ClientSocket* pClient = new ClientSocket(serverAddr.c_str(), port);
            pClient->setId(num);
            pClient->setSecondsToLive(TTL_TIME);
            CallBack<int>* pOpenCallBack  = dynamic_cast<CallBack<int>*>(
                                      new ObjectCallBack<Raymund, int>(this, 
                                                    &Raymund::processClientOpen));
            pClient->setOpenCallBack(pOpenCallBack);

            CallBack<ReadMessage* >* pReadCallBack  = \
                                    dynamic_cast<CallBack<ReadMessage* >*>(
                                      new ObjectCallBack<Raymund, ReadMessage*>(this, 
                                                    &Raymund::processRead));
            pClient->setReadCallBack(pReadCallBack);

            CallBack<int>* pCloseCallBack  = dynamic_cast<CallBack<int>*>(
                                      new ObjectCallBack<Raymund, int>(this, 
                                                    &Raymund::processClientClose));
            pClient->setCloseCallBack(pCloseCallBack);
            mClientList[num] = pClient;
            addToList(dynamic_cast<Fd*> (pClient));
        }
        else
            hasServer = true;
    }
    if (hasServer)
    {
        mServerSocket.setPortNumber(serverInfo[mId].second);
        
        CallBack<int> * pAcceptCallBack = (dynamic_cast<CallBack<int>*> 
                             (new ObjectCallBack<Raymund, int>(this, 
                                            &Raymund::processServerAccept)));
                             
        mServerSocket.setAcceptCallBack(pAcceptCallBack);
        mTimeToLiveTimer.start((VoidCallBack*)&mTtlCallBack, 
                                      TTL_CHECK_INTERVAL, true);
        mServerSocket.setId(-32000);
        addToList(dynamic_cast<Fd*> (&mServerSocket));
    }

}//end Raymund::readConfig

void Raymund::processServerAccept(int fdId)
{
    //
    ConnSocket* pConn = new ConnSocket(fdId, sizeof(Message), true, true);
    pConn->setId(-fdId);
    DEBUG("processServerAccept fdId=" << fdId 
          << " ID=" << pConn->getId());
    pConn->setFd(fdId);
    //set selectable
    pConn->setNonBlocking();
    pConn->setSelectable(true);
    DEBUG("processServerAccept FD=" << pConn->getFd() 
          << " ID=" << pConn->getId() << " sel=" 
          << pConn->isSelectable() << " open=" << pConn->isOpen() );
    //set TTL
    pConn->setSecondsToLive(TTL_TIME);
    //need to add open and close call back
    CallBack<ReadMessage* >* pReadCallBack  = \
                            dynamic_cast<CallBack<ReadMessage* >*>(
                              new ObjectCallBack<Raymund, ReadMessage*>(this, 
                                            &Raymund::processUnknownRead));
    pConn->setReadCallBack(pReadCallBack);
    CallBack<int>* pCloseCallBack  = dynamic_cast<CallBack<int>*>(
                              new ObjectCallBack<Raymund, int>(this, 
                                            &Raymund::processUnknownClose));
    pConn->setCloseCallBack(pCloseCallBack);
    mUnknownNodeList[-fdId] = pConn;
    addToList(dynamic_cast<Fd*> (pConn));
    //declare our identity
    Message nodeIdMsg;
    nodeIdMsg.createNodeIdMsg(mId);
    pConn->setNonBlocking();
    pConn->writeMsg(nodeIdMsg);
}//end Raymund::processServerAccept

void Raymund::addProcessArg(string argPrefix, string helpString,
                           void (Raymund::*pFunc)(string), 
                           bool isRequired,  
                           bool hasNoInputRequired, string paramName)
{
    CallBack<string>* pCallBack (dynamic_cast<CallBack<string>*> 
                         (new ObjectCallBack<Raymund, string>(this, pFunc)));
    Process::addProcessArg(argPrefix, helpString, pCallBack, isRequired, 
                           hasNoInputRequired, paramName);
}//end Raymund::addProcessArg

void Raymund::processClientOpen(int nodeId)
{
    //send node ID
    DEBUG("Process Client Open=" << nodeId);
    Message msg;
    msg.createNodeIdMsg(mId);
    mClientList[nodeId]->setNonBlocking();
    mClientList[nodeId]->writeMsg(msg);
    mClientList[nodeId]->setSecondsToLive(TTL_TIME);
    mClientList[nodeId]->setSelectable(true);
    checkSystemReady();
}

void Raymund::processRead(ReadMessage* pMsg)
{
    //process if system ready
    Message msg(pMsg->getMessage());
    int sender = msg.getSrcNode();
    //check for system ready
    if (mIsSystemReady)
    {
        //look for txt msg, proposed timestamp, final timestamps
        int time = std::max(mCurTime, mCurTime) + 1;
        bool isValidMsg = true;
        switch(msg.getMsgType())
        {
            case TOKEN_REQUEST:
            {
                //put in queue
                if (mHasToken and mIsIdle)
                {
                    //just pass token to neighbor
                    Message token(TOKEN_MSG, sender);
                    sendMsg(token, sender);
                    mTokenDirection = sender;
                    mHasToken = false;
                    STATUS("Got Request from " << sender << ", sending token..." );
                }
                else if (mHasToken)
                {
                    STATUS("Got Request from " << sender << ", saving request..." );
                    mRequestQueue.push(sender); //just put in queue
                }
                else if (!mHasToken)
                {
                    if (mRequestQueue.empty())
                    {
                        //send a request in direction of holder
                        Message request(TOKEN_REQUEST, mTokenDirection);
                        sendMsg(request, mTokenDirection);
                        STATUS("Got Request from " << sender << 
                               ", forwarding to " << mTokenDirection );
                    }
                    STATUS("Saving request from " << sender);
                    mRequestQueue.push(sender);
                }
                
                break;
            }//case TOKEN_REQUEST:
            case TOKEN_MSG:
            {
                //remove first entry
                int request = 0;
                if (!mRequestQueue.empty())
                {
                    request = mRequestQueue.front();
                    mRequestQueue.pop();
                    if (request == mId)
                    {
                        mHasRequest = false;
                        mHasToken = true;
                        mIsIdle = false;
                        STATUS("<<<<<    Entering critical section...    >>>>>");
                    }
                    else
                    {
                        STATUS("Forwarding token to " << request);
                        Message token(TOKEN_MSG, request);
                        sendMsg(token, request);
                        mTokenDirection = request;
                        if (!mRequestQueue.empty())
                        {
                            STATUS("Sending request for next in queue " << 
                                   mRequestQueue.front() );
                            //send another request for the next guy in the queue
                            Message requestMsg(TOKEN_REQUEST, mTokenDirection);
                            sendMsg(requestMsg, mTokenDirection);
                        }
                    }
                }
                else 
                {
                    STATUS("NO REQUESTS IN QUEUE");
                    mHasToken = true;
                    mIsIdle = true;
                }
                break;
            }//case TOKEN_MSG
            case NODE_ID:
                //do nothing;
                break;
            default:
                isValidMsg = false;
                DEBUG("UNSUPPORTED msgType=" << msg.getMsgType() << " exiting...");
                mShouldRun = false;
                break;
        }//end switch
        if (isValidMsg)
            mCurTime = time;
    }//end is system ready
    else
    {
        std::map<int, ClientSocket*>::iterator cItr = mClientList.find(sender); 
        DEBUG("Sender=" <<sender);
        if (cItr != mClientList.end())
        {
            if (msg.getMsgType() == NODE_ID  and 
                msg.getNodeId() <= mNumNodes)
            {
                DEBUG("Verified Connection to Process " << msg.getNodeId() );
                cItr->second->setSecondsToLive(-1);
                checkSystemReady();
            }//end getting NODE ID message
            else
            {
                DEBUG(__func__ << " msgType=" << msg.getMsgType() << " nodeId=" << msg.getNodeId() << " sender=" << sender);
                cItr->second->close();
            }
        }
        else
            DEBUG("Can't find sender=" << sender << " in list" );
    }
}//end Raymund::processRead(ReadMessage* pMsg)

void Raymund::processTimeToLive()
{
    FdMapItr itr = mUnknownNodeList.begin();
    for (; itr != mUnknownNodeList.end(); ++itr )
    {
        ConnSocket* pSock = itr->second;
        int curTimeToLive = pSock->getSecondsToLive();

        if (curTimeToLive == 0)
        {
            //close this unidentified socket
            DEBUG("CLOSING UNIDENTIFIED Socket ID=" << pSock->getId());
            pSock->close();
        }
        else if (curTimeToLive > 0)
        {
            pSock->setSecondsToLive((curTimeToLive-TTL_CHECK_INTERVAL));
        }
        //else if -1, ignore
    }
    //TODO: also check clients
    std::map<int, ClientSocket*>::iterator cItr = mClientList.begin();
    for (; cItr != mClientList.end(); ++cItr)
    {
        ClientSocket* pClientSock = cItr->second;
        if (pClientSock != 0)
        {
            int curTimeToLive = pClientSock->getSecondsToLive();

            if (curTimeToLive == 0)
            {
                //close this unidentified socket
                DEBUG("CLOSING CLIENT Socket ID=" << pClientSock->getId());
                pClientSock->close();
            }
            else if (curTimeToLive > 0)
            {
                pClientSock->setSecondsToLive((curTimeToLive-TTL_CHECK_INTERVAL));
            }
        }
    }
}//end Raymund::processTimeToLive

void Raymund::processUnknownRead(ReadMessage* pMsg)
{
    DEBUG("ProcessUnknownRead");
    int senderNode = pMsg->getNodeId();
    Message msg(pMsg->getMessage());
    //find the node
    FdMapItr itr = mUnknownNodeList.find(senderNode);
    ConnSocket* pConn = (itr != mUnknownNodeList.end() ? itr->second : 0);
    bool shouldRemove = true;
    if (pConn)
    {
        //if the message is the node ID, don't remove
        FdMapItr nItr = mConnList.find(msg.getNodeId());
        
        if (msg.getMsgType() == NODE_ID and nItr == mConnList.end() and 
            msg.getNodeId() <= mNumNodes)
        {
            shouldRemove = false;
            STATUS("Accepting Connection to Process " << msg.getNodeId() );
            //move to known nodes
            mConnList[msg.getNodeId()] = pConn;
            pConn->setId(msg.getNodeId());
            mUnknownNodeList.erase(itr);
            //reset callbacks
            CallBack<ReadMessage* >* pReadCallBack  = \
                                    dynamic_cast<CallBack<ReadMessage* >*>(
                                      new ObjectCallBack<Raymund, ReadMessage*>(this, 
                                                    &Raymund::processRead));
            pConn->setReadCallBack(pReadCallBack);
            CallBack<int>* pCloseCallBack  = dynamic_cast<CallBack<int>*>(
                                      new ObjectCallBack<Raymund, int>(this, 
                                                    &Raymund::processKnownClose));
            pConn->setCloseCallBack(pCloseCallBack);
            checkSystemReady();
        }//end getting NODE ID message
        else
            shouldRemove = true;

    }//end if socket exists
    if (shouldRemove and pConn)
    {
        pConn->close();
    }
}

void Raymund::processUnknownClose(int nodeId)
{
    //remove
    DEBUG("Unknown close=" << nodeId);
    std::map<int, ConnSocket*>::iterator itr = mUnknownNodeList.find(nodeId);
    ConnSocket* pConn = (itr != mUnknownNodeList.end() ? itr->second : 0);
    if (pConn)
    {
        pConn->setSelectable(false);
        removeFromList(pConn->getId());
        mUnknownNodeList.erase(itr);
        mRemoveList.push_back(pConn);
    }
}

void Raymund::processClientClose(int nodeId)
{
    STATUS("Client socket ID=" << nodeId << " closed, exiting");
    if (mIsSystemReady)
        mShouldRun = false; 
}

void Raymund::processKnownClose(int nodeId)
{
    //remove
    STATUS("Conn socket ID=" << nodeId << " closed, exiting");
    mShouldRun = false; 
    FdMapItr itr = mConnList.find(nodeId);
    ConnSocket* pConn = (itr != mConnList.end() ? itr->second : 0);
    if (pConn)
    {
        pConn->setSelectable(false);
        removeFromList(pConn->getId());
        mConnList.erase(itr);
        mRemoveList.push_back(pConn);
    }
}

void Raymund::checkSystemReady()
{
    int numOpenClients = 0;
    std::map<int, ClientSocket*>::iterator cItr = mClientList.begin();
    for (; cItr != mClientList.end(); ++cItr)
    {
        if (cItr->second->isOpen() and cItr->second->getSecondsToLive() == -1)
            numOpenClients++;
        else
            DEBUG("ClientOpen=" << cItr->second->isOpen() << 
                  " TTL=" << cItr->second->getSecondsToLive());
    }
    DEBUG("FdListSize=" << mConnList.size() << 
          " openClients=" << numOpenClients << " numNodes=" << mNumNodes <<
          " mNumNeighbors=" << mNumNeighbors);
    if ((size_t)mNumNeighbors == (mConnList.size() + numOpenClients))
    {
        STATUS("...ALL CONNECTIONS MADE...");
        //stop accepting from server
        mServerSocket.setShouldAcceptConnections(false);
        //close all unknown
        for (FdMapItr mItr = mUnknownNodeList.begin() ; 
             mItr != mUnknownNodeList.end() ; mItr++)
        {
            ConnSocket* pConnSocket = mItr->second;
            pConnSocket->close();
        }
        mUnknownNodeList.clear();
        mIsSystemReady = true;
        showPrompt();
        //mCmdLine.setNonBlocking();
        addToList(&mCmdLine);
    }
}

void Raymund::sendMsg(Message& rMsg, int destNode)
{
    //need to use forwarding table
    int fwdNeighbor = -1;
    std::map<int, int>::iterator fItr = mFwdTable.find(destNode);
    if (fItr != mFwdTable.end())
    {
        fwdNeighbor = fItr->second;
    }
    else 
    {
        STATUS("Can't send to node=" << destNode);
    }

    
    std::map<int, ClientSocket*>::iterator cItr = mClientList.find(fwdNeighbor);
    if (cItr != mClientList.end())
    {
        cItr->second->writeMsg(rMsg);
    }
    else
    {
        std::map<int, ConnSocket*>::iterator itr = mConnList.find(fwdNeighbor);
        if (itr != mConnList.end())
            itr->second->writeMsg(rMsg);
        else
            DEBUG("Can't route message, can't find destination FD");
    }
}

int main (int argc, char* argv[])
{
    Singleton<Raymund>::getInstance().run(argc, argv);
}

