#ifndef TCP_CONN_SERVER_THREAD_H
#define TCP_CONN_SERVER_THREAD_H

#include "commTestNode.h"
#include "thread.h"

class CommTestNode;

typedef struct _TcpConnServerThreadArg{
	int connectedSocket;
	string ipAddress;
	CommTestNode *ctNode;
} TcpConnServerThreadArg;

class TcpConnServerThread : public Thread{

public:
	TcpConnServerThread();
	~TcpConnServerThread();
protected:
	void* run(void* arg);
	fd_set mFDSet;
	int mFDMax;
	char buf[BUF_SIZE];

};

#endif