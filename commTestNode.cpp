#include "commTestNode.h"
#include "tcpServerThread.h"
#include "tcpClientThread.h"
#include "mcastUdpServerThread.h"
#include "mcastUdpClientThread.h"

/*
--------TO DO-------------
Add mutex for mThreads,mNeighbors
 */

void * Malloc(int size){
	void *p = malloc(size);
	if (p==NULL){
		fprintf(stderr,"malloc failed\n");
		return NULL;
	}
	else 
		return p;
}

//Convert a struct sockaddr address to IPv4 string
int getIPAddress(struct sockaddr *sa, char *ipAddr){
	char ip[INET_ADDRSTRLEN];
    if(sa->sa_family == AF_INET) {
    	if(inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),ip, INET_ADDRSTRLEN) != NULL){
    		/*copy the address into ipAddr*/
			memcpy(ipAddr,ip,INET_ADDRSTRLEN);
			return 1;
		}
    }
  	return -1;
}

int generateRandomNameAndIP(const char* prefix, const char* netmask, 
	char* ipAddr, char *nodeName) {
	unsigned int maxHostSupported,host;
	in_addr address;
	in_addr_t netmaskBinary, prefixBinary;
	sockaddr_in sa;
	char ip[INET_ADDRSTRLEN];
	char name[NODENAME_LIMIT], temp[NODENAME_LIMIT];
	memset(name,0,NODENAME_LIMIT);
	/*convert netmask to binary format in network byte order */
	netmaskBinary = inet_addr(netmask);
	/*Convert network byte to host byte order*/
	netmaskBinary = ntohl(netmaskBinary);
	/*NOTing netmask gives you the number of nodes supported on this subnet*/
	maxHostSupported  = ~netmaskBinary;
	/*generate host ID . 255 is broadcast. hence 1-254 are randomly generated*/
	host = (rand() % (maxHostSupported-1))+1;
	
	/*Node naming convention: "TestNode-HostNo" */
	sprintf(temp,"%d",host);
	strcat(name,"TestNode-");
	strcat(name,temp);
	strcpy(nodeName,name);


	/* Get the final prefix after netmasking */
	prefixBinary = inet_addr(prefix);
	prefixBinary = prefixBinary & htonl(netmaskBinary);
	/*Combine prefix and host to get the final ip address*/
	address.s_addr = prefixBinary | htonl(host);
	if(inet_ntop(AF_INET, &address, ip, INET_ADDRSTRLEN)!=NULL){
		/*copy the address into ipAddr*/
		memcpy(ipAddr,ip,INET_ADDRSTRLEN);
		return 1;
	}
	return -1;

}

CommTestNode::CommTestNode(){
	Init();
}

CommTestNode::~CommTestNode(){
}

void CommTestNode::Init(){
	//TcpServerThreadArg* threadArg = (TcpServerThreadArg*) Malloc(sizeof(TcpServerThreadArg));
	TcpServerThreadArg* threadArg = new TcpServerThreadArg;
	if(!threadArg){
		printf("ERROR: Could not instantiate TcpServerThreadArg object");
		exit(1);
	}
	threadArg->ctNode = this;
	Thread* tcpSrvThread = new TcpServerThread();
	mThreads.push_back(tcpSrvThread);
	tcpSrvThread->run(threadArg);

	//McastUdpServerThreadArg* msArg = (McastUdpServerThreadArg*) Malloc(sizeof(McastUdpServerThreadArg));
	McastUdpServerThreadArg* msArg =  new McastUdpServerThreadArg;
	if(!msArg){
		printf("ERROR: Could not instantiate McastUdpServerThreadArg object");
		exit(1);
	}
	msArg->ctNode = this;
	Thread* mcastUdpSrvThread = new McastUdpServerThread();
	mThreads.push_back(mcastUdpSrvThread);
	mcastUdpSrvThread->run(msArg);

	//McastUdpClientThreadArg* mcArg = (McastUdpClientThreadArg*) Malloc(sizeof(McastUdpClientThreadArg));
	McastUdpClientThreadArg* mcArg = new McastUdpClientThreadArg;
	if(!mcArg){
		printf("ERROR: Could not instantiate McastUdpClientThreadArg object");
		exit(1);
	}
	mcArg->ctNode = this;
	Thread* mcastUdpCliThread = new McastUdpClientThread();
	mThreads.push_back(mcastUdpCliThread);
	mcastUdpCliThread->run(mcArg);


}
void CommTestNode::setNameAndIPAddress(){
	char ipStr[INET_ADDRSTRLEN];
	char nameStr[NODENAME_LIMIT];
	if (generateRandomNameAndIP(NETWORK_PREFIX, NETMASK, ipStr, nameStr)==1){
		string ipaddr(ipStr);
		string nodeName(nameStr);
		this->ipAddress = ipaddr;
		this->name = nodeName;
	}
}

string CommTestNode::getName() const{
	return this->name;
}

void CommTestNode::addNeighbor(const char* ipStr, int sock, 
	bool connFlag){
	string ipAddr(ipStr);
	NodeInfo node;
	if(mNeighbors.find(ipAddr) == mNeighbors.end()){
		node.tcpConnSocket = sock;
		node.tcpConnFlag = connFlag;
		printf("adding %s to neighbor map\n",ipStr);
		mNeighbors.insert ( std::pair<string,NodeInfo>(ipAddr,node));
		/*Initiate tcp connection to server for this newly added neighbor*/
		connectToTCPServer(ipAddr);
	}

}

void CommTestNode::updateNeighbor(string ipStr, int sock, bool connFlag){
	/*Mutex*/
	mNeighbors[ipStr].tcpConnSocket = sock;
	mNeighbors[ipStr].tcpConnFlag = connFlag;
}

void CommTestNode::removeNeighbor(const char* ipStr){
	std::map<string,NodeInfo>::iterator it;
	string ipAddr(ipStr);
	it =  mNeighbors.find(ipAddr);
	if (it != mNeighbors.end()){
		cout<<"erasing mNeighbors"<<it->first<<endl;
		mNeighbors.erase(it);
	}
		
}

void CommTestNode::setNeighborRTT(int long val){

}
bool CommTestNode::connectToTCPServer(string ipAddr){
	int cliSock;
	int retVal;
	sockaddr_in dstSrvAddr;
	if((cliSock = socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("tcp server socket() fail");
	}
	dstSrvAddr.sin_family = AF_INET;
	dstSrvAddr.sin_port = htons(TCP_SERVER_PORT);
	dstSrvAddr.sin_addr.s_addr = inet_addr(ipAddr.c_str());
	retVal = connect(cliSock,(struct sockaddr*)&dstSrvAddr,sizeof(dstSrvAddr));
	if(retVal == -1){
		perror("Client socket conect() failed with");
		return false;
	}
	/*Update neighbor map with this newneighbor*/
	this->updateNeighbor(ipAddr,cliSock,true);
	startTcpClientForNeighbor(ipAddr);
	return true;

}
void CommTestNode::startTcpClientForNeighbor(string ipAddr){
	TcpClientThreadArg* threadArg = new TcpClientThreadArg;
	if(!threadArg){
		printf("ERROR: Could not instantiate TcpClientThreadArg object");
		exit(1);
	}
	threadArg->ctNode = this;
	threadArg->ipStr =  ipAddr;
	Thread* tcpSrvThread = new TcpClientThread();
	mThreads.push_back(tcpSrvThread);
	tcpSrvThread->run(threadArg);

}

/*

void CommTestNode::getNeighborStats(){
	std::map<string,NodeInfo>::iterator it = mNeighbors.begin();
	while( it != mNeighbors.end()){
		//Already have a connection to the neighbor
		if(it->second.tcpConnFlag){
			measureNeighborLatency(it->second.tcpConnSocket);
			measureNeighborBandwidth(it->first);
		}
		else{
			//The new neighbor was just added. So connect to server first before measuring the latency
			if(connectToTCPServer(it->first)){
				//it->second.rtt = measureNeighborLatency(it->second.tcpConnSocket);
				measureNeighborBandwidth(it->first);
			}
		}
		it++;
	}
}





void CommTestNode::runTCPServer(){
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
		return;
	}
	listen(tSock,TCP_LISTEN_LIMIT);
	tcpCliSize = sizeof(tcpCliAddr);
	while(1){
		connSock = accept(tSock, (struct sockaddr*)&tcpCliAddr, &tcpCliSize);
		if (connSock < 0) 
			perror("error on accept()");
        else{
        	::getIPAddress((struct sockaddr*)&tcpCliAddr,ipAddr);
			this->addNeighbor(ipAddr,connSock,true);
         }
         sleep(1);  
	    
	}
}

void CommTestNode::sendMcastPingOverUDP(){
	int uSock;
	int bytesSent;
	in_addr sockOption;
	char pingMsg[BUF_SIZE];
	struct sockaddr_in dstMcastAddr;
	int interval = 2;
	sockOption.s_addr = INADDR_ANY;
	if((uSock = socket(AF_INET,SOCK_DGRAM,0))==-1){
		printf("udp send socket() fail\n");
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
	return;

}

void CommTestNode::recvMcastPingOverUDP(){
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
				this->addNeighbor(ip,0,false);
			}		
		}
		sleep(UDP_PING_INTERVAL);
	}
	close(rSock);
	return;

}
*/

int main(){
	/*initialize random generator seed*/
	srand(time(NULL));
	//setenv("myIP",(const char*) MY_IP_ADDR,1);
	//system("echo \"ping $myIP\"");
	//system("ping $myIP");
	//TestNodeIPAndName();
	return 0;
	
}

