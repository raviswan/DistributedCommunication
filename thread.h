#ifndef THREAD_CPLUSPLUS_H_
#define THREAD_CPLUSPLUS_H_

#include <iostream>
#include <exception>
#include <pthread.h>

class Thread
{
public:
    Thread();
    virtual ~Thread() { }
    const pthread_t& getThreadID() const;
    bool join();
    void* run(void* arg);
    static void* threadProc(void* arg);

private:
    Thread(const Thread&);               
    Thread& operator=(const Thread&);    
    pthread_t m_tid;
};

#endif