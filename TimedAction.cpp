/*
 *  TimedAction.cpp
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010
 */

#include "TimedAction.h"
#include <cerrno>

TimedAction::TimedAction(VoidCallBack* pCallBack, SECONDS interval,
                         int actionId, bool isRecurrent) :
                         mIsTimerPresent(false),
                         mIsReadyToEmit(false), mActionId(actionId),
                         mTimeInterval(interval), mpCallBack(pCallBack),
                         mShouldDelete(false)
{
    startTimer();
}

TimedAction::~TimedAction()
{
    disconnect();
    stopTimer();
}

void TimedAction::callEmit()
{
    if (mIsReadyToEmit)
    {
        mpCallBack->doCallBack();
        setNotReadyToEmit();
    }
}

int TimedAction::getActionId()
{
    return mActionId;
}

void TimedAction::setReadyToEmit()
{
    mIsReadyToEmit = true;
}

void TimedAction::setNotReadyToEmit()
{
    mIsReadyToEmit = false;
}

void TimedAction::setShouldDelete()
{
   mShouldDelete = true;
}

bool TimedAction::shouldDelete()
{
    return (mShouldDelete);
}

TimedAction::TimedAction()
{
}

TimedAction::TimedAction(const TimedAction& rTimedAction)
{
}

TimedAction& TimedAction::operator=(const TimedAction& rTimedAction)
{
    return (*this);
}

void TimedAction::disconnect()
{
    setNotReadyToEmit();   
}

void TimedAction::connect()
{
}

void TimedAction::startTimer()
{
    sigevent sigEv;
    int retCode = 0;
    memset(&sigEv, 0, sizeof(sigEv));
    sigEv.sigev_notify = SIGEV_SIGNAL;
    sigEv.sigev_signo = TIMEDACTION_SIGNAL;
    sigEv.sigev_value.sival_ptr = (void *)this;

    if (retCode = timer_create(CLOCK_REALTIME, &sigEv, &mTimer))
    {
        //error out
        std::cerr << __FILE__ << "(" << __LINE__ << ")" 
                  << ": timer_create failed" << std::endl;
    }
    else
    {
        itimerspec timeout;
        memset(&timeout, 0, sizeof(timeout));
        mIsTimerPresent = true;

        if (mTimeInterval < 1 )//less than 1 second
        {
            timeout.it_value.tv_nsec = 
                    static_cast<long int>(SECONDS_TO_NANOSECONDS(mTimeInterval));
            if (mIsRecurrent)
                timeout.it_interval.tv_nsec = 
                   static_cast<long int>(SECONDS_TO_NANOSECONDS(mTimeInterval));
        }
        else
        {
            long int secs = EXTRACT_SECONDS(mTimeInterval);
            long int nsecs = EXTRACT_NANOSECONDS(mTimeInterval);
            timeout.it_value.tv_sec = secs;
            timeout.it_value.tv_nsec = nsecs;

            if (mIsRecurrent)
            {
                timeout.it_interval.tv_sec = secs;
                timeout.it_interval.tv_nsec = nsecs;
            }
        }
        if ((retCode = timer_settime(mTimer, 0, &timeout, 0)) == -1)
        {
            std::cerr << __FILE__ << "(" << __LINE__ << ")" 
                      << ": timer_settime failed " << strerror(errno) 
                      << " tv_sec=" << timeout.it_value.tv_sec 
                      << " tv_nsec=" << timeout.it_value.tv_nsec<< std::endl;
        }
    }
}//end startTimer()

void TimedAction::stopTimer()
{
    if (mIsTimerPresent)
        timer_delete(mTimer);
}
 
void TimedAction::setActionId(int actionId)
{
    mActionId = actionId;
}

