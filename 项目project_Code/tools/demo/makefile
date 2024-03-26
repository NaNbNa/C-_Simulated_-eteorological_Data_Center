PUBINCL = -I/project/public

PUBCPP = /project/public/_public.cpp

CFLAGS = -g

#################################
ORAINCL = -I$(ORACLE_HOME)/rdbms/public -I/project/public/db/oracle

ORALIB = -L$(ORACLE_HOME)/lib -L.

ORALIBS = -lclntsh

ORACPP = /project/public/db/oracle/_ooci.cpp
#################################

all: server client server1 server2 client2

server:server.cpp
	g++ $(CFLAGS) -o server server.cpp $(PUBINCL) $(PUBCPP)

client:client.cpp
	g++ $(CFLAGS) -o client client.cpp $(PUBINCL) $(PUBCPP)

server1:server1.cpp
	g++ $(CFLAGS) -o server1 server1.cpp $(PUBINCL) $(PUBCPP)

server2:server2.cpp
	g++ $(CFLAGS) -o server2 server2.cpp $(PUBINCL) $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP) 

client2:client2.cpp
	g++ $(CFLAGS) -o client2 client2.cpp $(PUBINCL) $(PUBCPP)

clean:
	rm -f server client server2 client2