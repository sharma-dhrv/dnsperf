all: monitor.o dnsperf

monitor.o: monitor.h monitor.cpp
	g++ -std=c++11 -c monitor.cpp -o monitor.o -I/usr/include/mysql++ -I/usr/include/mysql -lmysqlpp

dnsperf: dnsperf.cpp monitor.o
	g++ -std=c++11 dnsperf.cpp monitor.o -o dnsperf -I/usr/include/mysql++ -I/usr/include/mysql -lmysqlpp -lpthread

clean:
	rm *.o dnsperf
