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
#define MULTICAST_ADDRESS "231.0.0.1"
#define UDP_MULTICAST_PORT 8234
#define BUF_SIZE 1024
#define UDP_PING_MESSAGE "udp ping"
#define UDP_PING_INTERVAL 1
#define RECV_BUF_LEN 1024
#define TCP_SERVER_PORT 12025
#define TCP_LISTEN_LIMIT 5
#define NUM_THREADS 4
#define RTT_COUNT 50
#define SOCKET_TIMEOUT 60
#define SLEEP_INTERVAL 5
#define TX_DATA_SIZE 128*1024*1024
#define REPORTING_INTERVAL 50

int getIPAddress(struct sockaddr *sa, char *ipAddr);


typedef struct{
	int tcpConnSocket;
	bool tcpConnFlag;
	double linkRTT;
	double linkBW;
} NodeInfo;

class CommTestNode
{
	friend class TcpServerThread;
	friend class TcpClientThread;
	friend class McastUdpServerThread;
	friend class McastUdpClientThread;

public:
	CommTestNode();
	void StartThreads();
	void StopThreads();
	void setNameAndIPAddress();
	void sendMcastPingOverUDP();
	void recvMcastPingOverUDP();
	void runTCPServer();
	void addNeighbor(const char* ipStr, int sock, bool connFlag);
	void updateNeighbor(string ipStr, int sock, bool connFlag);
	void removeNeighbor(string ipAddr);
	void removeAllNeighbors();
	void measureNeighborLatency(int sock);
	void measureNeighborBandwidth(string ipAddr);
	bool connectToTCPServer(string ipAddr);
	void startTcpClientForNeighbor(string ipAddr);
	void printNetworkStatistics(void);
	void printNeighbors(void);


	string getIPAddress() const;
	string getName() const;
	~CommTestNode();
private:
	string name;
	string ipAddress;
	std::map <string,NodeInfo > mNeighbors;
	std::vector<Thread*> mThreads;
	pthread_mutex_t mThreadsMutex;
	pthread_mutex_t mNeighborsMutex;


};

#endif
