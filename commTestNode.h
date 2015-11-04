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

/*Structure to hold each neighbor's info*/
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
	/*Starts Mcast and Tcp server threads*/
	void StartThreads();
	/*Stops ALL the threads*/
	void StopThreads();
	/*To set name and IP. Not presently used*/
	void setNameAndIPAddress();
	/*Store a newly discovered neighbor*/
	void addNeighbor(const char* ipStr, int sock, bool connFlag);
	/*Update neihhbor with connected socket info*/
	void updateNeighbor(string ipStr, int sock, bool connFlag);
	/*delete neighbor from the neighbor list*/
	void removeNeighbor(string ipAddr);
	/*wipe out the neighbor list when threads are stopped*/
	void removeAllNeighbors();
	/*update the round trip time of a given neighbor*/
	void measureNeighborLatency(int sock);
	/*update the bandwidth to a given neighbor*/
	void measureNeighborBandwidth(string ipAddr);
	/*When a new neighbor is added, connect to its TCP server*/
	bool connectToTCPServer(string ipAddr);
	/*Start a new TCP client for each connected neighbor*/
	void startTcpClientForNeighbor(string ipAddr);
	/*print BW and RTT stats for each neighbor*/
	void printNetworkStatistics(void);
	/*To see who your neighbors are*/
	void printNeighbors(void);
	/*Get the ip address given socket structure*/
	string getIPAddress() const;
	string getName() const;
	~CommTestNode();
private:
	string name;
	string ipAddress;
	/*STL map to store neighbors*/
	std::map <string,NodeInfo > mNeighbors;
	/*storage of all the created threads */
	std::vector<Thread*> mThreads;
	/*mutex for creating/deleting threads*/
	pthread_mutex_t mThreadsMutex;
	/*mutex for accessing neighbor map*/
	pthread_mutex_t mNeighborsMutex;


};

#endif
