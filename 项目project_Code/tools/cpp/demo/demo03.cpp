#include "_public.h"

using namespace idc;

ctcpclient client;

string readbuffer;
string writebuffer;

bool login(int timeout);

bool checkye(int timeout);

bool transfer(int timeout);

int main(int argc,char* argv[])
{
    if (argc != 4)
    {
        printf("Example:\n");
        printf("./demo03 192.168.240.131 5005 10\n");
        return -1;
    }

    if (client.connect(argv[1],atoi(argv[2])) == false)
    {
        printf("client.connect(%s,%d) failed\n",argv[1],atoi(argv[2]));return -1;
    }
    
    //1
    login(atoi(argv[3]));
    //2
    checkye(atoi(argv[3]));
    //sleep(11);
    //3
    transfer(atoi(argv[3]));
}

bool login(int timeout)
{
    writebuffer = "<retcode>1</retcode><username>CTB</username><password>1</password>";

    if (client.write(writebuffer) == false)
    {
        printf("client.writebuffer(%s) failed\n",writebuffer.c_str());return false;
    }
    cout << "client 发送：" << writebuffer << endl;


    if (client.read(readbuffer,timeout) == false)
    {
        printf("client.readbuffer() failed\n");return false;
    }
    cout << "client 接收：" << readbuffer <<endl;

    return true;
}

bool checkye(int timeout)
{
    writebuffer = "<retcode>2</retcode><cardid>123456</cardid>";

    if (client.write(writebuffer) == false)
    {
        printf("client.writebuffer(%s) failed\n",writebuffer.c_str());return false;
    }
    cout << "client 发送：" << writebuffer << endl;


    if (client.read(readbuffer,timeout) == false)
    {
        printf("client.readbuffer() failed\n");return false;
    }
    cout << "client 接收：" << readbuffer <<endl;

    return true;
}

bool transfer(int timeout)
{
    writebuffer = "<retcode>3</retcode><cardid1>123456</cardid1><cardid2>234567</cardid2>";

    if (client.write(writebuffer) == false)
    {
        printf("client.writebuffer(%s) failed\n",writebuffer.c_str());return false;
    }
    cout << "client 发送：" << writebuffer << endl;


    if (client.read(readbuffer,timeout) == false)
    {
        printf("client.readbuffer() failed\n");return false;
    }
    cout << "client 接收：" << readbuffer <<endl;

    return true;
}

