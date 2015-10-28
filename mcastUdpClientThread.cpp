#include "mcastUdpClientThread.h"

McastUdpClientThread::McastUdpClientThread():Thread(){
};

McastUdpClientThread::~McastUdpClientThread(){
}; 

void* McastUdpClientThread::run(void *arg){
	McastUdpClientThreadArg* threadArg = (McastUdpClientThreadArg*) arg;
	CommTestNode *commTestNode = threadArg->ctNode;
	int rSock;
	int bytesRcvd;
	char rBuf[RECV_BUF_LEN];
	char ip[INET_ADDRSTRLEN];
	struct ip_mreq mcastAddReq;
	sockaddr_in recvAddr;
	sockaddr_in fromAddr;
	socklen_t fromAddrSize = sizeof(fromAddr);
	int retVal;

	if((rSock = socket(AF_INET,SOCK_DGRAM,0))==-1){
		printf("udp recv socket() fail\n");
	}

	memset(&mcastAddReq, 0, sizeof(struct ip_mreq));
	mcastAddReq.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDRESS);
	mcastAddReq.imr_interface.s_addr = INADDR_ANY;
	if (setsockopt(rSock,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mcastAddReq,
		sizeof(struct ip_mreq))==-1){
		printf("setsockopt IP_MULTICAST_ADD_MEMBERSHIP recv side failed\n");
	}

	recvAddr.sin_family = AF_INET;
	recvAddr.sin_port = htons(UDP_MULTICAST_PORT);
	recvAddr.sin_addr.s_addr = INADDR_ANY;
	retVal = bind(rSock,(sockaddr *)&recvAddr,sizeof(recvAddr));
	if(retVal == -1){
		perror("Error binding mcast UDP rcv socket");
		exit(1);
	}
	while(1){
		memset(rBuf,0,sizeof(rBuf));
		bytesRcvd = recvfrom(rSock,(char *)&rBuf,RECV_BUF_LEN,0,
			(struct sockaddr *)&fromAddr,&fromAddrSize);
		if(bytesRcvd > 0){
			if(inet_ntop(AF_INET, &fromAddr, ip, INET_ADDRSTRLEN)!=NULL){
				printf("Received from %s: %s",ip,rBuf);
				/*Add mcast neighbor's ip address to neigbor map. 
				TPC connection socket to neighbor will be obtained later*/
				commTestNode->addNeighbor(ip,0,false);
			}		
		}
		sleep(UDP_PING_INTERVAL);
	}
	close(rSock);
	return NULL;

}