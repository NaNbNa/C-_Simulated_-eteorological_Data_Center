PUBINCL = -I/project/public

PUBCPP = /project/public/_public.cpp

CFLAGS = -g
 
PUBLIB = /project/public/libftp.a /project/public/_ftp.cpp

#################################
ORAINCL = -I$(ORACLE_HOME)/rdbms/public -I/project/public/db/oracle

ORALIB = -L$(ORACLE_HOME)/lib -L.

ORALIBS = -lclntsh

ORACPP = /project/public/db/oracle/_ooci.cpp
#################################

all: procctl checkproc deletefiles gzipfiles ftpgetfiles ftpputfiles \
	tcpputfiles fileserver tcpgetfiles  dminingoracle xmltodb deletetable \
	migratetable syncref syncinc inetd rinetd rinetdin webserver


procctl: procctl.cpp
	g++ -g -o procctl procctl.cpp
	cp -f procctl ../bin/.

checkproc:checkproc.cpp
	g++ -g -o checkproc checkproc.cpp $(PUBINCL) $(PUBCPP)
	cp -f checkproc ../bin/.

deletefiles:deletefiles.cpp
	g++ -g -o deletefiles deletefiles.cpp $(PUBINCL) $(PUBCPP)
	cp -f deletefiles ../bin/.

gzipfiles:gzipfiles.cpp
	g++ -g -o gzipfiles gzipfiles.cpp $(PUBINCL) $(PUBCPP)
	cp -f gzipfiles ../bin/.

ftpgetfiles:ftpgetfiles.cpp
	g++ $(CFLAGS) -o ftpgetfiles ftpgetfiles.cpp $(PUBINCL)  $(PUBCPP) $(PUBLIB)
	cp -f ftpgetfiles ../bin/.
	
ftpputfiles:ftpputfiles.cpp
	g++ $(CFLAGS) -o ftpputfiles ftpputfiles.cpp $(PUBINCL)  $(PUBCPP) $(PUBLIB)
	cp -f ftpputfiles ../bin/.

tcpputfiles:tcpputfiles.cpp
	g++ $(CFLAGS) -o tcpputfiles tcpputfiles.cpp $(PUBINCL)  $(PUBCPP) 
	cp -f tcpputfiles ../bin/.

tcpgetfiles:tcpgetfiles.cpp
	g++ $(CFLAGS) -o tcpgetfiles tcpgetfiles.cpp $(PUBINCL)  $(PUBCPP) 
	cp -f tcpgetfiles ../bin/.

fileserver:fileserver.cpp
	g++ $(CFLAGS) -o fileserver fileserver.cpp $(PUBINCL)  $(PUBCPP) 
	cp -f fileserver ../bin/.

dminingoracle:dminingoracle.cpp
	g++ $(CFLAGS) -o dminingoracle dminingoracle.cpp $(PUBINCL)  $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP) 
	cp -f dminingoracle ../bin/.

xmltodb:xmltodb.cpp tools.cpp
	g++ $(CFLAGS) -o xmltodb xmltodb.cpp  $(PUBINCL)  $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP)  tools.cpp
	cp -f xmltodb ../bin/.

deletetable:deletetable.cpp
	g++ $(CFLAGS) -o deletetable deletetable.cpp $(PUBINCL)  $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP) 
	cp -f deletetable ../bin/.
migratetable:migratetable.cpp tools.cpp
	g++ $(CFLAGS) -o migratetable migratetable.cpp $(PUBINCL)  $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP) tools.cpp
	cp -f migratetable ../bin/.

syncref:syncref.cpp
	g++ $(CFLAGS) -o syncref syncref.cpp $(PUBINCL)  $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP) tools.cpp
	cp -f syncref ../bin/.

syncinc:syncinc.cpp
	g++ $(CFLAGS) -o syncinc syncinc.cpp $(PUBINCL)  $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP) tools.cpp
	cp -f syncinc ../bin/.

inetd:inetd.cpp
	g++ $(CFLAGS) -o inetd inetd.cpp $(PUBINCL)  $(PUBCPP)
	cp -f inetd ../bin/.

rinetd:rinetd.cpp
	g++ $(CFLAGS) -o rinetd rinetd.cpp $(PUBINCL)  $(PUBCPP)
	cp -f rinetd ../bin/.

rinetdin:rinetdin.cpp
	g++ $(CFLAGS) -o rinetdin rinetdin.cpp $(PUBINCL)  $(PUBCPP)
	cp -f rinetdin ../bin/.

webserver:webserver.cpp
	g++ $(CFLAGS) -o webserver webserver.cpp $(PUBINCL)  $(PUBCPP)  $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP) -lpthread
	cp -f webserver ../bin/.


clean:
	rm -f procctl checkproc deletefiles ftpgetfiles ftpputfiles 
	rm -f tcpputfiles fileserver dminingoracle xmltodb deletetable 
	rm -f migratetable syncref syncinc inetd rinetd rinetdin webserver
