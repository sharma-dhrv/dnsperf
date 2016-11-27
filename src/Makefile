#########################################
#		Source Directory Makefile		#
#########################################

CXX = g++
CXXFLAGS = -std=c++11 -c -Wall
OBJS = monitor.o dnsperf.o
DEPS = monitor.h
LIBS = -lmysqlpp -lpthread -lldns -lm
INCLUDES = -I/usr/include/mysql++ -I/usr/include/mysql -I/usr/include/ldns

all: $(OBJS)

monitor.o: monitor.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) $< -o $@ $(INCLUDES) $(LIBS)

dnsperf.o: dnsperf.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) $< -o $@ $(INCLUDES) $(LIBS)

.PHONY: clean

clean:
	$(RM) *.o
