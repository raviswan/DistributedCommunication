#include "mcastUdpServerThread.h"

McastUdpServerThread::McastUdpServerThread():Thread(){
};

McastUdpServerThread::~McastUdpServerThread(){
}; 

void* McastUdpServerThread::run(void *arg){
	McastUdpServerThreadArg* threadArg = (McastUdpServerThreadArg*) arg;
	CommTestNode *commTestNode = threadArg->ctNode;
	int uSock;
	int bytesSent;
	in_addr sockOption;
	char pingMsg[BUF_SIZE];
	struct sockaddr_in dstMcastAddr;
	int interval = 2;
	/*Send the multicast packet over the default interface chosen by kernel*/
	sockOption.s_addr = INADDR_ANY;
	if((uSock = socket(AF_INET,SOCK_DGRAM,0))==-1){
		printf("udp send socket() fail\n");
		return NULL;
	}
	if (setsockopt(uSock,IPPROTO_IP,IP_MULTICAST_IF,&sockOption,
		sizeof(struct in_addr))==-1){
		printf("setsockopt IP_MULTICAST_IF send side failed\n");
	}
	dstMcastAddr.sin_family = AF_INET;
	dstMcastAddr.sin_port = htons(UDP_MULTICAST_PORT);
	dstMcastAddr.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);
	int i=0;
	while(1){
		sprintf(pingMsg, "%s %d\n", UDP_PING_MESSAGE,i);
		if ((bytesSent = sendto(uSock,pingMsg,strlen(pingMsg),0,
			(struct sockaddr*)&dstMcastAddr,sizeof(dstMcastAddr))) == -1){
			perror("UDP MCAST sendto() failure");
			exit(1);
		}
		printf("Sent %s\n",pingMsg);
		sleep(UDP_PING_INTERVAL);
		i++;
	}
	close(uSock);
	return NULL;

}