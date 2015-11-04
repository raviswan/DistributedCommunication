#include "thread.h"

Thread::Thread():hasJoined(false){
}

Thread::~Thread(){
    join();
}

void Thread::start(void *arg){
    this->derivedObjArg = arg;
    int retVal = pthread_create(&mThreadID, 0, threadFunc,this);
    if (retVal){
        printf("Failed to create Thread\n");
        exit(1);
    }
    return;
}

void Thread::join(){
    int retVal;
    if(mThreadID == pthread_self()){
        hasJoined = true;
        retVal = pthread_detach(mThreadID);
        if(retVal ==0)
            printf("thread successfully detached\n");
        else
            perror("pthread_detach()");
    }
    else{
        if(!hasJoined){
            hasJoined = true;
            retVal = pthread_join(mThreadID,NULL);
            if(retVal ==0)
                printf("thread successfully joined\n");
            else
                perror("pthread_join()");
        }
    }
}


const pthread_t& Thread::getThreadID() const { 
    return mThreadID; 
}

void* Thread::run(void* arg){
    printf("Thread run() should not be\n");
    return NULL;
}

void* Thread::threadFunc(void* arg){
    Thread* t = static_cast<Thread*>(arg);
    return t->run(t->derivedObjArg);
}

