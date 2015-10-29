#include "tcpConnServerThread.h"

static double totalBytes = 0.0;
TcpConnServerThread::TcpConnServerThread():Thread(){
	mFDMax = 0;
	FD_ZERO(&mFDSet);
};

TcpConnServerThread::~TcpConnServerThread(){
}; 

void* TcpConnServerThread::run(void *arg){
	TcpConnServerThreadArg* threadArg = (TcpConnServerThreadArg*) arg;
	int fd= threadArg->connectedSocket;
	double bytesRcvd = 0.0;
	int retVal;
	string ipAddr = threadArg->ipAddress;
	struct timespec timeout;
	timeout.tv_sec = SOCKET_TIMEOUT;
	timeout.tv_nsec = 0;
	FD_ZERO(&mFDSet);
	FD_SET(fd,&mFDSet);
	if(fd >= mFDMax)
		mFDMax = fd + 1; 

	while(pselect(mFDMax,&mFDSet,NULL,NULL,&timeout,NULL) > 0){
		if(FD_ISSET(fd,&mFDSet)){
        	if ((bytesRcvd = read(fd,buf,BUF_SIZE)) > 0){
        		retVal = send(fd, buf, strlen(buf), 0);
        		if (retVal < 0)
        			perror("tcpConnServver send() failed");
        	}
        	else	
        		perror("tcpConnServver read() failed");

		}		

	}
	return NULL;
}