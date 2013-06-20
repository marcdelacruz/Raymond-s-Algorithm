/*
 * CallBack.h
 * Marc DelaCruz
 * CS 6378-5U1 AOS Project 2 Summer 2010 
 */


#ifndef CALL_BACK_H
#define CALL_BACK_H

using namespace std;

class VoidCallBack
{
public:
    
    virtual void doCallBack() = 0;
};

class VoidStaticCallBack : public VoidCallBack
{
public:
    VoidStaticCallBack(void (*pFunc)()) { mpFunc = pFunc; }
    void doCallBack() { (*mpFunc)(); }

    VoidStaticCallBack() {}
    virtual ~VoidStaticCallBack() {}
    VoidStaticCallBack(const VoidStaticCallBack& rVoidStaticCallBack) : 
                   mpFunc(rVoidStaticCallBack.mpFunc) {}

    VoidStaticCallBack& operator=(const VoidStaticCallBack& rVoidStaticCallBack)
    {
        mpFunc = rVoidStaticCallBack.mpFunc;
        return (*this);
    }
protected:
    void (*mpFunc)();
};

/*
 * WARNING!!! If the object gets deleted then
 * the callback is called, the program will crash!
 */ 

template<class Obj>
class VoidObjectCallBack : public VoidCallBack 
{
public:
    VoidObjectCallBack(Obj* pObject, void (Obj::*pFunc)(void)) 
    { mpFunc = pFunc; mpObject = pObject; }
    virtual void doCallBack() { (mpObject->*mpFunc)(); }

    VoidObjectCallBack() {}
    virtual ~VoidObjectCallBack() {}
    VoidObjectCallBack(const VoidObjectCallBack& rVoidObjectCallBack) : 
                       mpFunc(rVoidObjectCallBack.mpFunc), 
                       mpObject(rVoidObjectCallBack.mpObject) {}

    VoidObjectCallBack& operator=(const VoidObjectCallBack& rVoidObjectCallBack)
    {
        mpFunc = rVoidObjectCallBack.mpFunc;
        mpObject = rVoidObjectCallBack.mpObject;
        return (*this);
    }

protected:
    void (Obj::*mpFunc)(void);
    Obj* mpObject;
};

/**********************************************************/

template<class T_param>
class CallBack
{
public:
    virtual void doCallBack(T_param param) = 0;
};

template<typename T_param>
class StaticCallBack : public CallBack<T_param>
{
public:
    StaticCallBack(void (*pFunc)(T_param)) { mpFunc = pFunc; }
    virtual void doCallBack(T_param param) 
    { 
        (*mpFunc)(param); 
    }
    StaticCallBack() {}
    virtual ~StaticCallBack() {}
    StaticCallBack(const StaticCallBack& rStaticCallBack) : 
                   mpFunc(rStaticCallBack.mpFunc) {}

    StaticCallBack& operator=(const StaticCallBack& rStaticCallBack)
    {
        mpFunc = rStaticCallBack.mpFunc;
        return (*this);
    }
    
protected:
    void (*mpFunc)(T_param);
};

/*
 * WARNING!!! If the object gets deleted then
 * the callback is called, the program will crash!
 */ 

template<class Obj, class T_param>
class ObjectCallBack : public CallBack <T_param>
{
public:
    ObjectCallBack(Obj* pObject, void (Obj::*pFunc)(T_param)) 
    { mpFunc = pFunc; mpObject = pObject; }
    virtual void doCallBack(T_param param) { (mpObject->*mpFunc)(param); }

    ObjectCallBack() {}
    virtual ~ObjectCallBack() {}
    ObjectCallBack(const ObjectCallBack& rObjectCallBack) : 
                   mpFunc(rObjectCallBack.mpFunc), 
                   mpObject(rObjectCallBack.mpObject) {}

    ObjectCallBack& operator=(const ObjectCallBack& rObjectCallBack)
    {
        mpFunc = rObjectCallBack.mpFunc;
        mpObject = rObjectCallBack.mpObject;
        return (*this);
    }

protected:
    void (Obj::*mpFunc)(T_param);
    Obj* mpObject;
};


#endif //CALL_BACK_H



