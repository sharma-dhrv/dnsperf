#################################################
#       Project's Root Directory Makefile       #
#################################################

SRC_DIR = src
EXEC = dnsperf
OBJS = $(SRC_DIR)/monitor.o $(SRC_DIR)/dnsperf.o
CXX = g++
CXXFLAGS = -std=c++11 -Wall
DEPS = monitor.h
LIBS = -lmysqlpp -lpthread -lldns -lm
INCLUDES = -I/usr/include/mysql++ -I/usr/include/mysql -I/usr/include/ldns

all: build $(EXEC)

build:
	+$(MAKE) -C $(SRC_DIR)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(INCLUDES) $(LIBS)

.PHONY: clean

clean:
	+$(MAKE) -C $(SRC_DIR) clean
	$(RM) $(EXEC)
