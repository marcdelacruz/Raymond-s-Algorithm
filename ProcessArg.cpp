/*
 *  ProcessArg.cpp
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#include "ProcessArg.h"

ProcessArg::ProcessArg() : mIsRequired(REQUIRED_ARG),  
                           mHasNoInputRequired(NO_INPUT_NEEDED), 
                           mWasArgProcessed(false), mParamName("")
{
} 

ProcessArg::ProcessArg(string argPrefix, string helpString, 
                       CallBack<string>* pCallBack,
                       bool isRequired /*= REQUIRED_ARG*/,  
                       bool hasNoInputRequired/* = NO_INPUT_NEEDED*/,
                       string paramName /* = "" */) :
                       mArgPrefix(argPrefix), mHelpString(helpString),
                       mpCallBack(pCallBack), mIsRequired(isRequired),
                       mHasNoInputRequired(hasNoInputRequired), 
                       mWasArgProcessed(false), mParamName(paramName)
{
}

ProcessArg::~ProcessArg()
{
}

void ProcessArg::performCallBack(string argString)
{
    mpCallBack->doCallBack(argString);
    mWasArgProcessed = true;
}

ProcessArg::ProcessArg(const ProcessArg& rProcessArg) :
           mArgPrefix(rProcessArg.mArgPrefix), 
           mHelpString(rProcessArg.mHelpString),
           mpCallBack(rProcessArg.mpCallBack),
           mIsRequired(rProcessArg.mIsRequired),
           mHasNoInputRequired(rProcessArg.mHasNoInputRequired),
           mWasArgProcessed(rProcessArg.mWasArgProcessed),
           mParamName(rProcessArg.mParamName)
{
}

ProcessArg& ProcessArg::operator=(const ProcessArg& rProcessArg)
{
    mArgPrefix = rProcessArg.mArgPrefix;
    mHelpString = rProcessArg.mHelpString;
    mpCallBack = rProcessArg.mpCallBack;
    mIsRequired = rProcessArg.mIsRequired;
    mHasNoInputRequired = rProcessArg.mHasNoInputRequired;
    mWasArgProcessed = rProcessArg.mWasArgProcessed;
    mParamName = rProcessArg.mParamName;
    return (*this);
}

