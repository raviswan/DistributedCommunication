#ifndef MCAST_UDP_SERVER_THREAD_H
#define MCAST_UDP_SERVER_THREAD_H

#include "commTestNode.h"
#include "thread.h"

class CommTestNode;

typedef struct _McastUdpServerThreadArg{
	CommTestNode *ctNode;
} McastUdpServerThreadArg;

class McastUdpServerThread : public Thread{

public:
	McastUdpServerThread();
	~McastUdpServerThread();
protected:
	void* run(void* arg);
private: 
	char mBuf[BUF_SIZE];
	fd_set mFDSet;
	int mFDMax;

};

#endif