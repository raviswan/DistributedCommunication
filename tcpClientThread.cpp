#include "tcpClientThread.h"

double subtractTimeVal(struct timeval *currTime,struct timeval *prevTime){
	double timeDiff;
	timeDiff = currTime->tv_sec - prevTime->tv_sec;
	timeDiff += (currTime->tv_usec - prevTime->tv_usec)/1000000.0;
	return timeDiff;
}

NetworkStatsFuncType TcpClientThread::fnPtr = 0;

TcpClientThread::TcpClientThread():Thread(){
	mFDMax = 0;
	FD_ZERO(&mFDSet);
	mRTT = -1;
	mBW = -1;
	testMessageCount = 0;
	totalBytes = 0.0;
	bwCounter = 1;
	bwSum = 0.0;
};

TcpClientThread::~TcpClientThread(){
	if(mCharBuf)
		delete mCharBuf;
}; 

void TcpClientThread::RegisterNetworkStatsHandler(NetworkStatsFuncType ptr){
	fnPtr = ptr;
}

void TcpClientThread::measureNeighborLatency(string ipAddr, 
	struct timeval *prevTime, struct timeval *currentTime){
	double currentRTT = subtractTimeVal(currentTime,prevTime);
    if(mRTT < 0)
    	mRTT = currentRTT;
    else
    	//weighted average
        mRTT = 0.8*mRTT + 0.2*currentRTT;
    pthread_mutex_lock( &mTestNodePtr->mNeighborsMutex);
    //RTT in msec
    mTestNodePtr->mNeighbors[ipAddr].linkRTT = mRTT*1000;
    pthread_mutex_unlock( &mTestNodePtr->mNeighborsMutex);
}

//void TcpClientThread::sendPacketToServer(int sock) {
    //bzero(buf,BUF_SIZE);
   
   // sprintf(buf, "Test message %ld\n",testMessageCount++);
    //if(send(sock, buf, strlen(buf), 0) < 0){
   
  
//}

void TcpClientThread::measureNeighborBandwidth(string ipAddr,double *bytesRcvd,
	struct timeval *firstPktTime,struct timeval* currentTime) {
	/*For tx and receive, so twice*/
	totalBytes += (*bytesRcvd * 2);
	double totalTimeTaken = subtractTimeVal(currentTime,firstPktTime);
	double currentBW = (totalBytes)/(1024)/totalTimeTaken;
	if (mBW <0)
		mBW = currentBW;
	else{
		bwCounter++;
		bwSum += currentBW;
		//averaging BW with time
		mBW = bwSum/bwCounter;
	}
	/* If REPORTING_INTERVAL=50, the BW/RTT values are reported  every 50 reads*/
	if(!(++testMessageCount%REPORTING_INTERVAL)){
		pthread_mutex_lock( &mTestNodePtr->mNeighborsMutex);
		/*BW in Kbits/sec*/
		mTestNodePtr->mNeighbors[ipAddr].linkBW = mBW*8;
		pthread_mutex_unlock( &mTestNodePtr->mNeighborsMutex);
		fnPtr(mTestNodePtr);
	}
}

void* TcpClientThread::run(void *arg){
	TcpClientThreadArg* threadArg = (TcpClientThreadArg*) arg;
	mTestNodePtr = threadArg->ctNode;
	string ipAddr = threadArg->ipStr;
	mCharBuf = new char[TX_DATA_SIZE];
    memset(mCharBuf,'-',TX_DATA_SIZE);
	//Get the socket that talks to the server
	int fd = mTestNodePtr->mNeighbors[ipAddr].tcpConnSocket;
	int retVal;
	int firstPktFlag = 0;
	double bytesRcvd = 0.0;
	struct timespec timeout;
    struct timeval startTime,currTime,firstPktTime;
    startTime.tv_sec = 0;
    startTime.tv_usec = 0;
    currTime.tv_sec = 0;
    currTime.tv_usec = 0;
    firstPktTime.tv_sec = 0;
    firstPktTime.tv_usec = 0;
 
	while(1){
		timeout.tv_sec = SOCKET_TIMEOUT;
		timeout.tv_nsec = 0;
		FD_ZERO(&mFDSet);
		FD_SET(fd,&mFDSet);
		if(fd >= mFDMax)
			mFDMax = fd + 1; 
		gettimeofday(&startTime,NULL);
        //sendPacketToServer(fd);
        if(send(fd, mCharBuf, sizeof(mCharBuf), 0) < 0){
	        perror("Error sendPacketToServer()");
	        //goto ENDCLIENT;
	        continue;
    	}
        retVal = pselect(mFDMax,&mFDSet,NULL,NULL,&timeout,NULL);
		if (retVal> 0){
			if(FD_ISSET(fd,&mFDSet)){
				bzero(buf,BUF_SIZE);
				gettimeofday(&currTime,NULL);
            	bytesRcvd = read(fd,buf,BUF_SIZE);
				measureNeighborLatency(ipAddr,&startTime,&currTime);
				if(!firstPktFlag){
					/*serves as base time for BW calculation*/
					gettimeofday(&firstPktTime,NULL);
					firstPktFlag = 1;
				}
				else{
					measureNeighborBandwidth(ipAddr,&bytesRcvd,&firstPktTime,
						&currTime);
				}

			}
		}
		else if(retVal==0){
			printf("pselect() expired. TCP Server isn't responding. Goodbye!");
			break;
		}
		else{
			perror("tcpClientThread pselect()");
			break;
		}
	}
ENDCLIENT:	
	FD_CLR(fd, &mFDSet);
	mTestNodePtr->removeNeighbor(ipAddr);
	if(threadArg!=NULL)
		delete threadArg;
	return NULL;	
}