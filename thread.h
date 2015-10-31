#ifndef THREAD_CPLUSPLUS_H_
#define THREAD_CPLUSPLUS_H_

#include <iostream>
#include <exception>
#include <pthread.h>

class Thread
{
public:
    Thread();
    virtual ~Thread(); 
    const pthread_t& getThreadID() const;
    void join();
    virtual void* run(void* arg) = 0;
    virtual void start(void *arg);
    
private:
    Thread(const Thread&);               
    Thread& operator=(const Thread&); 
    static void* threadFunc(void* arg); 
    //The thread argument passed to Start()  
    void *derivedObjArg; 
    bool hasJoined;
    pthread_t mThreadID;
};

#endif