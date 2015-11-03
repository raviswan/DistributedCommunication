#ifndef TCP_CLIENT_THREAD_H
#define TCP_CLIENT_THREAD_H

#include "commTestNode.h"
#include "thread.h"

typedef void (*NetworkStatsFuncType)(void *);

class CommTestNode;

typedef struct _TcpClientThreadArg{
	CommTestNode *ctNode;
	string ipStr;
} TcpClientThreadArg;

class TcpClientThread : public Thread {

public:
	TcpClientThread();
	~TcpClientThread();
	void measureNeighborLatency(string ipAddr, struct timeval *prevTime,
	 struct timeval *currentTime);
	void measureNeighborBandwidth(string ipAddr,double *bytesRcvd,
	struct timeval *firstPktTime,struct timeval* currTime);
	//void sendPacketToServer(int sock);
	static void RegisterNetworkStatsHandler(NetworkStatsFuncType fPtr);
protected:
	void* run(void* arg); 
private:
	CommTestNode *mTestNodePtr;
	fd_set mFDSet;
	int mFDMax;
	char buf[BUF_SIZE];
	char *mCharBuf;
	double mRTT;
	double mBW;
	//For BW measurement
	double totalBytes;
	double bwCounter;
	double bwSum;
	unsigned long long testMessageCount;
	static NetworkStatsFuncType fnPtr;

};

#endif