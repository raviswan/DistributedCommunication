#include "mcastUdpServerThread.h"

McastUdpServerThread::McastUdpServerThread():Thread(){
	mFDMax = 0;
	FD_ZERO(&mFDSet);
}

McastUdpServerThread::~McastUdpServerThread(){
}

void* McastUdpServerThread::run(void *arg){
	McastUdpServerThreadArg* threadArg = (McastUdpServerThreadArg*) arg;
	CommTestNode *commTestNode = threadArg->ctNode;
	int rSock;
	int bytesRcvd;
	//char ip[INET_ADDRSTRLEN];
	struct ip_mreq mcastAddReq;
	sockaddr_in srvAddr;
	sockaddr_in fromAddr;
	socklen_t fromAddrSize = sizeof(fromAddr);
	int retVal;
	u_char loop = 0;
	u_char loopGet;
	socklen_t sizeGet;

	if((rSock = socket(AF_INET,SOCK_DGRAM,0))==-1){
		printf("udp recv socket() fail\n");
		goto ENDMSRV;
	}
	//sockVal = 1;
  	//setsockopt(rSock, SOL_SOCKET, SO_REUSEADDR,  (const void *)&sockVal , sizeof(int));
	setsockopt(rSock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
	memset(&mcastAddReq, 0, sizeof(struct ip_mreq));
	
	/*Enable socket to receive multicast packets*/
	getsockopt(rSock, IPPROTO_IP, IP_MULTICAST_LOOP, &loopGet, &sizeGet);
	mcastAddReq.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDRESS);
	mcastAddReq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(rSock,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mcastAddReq,
		sizeof(struct ip_mreq))==-1){
		printf("setsockopt IP_MULTICAST_ADD_MEMBERSHIP recv side failed\n");
	}
	bzero((char *) &srvAddr, sizeof(srvAddr));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(UDP_MULTICAST_PORT);
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(rSock,(sockaddr *)&srvAddr,sizeof(srvAddr))< 0){
		perror("Error binding mcast UDP rcv socket");
		exit(1);
	}
	while(1){
		FD_ZERO(&mFDSet);
		FD_SET(rSock,&mFDSet);
		if(rSock >= mFDMax)
			mFDMax = rSock + 1; 
        retVal = pselect(mFDMax,&mFDSet,NULL,NULL,NULL,NULL);
		if (retVal> 0){
			if(FD_ISSET(rSock,&mFDSet)){
				bzero(mBuf,BUF_SIZE);
				bytesRcvd = recvfrom(rSock,(char *)mBuf,BUF_SIZE,0,
					(struct sockaddr *)&fromAddr,&fromAddrSize);
				if(bytesRcvd > 0){
					if(inet_ntoa(fromAddr.sin_addr)!=NULL){
						printf("Received from ip=%s: port:%d\n",
							inet_ntoa(fromAddr.sin_addr),ntohs(fromAddr.sin_port));
						/*Add mcast neighbor's ip address to neigbor map. 
						TPC connection socket to neighbor will be obtained later*/
						commTestNode->addNeighbor(inet_ntoa(fromAddr.sin_addr),0,false);
					}		
				}
				else{
					perror("mcastUdpServer recvfrom() fail");
					break;
				}
			}
		}
		else{
			perror("mcastUdpServer pselect");
			break;
		}
	}
	close(rSock);
ENDMSRV:
	//commTestNode->removeAllNeighbors();
	if(threadArg!=NULL)
		delete threadArg;	
	commTestNode->removeAllNeighbors();
	return NULL;

}


