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
	char ipAddrServer[INET_ADDRSTRLEN];
	int retVal,sockVal;
	sockaddr_in tcpSrvAddr,tcpCliAddr;

	if((tSock = socket(AF_INET,SOCK_STREAM,0))==-1){
		printf("tcp server socket() fail\n");
		goto END;
	}
	sockVal = 1;
  	setsockopt(tSock, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&sockVal , sizeof(int));
  	bzero((char *) &tcpSrvAddr, sizeof(tcpSrvAddr));
	tcpSrvAddr.sin_family = AF_INET;
	tcpSrvAddr.sin_port = htons(TCP_SERVER_PORT);
	tcpSrvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retVal = bind(tSock,(sockaddr *)&tcpSrvAddr,sizeof(tcpSrvAddr));
	if (retVal == -1){
		perror("TCP Server bind()");
		goto END;
	}
	//::getIPAddress((struct sockaddr*)&tcpSrvAddr,ipAddrServer);
	listen(tSock,TCP_LISTEN_LIMIT);
	tcpCliSize = sizeof(tcpCliAddr);
	while(1){
		connSock = accept(tSock, (struct sockaddr*)&tcpCliAddr, &tcpCliSize);
		if (connSock < 0) {
			perror("tcpServerThread accept()");
			break;
		}
        else{
        	::getIPAddress((struct sockaddr*)&tcpCliAddr,ipAddr);
        	printf("TCP client %s is connected to  address is %s\n",ipAddr,ipAddrServer);
        	/*Spawn new thread for every accepted connection.
        	Forking is another possiblity. To be considered later*/
			TcpConnServerThreadArg* tcpConnArg = new TcpConnServerThreadArg;
			if(!tcpConnArg){
				printf("ERROR: Could not instantiate TcpServerThreadArg object");
				break;
			}
			tcpConnArg->connectedSocket = connSock;
			tcpConnArg->ipAddress = ipAddr;
			tcpConnArg->ctNode = commTestNode;
			Thread* tcpConnServerThread = new TcpConnServerThread();
			pthread_mutex_lock( &commTestNode->mThreadsMutex );
			commTestNode->mThreads.push_back(tcpConnServerThread);
			pthread_mutex_unlock( &commTestNode->mThreadsMutex );
			tcpConnServerThread->start(tcpConnArg);
         }
	    
	}
END:
	close(tSock);
	if(threadArg!=NULL)
		delete threadArg;
	commTestNode->removeAllNeighbors();
	commTestNode->StopThreads();
	return NULL;
}