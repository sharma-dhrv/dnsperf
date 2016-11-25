all: db dns

db:	db.cpp
	g++ db.cpp -o db -I/usr/include/mysql++ -I/usr/include/mysql -lmysqlpp

dns: dns.cpp
	g++ dns.cpp -o dns -I/usr/include/ldns -lldns

clean:
	rm *.o db dns
