/*
 *  ProcessArg.h
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 */

#ifndef PROCESS_ARG_H
#define PROCESS_ARG_H

#include <string>
#include "CallBack.h"

using std::string;

class ProcessArg
{
public:
    #define OPTIONAL_ARG  false
    #define REQUIRED_ARG  true
    #define INPUT_NEEDED  false
    #define NO_INPUT_NEEDED  true   
    //add for blank arguments
    ProcessArg();
    ProcessArg(string argPrefix, string helpString, CallBack<string>* pCallBack,
               bool isRequired = REQUIRED_ARG,  
               bool hasNoInputRequired = NO_INPUT_NEEDED,
               string paramName = "");
    ~ProcessArg();
    string getArgPrefix() const { return mArgPrefix; }
    string getHelpString() const { return mHelpString; }
    void performCallBack(string argString);
    bool isRequired() const { return mIsRequired; }
    bool hasNoInputRequired() const { return mHasNoInputRequired; }
    bool wasArgProcessed() const { return mWasArgProcessed; }
    string getParamName() const { return mParamName; }

    ProcessArg(const ProcessArg& rProcessArg);
    ProcessArg& operator=(const ProcessArg& rProcessArg);

protected:
    string mArgPrefix;
    string mHelpString;
    CallBack<string>* mpCallBack;
    bool mIsRequired;
    bool mHasNoInputRequired;
    bool mWasArgProcessed;
    string mParamName;
};

#endif //PROCESS_ARG_H

