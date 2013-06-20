/*
 *  TimedActionHandler.cpp
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#include "TimedActionHandler.h" 

void TimedActionHandler::handleSignal (int sigNum, 
                                       siginfo_t* pSigInfo, 
                                       void *pContext)
{
    Singleton<TimedActionHandler>::getInstance();//ensure an instance is active
    //just put the siginfo in the queue;
    if (sigNum == TIMEDACTION_SIGNAL &&
        pSigInfo->si_value.sival_ptr != 0)
    {
        TimedAction* pTimedAction = 
                  (TimedAction*)pSigInfo->si_value.sival_ptr;
        pTimedAction->setReadyToEmit();
    }
} 

int TimedActionHandler::addTimedEvent(VoidCallBack* pCallBack,
                                      NANOSECONDS interval, bool isRecurrent) 
{
    bool hasFoundId = false; 

    mActionCtr = ((mActionCtr+1) < 0) ? 0 : mActionCtr+1;
    for (; mActionCtr < INT_MAX; mActionCtr++)
    {
        /* 
         * If we can't find a TimedAction object for this ID 
         * then we can use it. If not increment the mActionCtr until 
         * we find one that isn't used. 
         */
        map<int, TimedAction*>::iterator itr = 
                             mTimedActionMap.find(mActionCtr);
        if (itr == mTimedActionMap.end())
        {
            hasFoundId = true;
            break;
        }
    } 

    if (hasFoundId)
    {
        mTimedActionMap.insert(
                    make_pair(mActionCtr,
                                 new TimedAction(pCallBack, interval, 
                                                 mActionCtr, isRecurrent)));
        return mActionCtr;
    }
    else
    {
        return -1;
    } 
}//end TimedActionHandler::addTimedEvent 

void TimedActionHandler::deleteTimedEvent(int timedActionId) 
{
    /* 
     * look for the timed action based on the ID 
     * delete the TimedAction if found 
     */
    map<int, TimedAction*>::iterator itr = 
                           mTimedActionMap.find(timedActionId);
    if (itr != mTimedActionMap.end())
    {
        TimedAction *pTimedAction = itr->second;
        if (pTimedAction != 0)
            pTimedAction->setShouldDelete(); 
    } 
} 

void TimedActionHandler::processTimedActions() 
{
    /* 
     * callEmit for each element in the map 
     */ 
    TimedAction* pTimedAction = 0; 
    for(map<int, TimedAction* >::iterator iter = mTimedActionMap.begin(); 
        iter != mTimedActionMap.end(); ++iter)
    {
        pTimedAction = iter->second;
        if (pTimedAction != 0)
        {
            if (pTimedAction->shouldDelete())
            {
                delete pTimedAction;
                mTimedActionMap.erase(iter);
            }
            else
                pTimedAction->callEmit();
        }
    } 
} 

TimedActionHandler::TimedActionHandler() : mActionCtr(0) 
{
    //register as the signal handler here
    struct sigaction sigAction;
    memset(&sigAction, 0, sizeof(sigAction)); 
    
    sigAction.sa_sigaction = TimedActionHandler::handleSignal;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = (SA_SIGINFO | SA_RESTART);
    if (sigaction(TIMEDACTION_SIGNAL, &sigAction, NULL) == -1)
    {
        std::cerr << __FILE__ << "(" << __LINE__ << "): "
                  << "TimedActionHandler contructor: sigaction failed" 
                  << std::endl;
    }
} 

TimedActionHandler::TimedActionHandler(const TimedActionHandler & 
                                       rTimedActionHandler) : 
                                       mActionCtr(0) 
{
} 

TimedActionHandler::~TimedActionHandler() 
{
    TimedAction* pTimedAction = 0;
    for(map<int, TimedAction *>::iterator iter = mTimedActionMap.begin();
        iter != mTimedActionMap.end(); ++iter ) 
    {
        if ((pTimedAction = iter->second) != 0)
            delete pTimedAction;
    }
    mTimedActionMap.clear(); 
} 

TimedActionHandler & TimedActionHandler::operator=(const TimedActionHandler &
                                                   rTimedActionHandler) 
{
    return (*this); 
} 

