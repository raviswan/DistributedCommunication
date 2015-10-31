#include "tcpServerThread.h"
#include "tcpConnServerThread.h"

TcpServerThread::TcpServerThread():Thread(){
}

TcpServerThread::~TcpServerThread(){
}

void* TcpServerThread::run(void *arg){
	TcpServerThreadArg* threadArg = (TcpServerThreadArg*) arg;
	CommTestNode *commTestNode = threadArg->ctNode;
	int tSock,connSock;
	socklen_t tcpCliSize;
	char ipAddr[INET_ADDRSTRLEN];
	int retVal;
	sockaddr_in tcpSrvAddr,tcpCliAddr;

	if((tSock = socket(AF_INET,SOCK_STREAM,0))==-1){
		printf("tcp server socket() fail\n");
	}
	tcpSrvAddr.sin_family = AF_INET;
	tcpSrvAddr.sin_port = htons(TCP_SERVER_PORT);
	tcpSrvAddr.sin_addr.s_addr = INADDR_ANY;
	retVal = bind(tSock,(sockaddr *)&tcpSrvAddr,sizeof(tcpSrvAddr));
	if (retVal == -1){
		perror("TCP Server bind()");
		return NULL;
	}
	listen(tSock,TCP_LISTEN_LIMIT);
	tcpCliSize = sizeof(tcpCliAddr);
	while(1){
		connSock = accept(tSock, (struct sockaddr*)&tcpCliAddr, &tcpCliSize);
		if (connSock < 0) 
			perror("error on accept()");
        else{
        	::getIPAddress((struct sockaddr*)&tcpCliAddr,ipAddr);
        	/*Spawn new thread for every accepted connection*/
			TcpConnServerThreadArg* tcpConnArg = new TcpConnServerThreadArg;
			if(!tcpConnArg){
				printf("ERROR: Could not instantiate TcpServerThreadArg object");
				exit(1);
			}
			tcpConnArg->connectedSocket = connSock;
			tcpConnArg->ipAddress = ipAddr;
			Thread* tcpConnServerThread = new TcpConnServerThread();
			pthread_mutex_lock( &commTestNode->mThreadsMutex );
			commTestNode->mThreads.push_back(tcpConnServerThread);
			pthread_mutex_unlock( &commTestNode->mThreadsMutex );
			tcpConnServerThread->start(tcpConnArg);
         }
	    
	}
	return NULL;
}