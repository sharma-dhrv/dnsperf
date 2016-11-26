all: monitor.o dnsperf

monitor.o: monitor.h monitor.cpp
	g++ -std=c++11 -c monitor.cpp -o monitor.o -I/usr/include/mysql++ -I/usr/include/mysql -I/usr/include/ldns -lmysqlpp -lpthread -lldns

dnsperf: dnsperf.cpp monitor.o
	g++ -std=c++11 dnsperf.cpp monitor.o -o dnsperf -I/usr/include/mysql++ -I/usr/include/mysql -I/usr/include/ldns -lmysqlpp -lpthread -lldns

clean:
	rm *.o dnsperf
