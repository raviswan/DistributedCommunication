#include "tcpClientThread.h"

static double testMessageCount = 0;
static double totalBytes = 0.0;
//For BW measurement
static double bwCounter = 1;
static double bwSum = 0.0;


double subtractTimeVal(struct timeval *currTime,struct timeval *prevTime){
	double timeDiff;
	timeDiff = currTime->tv_sec - prevTime->tv_sec;
	timeDiff += (currTime->tv_usec - prevTime->tv_usec)/1000000.0;
	return timeDiff;
}


TcpClientThread::TcpClientThread():Thread(){
	mFDMax = 0;
	FD_ZERO(&mFDSet);
	mRTT = -1;
	mBW = -1;
};

TcpClientThread::~TcpClientThread(){
}; 

void TcpClientThread::measureNeighborLatency(string ipAddr, 
	struct timeval *prevTime, struct timeval *currentTime){
	double currentRTT = subtractTimeVal(currentTime,prevTime);
    if(mRTT < 0)
    	mRTT = currentRTT;
    else
    	//weighted average
        mRTT = 0.8*mRTT + 0.2*currentRTT;
    mTestNodePtr->mNeighbors[ipAddr].linkRTT = mRTT;
}

void TcpClientThread::sendPacketToServer(int sock) {
    bzero(buf,BUF_SIZE);
    sprintf(buf, "Test message %g\n",testMessageCount++);
    if(send(sock, buf, strlen(buf), 0) < 0){
        perror("Error sendPacketToServer()");
        return;
    }
}

void TcpClientThread::measureNeighborBandwidth(string ipAddr,double *bytesRcvd,
	struct timeval *firstPktTime,struct timeval* currentTime) {
	/*For tx and receive, so twice*/
	totalBytes += (*bytesRcvd * 2);
	double totalTimeTaken = subtractTimeVal(currentTime,firstPktTime);
	double currentBW = totalBytes/totalTimeTaken;
	if (mBW <0)
		mBW = currentBW;
	else{
		bwCounter++;
		bwSum += currentBW;
		//averaging BW with time
		mBW = bwSum/bwCounter;
	}
	mTestNodePtr->mNeighbors[ipAddr].linkBW = mBW;
}

void* TcpClientThread::run(void *arg){
	TcpClientThreadArg* threadArg = (TcpClientThreadArg*) arg;
	mTestNodePtr = threadArg->ctNode;
	string ipAddr = threadArg->ipStr;
	//Get the socket to talk to the server
	int fd = mTestNodePtr->mNeighbors[ipAddr].tcpConnSocket;
	int retVal;
	int firstPktFlag = 0;
	double bytesRcvd = 0.0;
	sockaddr_in dstSrvAddr;
	struct timespec timeout;
    struct timeval startTime,currTime,firstPktTime,nthPktTime;
    startTime.tv_sec = 0;
    startTime.tv_usec = 0;
    currTime.tv_sec = 0;
    currTime.tv_usec = 0;
    firstPktTime.tv_sec = 0;
    firstPktTime.tv_usec = 0;
    nthPktTime.tv_sec = 0;
    nthPktTime.tv_usec = 0;

 
	while(1){
		timeout.tv_sec = SOCKET_TIMEOUT;
		timeout.tv_nsec = 0;
		FD_ZERO(&mFDSet);
		FD_SET(fd,&mFDSet);
		if(fd >= mFDMax)
			mFDMax = fd + 1; 
		gettimeofday(&startTime,NULL);
        sendPacketToServer(fd);

		if (pselect(mFDMax,&mFDSet,NULL,NULL,&timeout,NULL) > 0 ){
			if(FD_ISSET(fd,&mFDSet)){
				gettimeofday(&currTime,NULL);
            	bytesRcvd = read(fd,buf,BUF_SIZE);
				measureNeighborLatency(ipAddr,&startTime,&currTime);
				if(!firstPktFlag){
					/*serves as base time for BW calculation*/
					gettimeofday(&firstPktTime,NULL);
					firstPktFlag = 1;
				}
				else
					measureNeighborBandwidth(ipAddr,&bytesRcvd,&firstPktTime,
						&currTime);

			}
		}
	}		
}