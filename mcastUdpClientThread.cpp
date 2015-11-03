#include "mcastUdpClientThread.h"

McastUdpClientThread::McastUdpClientThread():Thread(){
};

McastUdpClientThread::~McastUdpClientThread(){
}; 

void* McastUdpClientThread::run(void *arg){
	McastUdpClientThreadArg* threadArg = (McastUdpClientThreadArg*) arg;
	CommTestNode *commTestNode = threadArg->ctNode;
	int uSock;
	int bytesSent;
	int i=0;;
	in_addr sockOption;
	char pingMsg[BUF_SIZE];
	struct sockaddr_in dstMcastAddr;
	u_char loop = 0;
	u_char loopGet;
	socklen_t sizeGet;
	/*Send the multicast packet over the default interface chosen by kernel*/
	sockOption.s_addr = INADDR_ANY;
	if((uSock = socket(AF_INET,SOCK_DGRAM,0))==-1){
		printf("udp send socket() fail\n");
		goto ENDMCLNT;
	}
	setsockopt(uSock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
	getsockopt(uSock, IPPROTO_IP, IP_MULTICAST_LOOP, &loopGet, &sizeGet);
	printf("Client getsockopt() IP_MULTICAST_LOOP=%d\n",loopGet);
	if (setsockopt(uSock,IPPROTO_IP,IP_MULTICAST_IF,&sockOption,
		sizeof(struct in_addr))==-1){
		printf("setsockopt IP_MULTICAST_IF send side failed\n");
	}
	bzero((char *) &dstMcastAddr, sizeof(dstMcastAddr));
	dstMcastAddr.sin_family = AF_INET;
	dstMcastAddr.sin_port = htons(UDP_MULTICAST_PORT);
	dstMcastAddr.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);
	while(1){
		sprintf(pingMsg, "%s %d\n", UDP_PING_MESSAGE,i);
		if ((bytesSent = sendto(uSock,pingMsg,strlen(pingMsg),0,
			(struct sockaddr*)&dstMcastAddr,sizeof(dstMcastAddr))) == -1){
			perror("UDP MCAST sendto() failure");
			break;
		}
		printf("Sent %s\n",pingMsg);
		sleep(5*UDP_PING_INTERVAL);
		i++;
	}
	close(uSock);
ENDMCLNT:
	//commTestNode->StopThreads();
	if(threadArg!=NULL)
		delete threadArg;	
	return NULL;

}