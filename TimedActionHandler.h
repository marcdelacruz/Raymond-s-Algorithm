/*
 *  TimedActionHandler.h
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#ifndef TIMED_ACTION_HANDLER_H
#define TIMED_ACTION_HANDLER_H

#include <utility>
#include <climits>
#include "TimedAction.h"
#include "Singleton.h"

using std::map;
using std::make_pair;

class TimedAction;

/** 
 * TimedActionHandler controls and manages the TimedAction objects. 
 */ 
class TimedActionHandler : public Singleton<TimedActionHandler>
{
public:
    static void handleSignal (int sigNum, siginfo_t* pSigInfo, 
                              void *pContext);
    virtual int addTimedEvent(VoidCallBack* pCallBack, 
                              SECONDS interval, bool isRecurrent);
    virtual void deleteTimedEvent(int timedActionId);
    virtual void processTimedActions();
    friend class Singleton<TimedActionHandler>;

protected:
    TimedActionHandler();
    virtual ~TimedActionHandler();

    
    TimedActionHandler(const TimedActionHandler & rTimedActionHandler);
    TimedActionHandler & operator=(const TimedActionHandler &
                                   rTimedActionHandler);

    int mActionCtr;
    map<int, TimedAction* > mTimedActionMap;
};

class Timer
{
public:
    Timer() : mTimedActionId(-1), mpCallBack(0), mInterval(0), 
              mIsRecurrent(false), mCallBack(this, &Timer::callBack)
    {
    }
    
    explicit Timer(int timedActionId) : mTimedActionId(timedActionId), 
                                        mpCallBack(0), mInterval(0), 
                                        mIsRecurrent(false), 
                                        mCallBack(this, &Timer::callBack)
    { 
    }

    Timer(const Timer& rTimer) : mTimedActionId(rTimer.mTimedActionId),
                                 mpCallBack(rTimer.mpCallBack),
                                 mInterval(rTimer.mInterval), 
                                 mIsRecurrent(rTimer.mIsRecurrent),
                                 mCallBack(this, &Timer::callBack)
    {
    }
    
    Timer& operator=(const Timer& rTimer) 
    {
        mTimedActionId = rTimer.mTimedActionId;
        mpCallBack = rTimer.mpCallBack;
        mInterval = rTimer.mInterval;
        mIsRecurrent = rTimer.mIsRecurrent;
        return (*this);
    }

    Timer& operator=(int timedActionId) 
    {
        mTimedActionId = timedActionId; 
        return (*this);
    }

    void create(VoidCallBack* pCallBack, SECONDS interval, bool isRecurrent)
    {
        mpCallBack = pCallBack;
        mInterval = interval;
        mIsRecurrent = isRecurrent;
    }
    
    void start(VoidCallBack* pCallBack, SECONDS interval, bool isRecurrent)
    {
        create(pCallBack, interval, isRecurrent);
        start();
    }
    
    void start()
    {
        stop();
        TimedActionHandler& rHandler = Singleton<TimedActionHandler>::getInstance();
        mTimedActionId = rHandler.addTimedEvent((VoidCallBack*)&mCallBack, 
                                                mInterval, mIsRecurrent);
    }
    
    void stop()
    {
        if (mTimedActionId != -1)
            Singleton<TimedActionHandler>::getInstance().deleteTimedEvent(mTimedActionId);
        mTimedActionId = -1;
    }
    
protected:
    void callBack()
    {
        if (mpCallBack != 0)
            mpCallBack->doCallBack();
        start();//restart
    }

    int mTimedActionId;
    VoidCallBack* mpCallBack;
    SECONDS mInterval;
    bool mIsRecurrent;
    VoidObjectCallBack<Timer> mCallBack;
};

#endif //TIMED_ACTION_HANDLER_H

