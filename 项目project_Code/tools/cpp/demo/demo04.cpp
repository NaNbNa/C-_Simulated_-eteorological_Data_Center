#include "_public.h"

using namespace idc;

ctcpserver server;

string readbuffer;
string writebuffer;

void EXIT(int sig);
void CHLDEXIT(int sig);

void workmain();
bool work1();
bool work2();
bool work3();

int main(int argc,char* argv[])
{
    if (argc != 3)
    {
        printf("Example:\n");
        printf("./demo04 5005 10\n");
        return -1;
    }

    if (server.initserver(atoi(argv[1])) == false)
    {
        printf("server.initserver(%d) failed\n",atoi(argv[2]));return -1;
    }

    closeioandsignal();
    signal(SIGINT,EXIT);signal(SIGTERM,EXIT);

    
    while(true)
    {
        if (server.accept() == false)
        {
            printf("server.accept() failed\n");return -1;
        }

        printf("客户端（%s)已连接\n",server.getip());

        if (fork()>0) { server.closeclient(); continue;}

        server.closelisten();
        signal(SIGINT,CHLDEXIT);signal(SIGTERM,CHLDEXIT);

        while(true)
        {
            if (server.read(readbuffer,atoi(argv[2])) == false)
            {
                printf("server read() failed\n"); break;
            }
            cout << "server 接收：" << readbuffer <<endl;
            //事务
            
            workmain();
            sleep(1);

            if (server.write(writebuffer) == false)
            {
                printf("server write() failed\n");break;
            }
            cout << "server 发送：" << writebuffer <<endl;
        }

        CHLDEXIT(0);
    }
}

void EXIT(int sig)
{
    signal(2,SIG_IGN); signal(15,SIG_IGN);

    printf("sig=%d\n",sig);

    server.closelisten();

    kill(0,15);
    exit(0);
}

void CHLDEXIT(int sig)
{
    signal(2,SIG_IGN); signal(15,SIG_IGN);

    printf("sig=%d\n",sig);

    server.closeclient();

    exit(0);
}

void workmain()
{
    int ret;
    getxmlbuffer(readbuffer,"retcode",ret);

    switch (ret)
    {
    case 1:
        work1();
        break;
    case 2:
        work2();
        break;
    case 3:
        work3();
        break;
    
    default:
        
        break;
    }
}

bool work1()
{
    string username,passwd;
    getxmlbuffer(readbuffer,"username",username);
    getxmlbuffer(readbuffer,"password",passwd);

    if (username == "CTB" && passwd == "1")
    {
        writebuffer = "<retcode>0</retcode><message>login sucess</message>";
    }
    else
        writebuffer = "<retcode>-1</retcode><message>login false</message>";

    return true;
}

bool work2()
{
    string cardid;
    getxmlbuffer(readbuffer,"cardid",cardid);

    if (cardid == "123456")
    {
        writebuffer = "<retcode>0</retcode><message>checkye sucess</message>";
    }
    else
        writebuffer = "<retcode>-1</retcode><message>checkye false</message>";

    return true;
}


bool work3()
{
    string cardid1,cardid2;
    getxmlbuffer(readbuffer,"cardid1",cardid1);
    getxmlbuffer(readbuffer,"cardid2",cardid2);

    if (cardid1 == "123456" && cardid2 == "234567")
    {
        writebuffer = "<retcode>0</retcode><message>transfer sucess</message>";
    }
    else
        writebuffer = "<retcode>-1</retcode><message>transfer false</message>";

    return true;
}