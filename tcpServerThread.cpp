#include "tcpServerThread.h"

TcpServerThread::TcpServerThread():Thread(){
};

TcpServerThread::~TcpServerThread(){
}; 

void* TcpServerThread::run(void *arg){
	TcpServerThreadArg* threadArg = (TcpServerThreadArg*) arg;
	CommTestNode *commTestNode = threadArg->ctNode;
	int tSock,connSock;
	socklen_t tcpCliSize;
	char connBuf[RECV_BUF_LEN];
	char ipAddr[INET_ADDRSTRLEN];
	int retVal;
	int portNum;
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
			//commTestNode->addNeighbor(ipAddr,connSock,true);
         }
         sleep(1);  
	    
	}

}