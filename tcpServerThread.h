#ifndef TCP_SERVER_THREAD_H
#define TCP_SERVER_THREAD_H

#include "commTestNode.h"
#include "thread.h"

class CommTestNode;

typedef struct _TcpServerThreadArg{
	CommTestNode *ctNode;
} TcpServerThreadArg;

class TcpServerThread : public Thread{

public:
	TcpServerThread();
	~TcpServerThread();
protected:
	void* run(void* arg);

};

#endif