#ifndef Singleton_H
#define Singleton_H

// use:
//
// Class Declaration Example:-
// ===========================
//
// class WiiManager : private SingletonClient 
// { 
//    public:
//
//	  private:
//
// }
//
//Definition Example, calling code:-
//==================================
//
//	WiiManager& Wii ( Singleton<WiiManager>::GetInstanceByRef() );


template <class _template>
class Singleton : private _template
{
private:
    Singleton() {}
public:
    static _template& GetInstanceByRef()
    {
        static _template inst;
        return inst;
    }
    static _template* GetInstanceByPtr()
    {
        _template& inst = GetInstanceByRef();
        return &inst;
    }
};

// This class will help prevent assignment or copying of singletons.
class SingletonClient
{
private:
    SingletonClient( const SingletonClient& sc ) { } // never called.
    SingletonClient operator=( const SingletonClient& sc ) {  return *this; }  // Never called
public:
    SingletonClient() {}
};


#endif 

