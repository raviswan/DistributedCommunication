#ifndef TCP_CLIENT_THREAD_H
#define TCP_CLIENT_THREAD_H

#include "commTestNode.h"
#include "thread.h"

class CommTestNode;

typedef struct _TcpClientThreadArg{
	CommTestNode *ctNode;
	string ipStr;
} TcpClientThreadArg;

class TcpClientThread : public Thread {

public:
	TcpClientThread();
	~TcpClientThread();
protected:
	void measureNeighborLatency(string ipAddr, struct timeval *prevTime,
	 struct timeval *currentTime);
	void measureNeighborBandwidth(string ipAddr,double *bytesRcvd,
		struct timeval *firstPktTime,struct timeval* currTime);
	void sendPacketToServer(int sock);
	void* run(void* arg); 
private:
	CommTestNode *mTestNodePtr;
	fd_set mFDSet;
	int mFDMax;
	char buf[BUF_SIZE];
	double mRTT;
	double mBW;

};

#endif