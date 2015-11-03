#ifndef MCAST_UDP_CLIENT_THREAD_H
#define MCAST_UDP_CLIENT_THREAD_H

#include "commTestNode.h"
#include "thread.h"

class CommTestNode;

typedef struct _McastUdpClientThreadArg{
	CommTestNode *ctNode;
} McastUdpClientThreadArg;

class McastUdpClientThread : public Thread{

public:
	McastUdpClientThread();
	~McastUdpClientThread();
protected:
	void* run(void* arg);
private:
	

};

#endif