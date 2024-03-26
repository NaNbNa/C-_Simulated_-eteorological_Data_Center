#include "_public.h"
using namespace idc;

int main(int argc,char* argv[]) //异步通讯
{
    if (argc!=2)
    {
        cout << "Using ./demo08 5005\n" <<endl;return -1;
    }

    ctcpserver tcpserver;
    if (tcpserver.initserver(atoi(argv[1]) ) == false)
    {
        cout << "initserver failed\n" <<endl;return -1;
    }

    tcpserver.accept();

    string strwritebuffer,strreadbuffer;

    while(true)
    {
        if (tcpserver.read(strreadbuffer) == false) break;

        strwritebuffer = "回复: " + strreadbuffer;
        if (tcpserver.write(strwritebuffer) == false) break;
    }
}