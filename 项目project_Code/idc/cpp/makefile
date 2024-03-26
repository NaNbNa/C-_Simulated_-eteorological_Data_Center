PUBINCL = -I/project/public

PUBCPP = /project/public/_public.cpp

#################################
ORAINCL = -I$(ORACLE_HOME)/rdbms/public -I/project/public/db/oracle

ORACPP = /project/public/db/oracle/_ooci.cpp

ORALIB = -L$(ORACLE_HOME)/lib -L.

ORALIBS = -lclntsh
#################################

CFLAGS = -g

all:crtsurfdata obtcodetodb obtmindtodb obtmindtodb2 

crtsurfdata:crtsurfdata.cpp
	g++ $(CFLAGS) -o crtsurfdata $< $(PUBINCL) $(PUBCPP)
	cp -f crtsurfdata ../bin/.

obtcodetodb:obtcodetodb.cpp
	g++ $(CFLAGS) -o obtcodetodb $< $(PUBINCL) $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP)
	cp -f obtcodetodb ../bin/.

obtmindtodb:obtmindtodb.cpp idcapp.cpp
	g++ $(CFLAGS) -o obtmindtodb $< idcapp.cpp $(PUBINCL) $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP)
	cp -f obtmindtodb ../bin/.

obtmindtodb2:obtmindtodb2.cpp 
	g++ $(CFLAGS) -o obtmindtodb2 $< $(PUBINCL) $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP) 
	cp -f obtmindtodb2 ../bin/.


clean:
	rm -f crtsurfdata obtcodetodb obtmindtodb obtmindtodb2