#include "_public.h"

using namespace idc;    //io复用

int main(int argc,char*argv[])
{
    if (argc != 3)
    {
        cout << "Using ./demo07 192.168.240.131 5005\n";
        return -1;
    }

    ctcpclient tcpclient;
    if (tcpclient.connect(argv[1],atoi(argv[2])) == false)
    {
        printf("connect(%s,%s) failed\n",argv[1],argv[2]);return -1;
    }

    clogfile logfile;
    logfile.open("/tmp/demo07.log");

    string readbuffer,writebuffer;

    int ack =0;
    for(int ii=1;ii<=100000;ii++)
    {
        sformat(writebuffer,"这是第%d个超级女生",ii);
        logfile.write("%s\n",writebuffer.c_str());
        tcpclient.write(writebuffer);

        while(tcpclient.read(readbuffer,-1) == true)
        {
            logfile.write("%s\n",readbuffer.c_str());
            ack++;
        }
    }

    while(ack < 100000)
    {
        if (tcpclient.read(readbuffer) == true)
        {
            logfile.write("%s\n",readbuffer.c_str());
            ack++;
        }
    }

}
