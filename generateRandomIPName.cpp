#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

using namespace std;
#define NETWORK_PREFIX "192.168.10.0"
#define NETMASK  "255.255.255.0"
#define NODENAME_LIMIT  128
#define MULTICAST_ADDRESS "236.0.0.1"


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
