#include "tcpConnServerThread.h"


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
	FD_ZERO(&mFDSet);
	FD_SET(fd,&mFDSet);
	if(fd >= mFDMax)
		mFDMax = fd + 1; 

	while(1){
		timeout.tv_sec = SOCKET_TIMEOUT;
		timeout.tv_nsec = 0;
		retVal = pselect(mFDMax,&mFDSet,NULL,NULL,&timeout,NULL);
		if(retVal > 0){
			if(FD_ISSET(fd,&mFDSet)){
				bzero(buf,BUF_SIZE);
	        	if ((bytesRcvd = read(fd,buf,BUF_SIZE)) > 0){
	        		/*echo the received packet from tcp client*/
	        		retVal = write(fd, buf, strlen(buf));
	        		if (retVal < 0){
	        			perror("tcpConnServver write() failed");
	        			break;
	        		}
	        	}
	        	else{	
	        		perror("tcpConnServver read() failed");
	        		break;
	        	}
			}		
		}
		else if(retVal==0){
			printf("tcpConnServer timeout expired. Gonna die. Goodbye!");
			break;
		}
		else{
			perror("TcpConnserverThread:pselect() failure");
			break;
		}
	}
	FD_CLR(fd, &mFDSet);
	if(threadArg!=NULL)
		delete threadArg;	
	return NULL;
}