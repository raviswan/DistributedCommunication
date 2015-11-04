#Makefile
EXE := cn
SRC := thread.cpp commTestNode.cpp tcpServerThread.cpp tcpConnServerThread.cpp tcpClientThread.cpp  mcastUdpServerThread.cpp mcastUdpClientThread.cpp
OBJS := thread.o commTestNode.o tcpServerThread.o tcpConnServerThread.o tcpClientThread.o  mcastUdpServerThread.o mcastUdpClientThread.o
#DEFS=-D_IPV6
CPP := g++
LD_OPTS=
LIBS := -lpthread
CPP_OPTS := -Wall
CPPFLAGS := $(CPP_OPTS) $(DEFS)

.PHONY: default clean

default: $(EXE)

$(EXE): $(OBJS)
	$(CPP) $(CPPFLAGS) -L. $(OBJS) $(LIBS) -o $@

$(OBJS): $(SRC)
	$(CPP) $(CPPFLAGS) -I.  -o thread.o -c thread.cpp
	$(CPP) $(CPPFLAGS) -I.  -o mcastUdpClientThread.o -c mcastUdpClientThread.cpp
	$(CPP) $(CPPFLAGS) -I.  -o mcastUdpServerThread.o -c mcastUdpServerThread.cpp
	$(CPP) $(CPPFLAGS) -I.  -o tcpClientThread.o -c tcpClientThread.cpp
	$(CPP) $(CPPFLAGS) -I.  -o tcpServerThread.o -c tcpServerThread.cpp
	$(CPP) $(CPPFLAGS) -I.  -o tcpConnServerThread.o -c tcpConnServerThread.cpp
	$(CPP) $(CPPFLAGS) -I.  -o commTestNode.o -c commTestNode.cpp

clean:
	rm -f $(EXE) *.o