/*
 *  TimedAction.h
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */


#ifndef TIMED_ACTION_H 
#define TIMED_ACTION_H 

#include <ctime>
#include <csignal>
#include <cstring>
#include <iostream>
#include <map>
#include <cmath>
#include "CallBack.h"

#define TIMEDACTION_SIGNAL SIGUSR1 
#define MILLISEC_PER_SEC   1000 
#define MICROSEC_PER_SEC   1000000 
#define NANOSEC_PER_SEC    1000000000 

typedef float NANOSECONDS;
typedef float MICROSECONDS;
typedef float MILLISECONDS;
typedef float SECONDS;
typedef float MINUTES;
typedef float HOURS;

#define MICROSECONDS_TO_SECONDS(x) static_cast<SECONDS>(x / MICROSEC_PER_SEC)
#define MILLISECONDS_TO_SECONDS(x) static_cast<SECONDS>(x / MILLISEC_PER_SEC)
#define NANOSECONDS_TO_SECONDS(x)  static_cast<SECONDS>(x / NANOSEC_PER_SEC)
#define MINUTES_TO_SECONDS(x)      static_cast<SECONDS>(x * 60)

#define SECONDS_TO_NANOSECONDS(x)  static_cast<NANOSECONDS>(x * NANOSEC_PER_SEC)
#define EXTRACT_SECONDS(x)         static_cast<long int>(floor(x))
#define EXTRACT_NANOSECONDS(x)     static_cast<long int>((x-floor(x))*NANOSEC_PER_SEC)


class TimedAction
{ 
public: 
    TimedAction(VoidCallBack* pCallBack, SECONDS interval,
                int actionId, bool isRecurrent); 

    ~TimedAction(); 
    void callEmit(); 
    int getActionId(); 
    void setReadyToEmit(); 
    void setNotReadyToEmit(); 
    void setShouldDelete(); 
    bool shouldDelete(); 

protected: 
    TimedAction(); 
    TimedAction(const TimedAction & rTimedAction); 
    TimedAction & operator=(const TimedAction & rTimedAction); 
    void disconnect(); 
    void connect(); 
    void startTimer(); 
    void stopTimer(); 
    void setActionId(int actionId); 

    bool mIsRecurrent; 
    bool mIsTimerPresent; 
    bool mIsReadyToEmit; 
    int mActionId; 
    SECONDS mTimeInterval; 
    timer_t mTimer; 
    VoidCallBack* mpCallBack;
    bool mShouldDelete; 
}; //class TimedAction

#endif //TIMED_ACTION_H 


