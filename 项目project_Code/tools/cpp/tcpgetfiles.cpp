#include "_public.h"

using namespace idc;

struct st_arg
{
    int clienttype;    //1上传？还是2下载,本程序固定2
    char ip[31];
    int port;
    char clientpath[256];  
    int ptype;
    bool andchild;  //子目录
    char srvbakpath[256];     //0--增量，1--删除，2--备份
    char srvpath[256];
    char matchname[256];
    int timetvl;        //程序不退出，这是睡眠时间
    int timeout;
    char pname[51];
}starg;

clogfile logfile;
ctcpclient tcpclient;
string readbuffer;
string writebuffer;
cpactive pactive;

bool activetest();
void EXIT(int sig);
void help();
bool xmltoarg(const string& xmlbuffer);
bool login(const char* xmlbuffer);

bool _tcpgetfiles();

bool recvfile(const string& filename,const string& mtime,const int filesize);

int main(int argc,char* argv[])
{
    if (argc != 3)
    {
        help();
        return -1;
    }

    signal(SIGINT,EXIT);signal(SIGTERM,EXIT);

    if (logfile.open(argv[1]) == false)
    {
        logfile.write("logfile.open(%s) failed\n",argv[1]);return -1;
    }

    if (xmltoarg(argv[2]) == false)
    {
        logfile.write("xmltoarg(%s)\n", argv[2]);return -1;
    }

    pactive.addpinfo(starg.timeout,starg.pname);    //心跳

    if (tcpclient.connect(starg.ip,starg.port) == false)
    {
        logfile.write("tcpclient.connect(%s,%d) failed\n",starg.ip,starg.port);return -1;
    }

    pactive.uptatime();

    if (login(argv[2]) == false)
    {
        logfile.write("login(%s) failed\n",argv[2]);return -1;
    }


    if (_tcpgetfiles() == false) 
    {
        logfile.write("_tcpputfiles() failed\n");EXIT(-1);
    }
    
    EXIT(0);
}

void help()
{
    printf("\n");
    printf("Example:./tcpgetfiles /log/idc/tcpgetfiles.log \"<ip>192.168.240.131</ip>"
            "<port>5005</port><clientpath>/tmp/client</clientpath><ptype>1</ptype>"
            "<matchname>*.xml,*.txt</matchname>"
            "<andchild>true</andchild><srvbakpath>/tmp/client</srvbakpath><timetvl>10</timetvl>"
            "<srvpath>/tmp/server</srvpath><timeout>50</timeout><pname>tcpgetfiles_surfdata</pname>\"\n\n"
            );
    
}



void EXIT(int sig)
{
    signal(2,SIG_IGN);signal(15,SIG_IGN);

    logfile.write("程序退出，sig=%d\n",sig);

    exit(0);
}

bool activetest()
{
    writebuffer = "<active>ok</active>";
    // logfile.write("发送: %s...",writebuffer.c_str());
    if (tcpclient.write(writebuffer) == false)
    {
        logfile << " ailed\n";return false;
    }
    // logfile << " ok\n";

    // logfile.write("接受: %s...",readbuffer.c_str());
    if (tcpclient.read(readbuffer,10) == false) 
    {
        logfile << " ailed\n";return false;
    }
    // logfile << " ok\n";

    return true;
}


bool xmltoarg(const string& xmlbuffer)
{
    memset(&starg,0,sizeof(starg));

    starg.clienttype = 2;

    getxmlbuffer(xmlbuffer,"ip",starg.ip,30);
    if (strlen(starg.ip) == 0)
    {
        logfile.write("ip is null or false\n");return false;
    }

    getxmlbuffer(xmlbuffer,"port",starg.port);
    if (starg.port == 0)
    {   
        logfile.write("port is null or false\n");return false;
    }

    getxmlbuffer(xmlbuffer,"clientpath",starg.clientpath,255);
    if (strlen(starg.clientpath) == 0)
    {
        logfile.write("clientpath is null or false\n");return false;
    }

    getxmlbuffer(xmlbuffer,"matchname",starg.matchname,255);
    if (strlen(starg.matchname) == 0)
    {
        logfile.write("matchname is null or false\n");return false;
    }
    

    getxmlbuffer(xmlbuffer,"srvpath",starg.srvpath,255);
    if (strlen(starg.srvpath) == 0)
    {
        logfile.write("srvpath is null or false\n");return false;
    }

    getxmlbuffer(xmlbuffer,"timetvl",starg.timetvl);
    if (starg.timetvl >30)  starg.timetvl = 30;
    if (starg.timetvl <= 0)
    {
        logfile.write("timetvl is null or false\n");return false;
    }

    getxmlbuffer(xmlbuffer,"timeout",starg.timeout);
    if (starg.timeout <= starg.timetvl) 
    {
        logfile.write("timeout(%d) is lower than timetvl(%d), false",starg.timeout,starg.timetvl);return false;
    }
    if (starg.timeout <= 0)
    {
        logfile.write("timeout is null or false\n");return false;
    }

    getxmlbuffer(xmlbuffer,"panme",starg.pname,50);

    getxmlbuffer(xmlbuffer,"andchild",starg.andchild);
    if (starg.andchild != true) starg.andchild = false;

    getxmlbuffer(xmlbuffer,"ptype",starg.ptype);
    if (starg.ptype == 2)
    {
        getxmlbuffer(xmlbuffer,"clientbakpath",starg.srvbakpath,255);
        if (strlen(starg.srvbakpath) == 0)
        {
            logfile.write("clientbakpath is null or false\n");return false;
        }
    }

    return true;
}

bool login(const char* xmlbuffer)
{
    sformat(writebuffer,"%s<clienttype>2</clienttype>",xmlbuffer);
    
    //logfile.write("login() send %s...",writebuffer.c_str());
    if (tcpclient.write(writebuffer) == false)
    {
        logfile.write("client send %s failed\n",writebuffer.c_str());return false;
    }
    //logfile.write("ok\n");

    if (tcpclient.read(readbuffer,10) == false)
    {
        logfile.write("login() recv %s...failed\n",readbuffer.c_str());return false;
    }

    logfile.write("登陆成功(%s %d)\n",starg.ip,starg.port);

    return true;
}

bool _tcpgetfiles()
{
    while(true)
    {
        pactive.uptatime();

        if (tcpclient.read(readbuffer,starg.timetvl + 10) == false) //读取报文
        {
            logfile.write("与服务端断开连接\n"); break;
        }
        //logfile.write("接受: %s ...ok\n",readbuffer.c_str());


        if (readbuffer == "<active>ok</active>")    //处理心跳
        {
            writebuffer = "ok";
            if (tcpclient.write(writebuffer) == false)
            {
                logfile.write("tcpclient.write() failed\n");break;
            }
            //logfile.write("发送: %s ...ok\n",writebuffer.c_str());
        }

        if (readbuffer.find("filename") != string::npos)    //下载服务端文件
        {
            string serverfilename;
            string mtime;
            int filesize;
            getxmlbuffer(readbuffer,"filename",serverfilename);
            getxmlbuffer(readbuffer,"mtime",mtime);
            getxmlbuffer(readbuffer,"size",filesize);

            string clientfilename;
            clientfilename = serverfilename;
         
            replacestr(clientfilename,starg.srvpath,starg.clientpath,false);
            //下载客户端文件
            logfile.write("接收文件(%s,%s,%d) ...",clientfilename.c_str(),mtime.c_str(),filesize);
            if (recvfile(clientfilename,mtime,filesize) == false)
            {
                logfile << " failed \n";
                sformat(writebuffer,"<filename>%s</filename><result>failed</result>",serverfilename.c_str());
            }
            else
            { 
                sformat(writebuffer,"<filename>%s</filename><result>ok</result>",serverfilename.c_str());
                logfile << " ok\n";
            }

            
            //logfile.write("发送%s ...",writebuffer.c_str());
            if (tcpclient.write(writebuffer) == false)
            {
                logfile << "failed \n";return false;
            }
            //ogfile << "ok \n";
        }
    }
    
    return true;
}


bool recvfile(const string& filename,const string& mtime,const int filesize)
{
    cofile ofile;
    if (ofile.open(filename,ios::binary) == false)
    {
        logfile.write("ofile.open(%s) false\n",filename.c_str());return false;
    }

    int totalbytes=0;
    int onread=0;
    char buffer[1000];int buflen = sizeof(buffer);

    while (filesize - totalbytes > 0)
    {
        if (filesize - totalbytes >= buflen) onread = buflen;
        else buflen = filesize - totalbytes;

        if (tcpclient.read(buffer,onread) == false)
        {
            logfile.write("client read(%s) failed\n",buffer);return false;
        }

        if (ofile.write(buffer,onread) == false)
        {
            logfile.write("ofile.write(%s,%d) failed\n",buffer,onread);return false;
        }

        totalbytes = totalbytes + onread;
    }

    ofile.closeandrename();
    setmtime(filename,mtime);

    return true;
}