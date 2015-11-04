#ifndef THREAD_CPLUSPLUS_H_
#define THREAD_CPLUSPLUS_H_

#include <iostream>
#include <exception>
#include <pthread.h>
#include <cstdlib>
#include <cstdio>

/*A wrapper class for POSIX threads*/
class Thread
{
public:
    Thread();
    virtual ~Thread(); 
    const pthread_t& getThreadID() const;
    void join();
    /*To be implemented by subclasses*/
    virtual void* run(void* arg);
    /*Store new thread's args and call a static function
    to start the thred. Not defined in subclasses.*/
    virtual void start(void *arg);
    
private:
    Thread(const Thread&);               
    Thread& operator=(const Thread&); 
    /*static func for creating thread*/
    static void* threadFunc(void* arg); 
    //The thread argument for the new thread 
    void *derivedObjArg; 
    /*flag for whether a thread has already joined or not*/
    bool hasJoined;
    pthread_t mThreadID;
};

#endif