#include "commTestNode.h"
#include "tcpServerThread.h"
#include "tcpClientThread.h"
#include "mcastUdpServerThread.h"
#include "mcastUdpClientThread.h"


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
    	if(inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
    		ip, INET_ADDRSTRLEN) != NULL){
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

void networkStatsHandler(void* arg){
	CommTestNode *ctNode = (CommTestNode*) arg;
	ctNode->printNetworkStatistics();
}

CommTestNode::CommTestNode(){
	
}

CommTestNode::~CommTestNode(){

}

void CommTestNode::StartThreads(){
	if(pthread_mutex_init( &mNeighborsMutex, NULL)!= 0)
		exit(1);
	if(pthread_mutex_init( &mThreadsMutex, NULL)!= 0)
		exit(1);

	TcpServerThreadArg* tcsArg = new TcpServerThreadArg;
	if(!tcsArg){
		printf("ERROR: Could not instantiate TcpServerThreadArg object");
		exit(1);
	}
	tcsArg->ctNode = this;
	Thread* tcpSrvThread = new TcpServerThread();
	pthread_mutex_lock( &mThreadsMutex );
	mThreads.push_back(tcpSrvThread);
	pthread_mutex_unlock( &mThreadsMutex );
	tcpSrvThread->start(tcsArg);

	McastUdpServerThreadArg* msArg =  new McastUdpServerThreadArg;
	if(!msArg){
		printf("ERROR: Could not instantiate McastUdpServerThreadArg object");
		exit(1);
	}
	msArg->ctNode = this;
	Thread* mcastUdpSrvThread = new McastUdpServerThread();
	pthread_mutex_lock( &mThreadsMutex );
	mThreads.push_back(mcastUdpSrvThread);
	pthread_mutex_unlock( &mThreadsMutex );
	mcastUdpSrvThread->start(msArg);

	McastUdpClientThreadArg* mcArg = new McastUdpClientThreadArg;
	if(!mcArg){
		printf("ERROR: Could not instantiate McastUdpClientThreadArg object");
		exit(1);
	}
	mcArg->ctNode = this;
	Thread* mcastUdpCliThread = new McastUdpClientThread();
	pthread_mutex_lock( &mThreadsMutex );
	mThreads.push_back(mcastUdpCliThread);
	pthread_mutex_unlock( &mThreadsMutex );
	mcastUdpCliThread->start(mcArg);


}

void CommTestNode::StopThreads(){
	Thread* thread;
	pthread_mutex_lock( &mThreadsMutex );
	while(!mThreads.empty()){
		thread = mThreads.back();
		mThreads.pop_back();
		delete thread;
	}
	pthread_mutex_unlock( &mThreadsMutex );
	
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
	pthread_mutex_lock( &mNeighborsMutex );
	std::map<string,NodeInfo>::iterator it = mNeighbors.find(ipAddr);
	if(it!=mNeighbors.end()){
		/*neighbor already added. release mutex and exit the function*/
		pthread_mutex_unlock( &mNeighborsMutex );
	}
	else{
		/*new neighbor, so add to neighbors map and start a tcp connection*/
		node.tcpConnSocket = sock;
		node.tcpConnFlag = connFlag;
		printf("adding %s to neighbor map\n",ipStr);
		mNeighbors.insert ( std::pair<string,NodeInfo>(ipAddr,node));
		pthread_mutex_unlock( &mNeighborsMutex );
		printNeighbors();
		/*Initiate tcp connection to server for this newly added neighbor*/
		connectToTCPServer(ipAddr);
	}

}

void CommTestNode::updateNeighbor(string ipAddr, int sock, bool connFlag){
	std::map<string,NodeInfo>::iterator it;
	pthread_mutex_lock( &mNeighborsMutex );
	if( (it = mNeighbors.find(ipAddr)) != mNeighbors.end()){
		mNeighbors[ipAddr].tcpConnSocket = sock;
		mNeighbors[ipAddr].tcpConnFlag = connFlag;
	}
	pthread_mutex_unlock( &mNeighborsMutex );
}

void CommTestNode::removeNeighbor(string ipAddr){
	std::map<string,NodeInfo>::iterator it;
	//string ipAddr(ipStr);
	pthread_mutex_lock( &mNeighborsMutex );
	it =  mNeighbors.find(ipAddr);
	if (it != mNeighbors.end()){
		printf("erasing neighbor with IP: %s\n",it->first.c_str());
		mNeighbors.erase(it);
	}
	pthread_mutex_unlock( &mNeighborsMutex );
		
}

void CommTestNode::removeAllNeighbors(){
	pthread_mutex_lock(&mNeighborsMutex);
	for(std::map<string,NodeInfo>::iterator it = mNeighbors.begin();
		it!=mNeighbors.end();++it){
			mNeighbors.erase(it);
	}
	pthread_mutex_unlock(&mNeighborsMutex);
	
	printf("erased all neighbors\n");
		
}

bool CommTestNode::connectToTCPServer(string ipAddr){
	int cliSock;
	int retVal,sockVal;
	sockaddr_in dstSrvAddr;
	if((cliSock = socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("tcp server socket() fail");
	}
	sockVal = 1;
  	setsockopt(cliSock, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&sockVal , sizeof(int));
  	bzero((char *) &dstSrvAddr, sizeof(dstSrvAddr));
	dstSrvAddr.sin_family = AF_INET;
	dstSrvAddr.sin_port = htons(TCP_SERVER_PORT);
	dstSrvAddr.sin_addr.s_addr = inet_addr(ipAddr.c_str());
	retVal = connect(cliSock,(struct sockaddr*)&dstSrvAddr,sizeof(dstSrvAddr));
	if(retVal == -1){
		perror("Client socket conect() failed with");
		return false;
	}
	printf("Tcp Client connect() succceeded with sock=%d\n",cliSock);
	/*Update neighbor map with this newneighbor*/
	this->updateNeighbor(ipAddr,cliSock,true);
	startTcpClientForNeighbor(ipAddr);
	return true;

}
void CommTestNode::startTcpClientForNeighbor(string ipAddr){
	printf("Starting tcp client to talk to %s\n",ipAddr.c_str());
	TcpClientThreadArg* threadArg = new TcpClientThreadArg;
	if(!threadArg){
		printf("ERROR: Could not instantiate TcpClientThreadArg object");
		return;
	}
	threadArg->ctNode = this;
	threadArg->ipStr =  ipAddr;
	Thread* tcpCliThread = new TcpClientThread();
	pthread_mutex_lock( &mThreadsMutex );
	mThreads.push_back(tcpCliThread);
	pthread_mutex_unlock( &mThreadsMutex );
	tcpCliThread->start(threadArg);

}
void CommTestNode::printNetworkStatistics(void){
	pthread_mutex_lock(&mNeighborsMutex);
	for(std::map<string,NodeInfo>::iterator it = mNeighbors.begin();
		it!=mNeighbors.end();++it){
		printf("Neighbor %s: Bandwith=%10g kbps, RTT=%5g msec\n",it->first.c_str(),mNeighbors[it->first].linkBW,
			mNeighbors[it->first].linkRTT);
	}
	pthread_mutex_unlock(&mNeighborsMutex);
}

void CommTestNode::printNeighbors(void){
	printf("Neighbors are:\n\n");
	pthread_mutex_lock(&mNeighborsMutex);
	for(std::map<string,NodeInfo>::iterator it = mNeighbors.begin();
		it!=mNeighbors.end();++it)
		printf("%s\n",it->first.c_str());
	pthread_mutex_unlock(&mNeighborsMutex);
}



int main(){
	/*initialize random generator seed*/
	srand(time(NULL));
	CommTestNode *cn = new CommTestNode();
	cn->StartThreads();
	TcpClientThread::RegisterNetworkStatsHandler(networkStatsHandler);
	//cn->StopThreads();
	//setenv("myIP",(const char*) MY_IP_ADDR,1);
	//system("echo \"ping $myIP\"");
	//system("ping $myIP");
	//TestNodeIPAndName();
	getc(stdin);
	cn->StopThreads();
	delete cn;
	return 0;
	
}

