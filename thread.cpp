#include "thread.h"

Thread::Thread()
{
    int retVal = ::pthread_create(
            &m_tid, 
            0, 
            threadProc, 
            static_cast<void*>(this)
        );
    if (retVal){
        printf("Failed to create Thread\n");
    }
}

bool Thread::join()
{
    // pthread_join will detect attempts to join a Thread to itself
    int retVal = ::pthread_join(getThreadID(), 0);
    return (retVal == 0);
}

const pthread_t& Thread::getThreadID() const { 
    return m_tid; 
}

void* Thread::threadProc(void* arg)
{
    Thread* pthread = static_cast<Thread*>(arg);
    return pthread->run(arg);
}

void* Thread::run(void *arg)
{
    std::cout << "Hello from Thread" << std::endl;
    return 0;
}

