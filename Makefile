#Makefile
SRC = thread.cpp commTestNode.cpp tcpServerThread.cpp tcpConnServerThread.cpp \
		tcpClientThread.cpp  mcastUdpServerThread.cpp mcastUdpClientThread.cpp

OBJS = $(SRC:%.c=%.o)
#-D_IPV6