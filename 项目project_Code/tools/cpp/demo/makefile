PUBINCL = -I/project/public

PUBCPP = /project/public/_public.cpp

CFLAGS = -g

all: demo03 demo04 demo05 demo08 demo06 demo07

demo03:demo03.cpp
	g++ $(CFLAGS) -o demo03 demo03.cpp $(PUBINCL) $(PUBCPP)

demo04:demo04.cpp
	g++ $(CFLAGS) -o demo04 demo04.cpp $(PUBINCL) $(PUBCPP)

demo05:demo05.cpp
	g++ $(CFLAGS) -o demo05 demo05.cpp $(PUBINCL) $(PUBCPP)

demo08:demo08.cpp
	g++ $(CFLAGS) -o demo08 demo08.cpp $(PUBINCL) $(PUBCPP)

demo07:demo07.cpp
	g++ $(CFLAGS) -o demo07 demo07.cpp $(PUBINCL) $(PUBCPP)

demo06:demo06.cpp
	g++ $(CFLAGS) -o demo06 demo06.cpp $(PUBINCL) $(PUBCPP)

clean:
	rm -f demo03 demo04 demo05 demo08 demo06 demo07