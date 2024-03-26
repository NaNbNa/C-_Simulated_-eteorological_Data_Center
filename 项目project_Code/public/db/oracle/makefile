
ORAINCL = -I$(ORACLE_HOME)/rdbms/public

ORALIB = -L$(ORACLE_HOME)/lib -L.

ORALIBS = -lclntsh

CFLAGS = -O2 -Wall
CFLAGS = -g -Wall -Wno-write-strings

# all:createtable inserttable updatetable deletetable selecttable filetoclob clobtofile filetoblob blobtofile 
#####################
all: demo01 demo02 demo03 demo04 

demo01:demo01.cpp
	g++ $(CFLAGS) -o demo01 demo01.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

demo02:demo02.cpp
	g++ $(CFLAGS) -o demo02 demo02.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

demo03:demo03.cpp
	g++ $(CFLAGS) -o demo03 demo03.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

demo04:demo04.cpp
	g++ $(CFLAGS) -o demo04 demo04.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

#####################
createtable: createtable.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o createtable createtable.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

inserttable:inserttable.cpp 
	g++ $(CFLAGS) -o inserttable inserttable.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

updatetable:updatetable.cpp 
	g++ $(CFLAGS) -o updatetable updatetable.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

deletetable:deletetable.cpp 
	g++ $(CFLAGS) -o deletetable deletetable.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

selecttable:selecttable.cpp 
	g++ $(CFLAGS) -o selecttable selecttable.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

filetoclob:filetoclob.cpp 
	g++ $(CFLAGS) -o filetoclob filetoclob.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

clobtofile:clobtofile.cpp 
	g++ $(CFLAGS) -o clobtofile clobtofile.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

filetoblob:filetoblob.cpp 
	g++ $(CFLAGS) -o filetoblob filetoblob.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

blobtofile:blobtofile.cpp 
	g++ $(CFLAGS) -o blobtofile blobtofile.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp

# clean:
# 	rm -rf createtable inserttable updatetable deletetable selecttable filetoclob clobtofile filetoblob blobtofile 
clean:
	demo01 demo02 demo03 demo04