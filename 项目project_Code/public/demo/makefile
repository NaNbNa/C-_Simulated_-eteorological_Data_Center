PUBINCL = -I/root/train/public

PUBCPP = /root/train/public/_public.cpp /root/train/public/_ftp.cpp  /root/train/public/libftp.a

# PROGRAMS = demo1 demo2 demo3 demo4 demo5 demo6 demo7 demo8 demo9 demo10 \
# 	demo11 demo12 demo13 demo14 demo15 demo16 demo17 demo18 demo19 demo20 \
# 	demo21 demo22 demo23 demo24 demo25 demo26 demo27 demo28 demo29 demo30 \
# 	demo31 demo32 demo33

PROGRAMS = demo23 demo24 demo25


THREAD = demo100 demo101

PROGRAMS_obj = %:%.cpp  $(PUBCPP)  
THREAD_obj = %:%.cpp  $(PUBCPP) -lpthread
COMPILE_FLAG = -g $(PUBINCL)

all:libftp $(PROGRAMS) $(THREAD) 

libftp:demo23 demo24 demo25
	cd .. && gcc -c -o libftp.a ftplib.c  
	cd .. && gcc -fPIC -shared -o libftp.so ftplib.c 

$(PROGRAMS):$(PROGRAMS_obj)
	g++ $(COMPILE_FLAG) -o $@ $^ 

$(THREAD):$(THREAD_obj)
	g++ $(COMPILE_FLAG) -o $@ $^

clean:
	rm -f $(PROGRAMS) $(THREAD) 
