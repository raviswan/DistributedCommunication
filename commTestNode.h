#ifndef COMM_TEST_NODE_H
#define COMM_TEST_NODE_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <string.h>
#include "thread.h"
#include <time.h>

using namespace std;
#define NETWORK_PREFIX "192.168.10.0"
#define NETMASK  "255.255.255.0"
#define NODENAME_LIMIT  128
#define MULTICAST_ADDRESS "236.0.0.1"
#define UDP_MULTICAST_PORT 8234
#define BUF_SIZE 256
#define UDP_PING_MESSAGE "udp ping"
#define UDP_PING_INTERVAL 1
#define RECV_BUF_LEN 1024
#define TCP_SERVER_PORT 12023
#define TCP_LISTEN_LIMIT 5
#define NUM_THREADS 4
#define RTT_COUNT 50
#define SOCKET_TIMEOUT 2

int getIPAddress(struct sockaddr *sa, char *ipAddr);
typedef struct{
	int tcpConnSocket;
	bool tcpConnFlag;
	long linkRTT;
	long linkBW;
} NodeInfo;

class CommTestNode
{
	friend class TcpServerThread;
	friend class TcpClientThread;
	friend class McastUdpServerThread;
	friend class McastUdpClientThread;

public:
	CommTestNode();
	void Init();
	void setNameAndIPAddress();
	void sendMcastPingOverUDP();
	void recvMcastPingOverUDP();
	void runTCPServer();
	void addNeighbor(const char* ipStr, int sock, bool connFlag);
	void updateNeighbor(string ipStr, int sock, bool connFlag);
	void removeNeighbor(const char* ipStr);
	void getNeighborStats();
	void measureNeighborLatency(int sock);
	void measureNeighborBandwidth(string ipAddr);
	bool connectToTCPServer(string ipAddr);
	void startTcpClientForNeighbor(string ipAddr);
	void setNeighborRTT(int long val);

	string getIPAddress() const;
	string getName() const;
	~CommTestNode();
private:
	string name;
	string ipAddress;
	std::map <string,NodeInfo > mNeighbors;
	std::vector<Thread*> mThreads;
	pthread_mutex_t mThreadLock;
	pthread_mutex_t mNeighborMapLock;


};

#endif
