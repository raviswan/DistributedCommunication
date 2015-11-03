# DistributedCommunication
communication test node for a distributed communication system

g++ -pthread -Wall  -o uber thread.cpp commTestNode.cpp tcpServerThread.cpp tcpConnServerThread.cpp tcpClientThread.cpp  mcastUdpServerThread.cpp mcastUdpClientThread.cpp 

The tcp client sends a message every 5 seconds.