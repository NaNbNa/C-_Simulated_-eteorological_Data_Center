#include "_public.h"

using namespace idc;

struct st_arg
{
    int clienttype;    //1上传？还是2下载,本程序固定1
    char ip[31];
    int port;
    char clientpath[256];  
    int ptype;
    bool andchild;  //子目录
    char clientbakpath[256];     //0--增量，1--删除，2--备份
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

bool _tcpputfiles(bool& bcontinue);
bool ackmessage(const string& strreadbuffer);

bool sendfile(const string& filename,const int filesize);
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

    bool bcontinue = true;
    while(true)
    {

        if (_tcpputfiles(bcontinue) == false) 
        {
            logfile.write("_tcpputfiles() failed\n");EXIT(-1);
        }

        if (bcontinue == false)     //如果本次循环有上传文件，就不休眠
            sleep(starg.timetvl);

        if (activetest() ==false ) break;

        pactive.uptatime();
    }

}

void help()
{
    printf("\n");
    printf("Example:./tcpputfiles /log/idc/tcpputfiles.log \"<ip>192.168.240.131</ip>"
            "<port>5005</port><clientpath>/tmp/client</clientpath><ptype>1</ptype>"
            "<matchname>*.xml,*.txt</matchname>"
            "<andchild>true</andchild><clientbakpath>/tmp/client</clientbakpath><timetvl>10</timetvl>"
            "<srvpath>/tmp/server</srvpath><timeout>50</timeout><pname>tcpputfiles_surfdata</pname>\"\n\n"
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

    starg.clienttype = 1;

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
        getxmlbuffer(xmlbuffer,"clientbakpath",starg.clientbakpath,255);
        if (strlen(starg.clientbakpath) == 0)
        {
            logfile.write("clientbakpath is null or false\n");return false;
        }
    }

    return true;
}

bool login(const char* xmlbuffer)
{
    sformat(writebuffer,"%s<clienttype>1</clienttype>",xmlbuffer);
    
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

bool _tcpputfiles(bool& bcontinue)
{
    bcontinue = false;

    cdir dir;

    if (dir.opendir(starg.clientpath,starg.matchname,10000,starg.andchild) == false)
    {
        logfile.write("cdir opendir(%s) false\n",starg.clientpath);return false;
    }

    int delayed = 0; //未接受确认报文数

    while(dir.readdir() == true)
    {
        bcontinue = true;

        sformat(writebuffer,"<filename>%s</filename><mtime>%s</mtime><size>%d</size>",\
                            dir.m_ffilename.c_str(),dir.m_mtime.c_str(),dir.m_filesize);
        
        if (tcpclient.write(writebuffer) == false)  //文本发送---文件信息
        {
            logfile.write("client send(%s) failed\n",writebuffer.c_str());return false;
        }
        

        //发送文件内容---------------二进制发送
        logfile.write("sendfile (%s,%d) ...",dir.m_ffilename.c_str(),dir.m_filesize);
        if (sendfile(dir.m_ffilename,dir.m_filesize) == false)
        {
            logfile <<  "failed\n";return false;
        }
        logfile << "ok \n";delayed ++ ;
        
        pactive.uptatime();

        while(delayed > 0)
        {
            if (tcpclient.read(readbuffer,-1) == false) break;
            //logfile.write("readbuffer=%s",readbuffer.c_str());
            delayed--;
            ackmessage(readbuffer);
        }
    }

    while(delayed > 0)
    {
        if (tcpclient.read(readbuffer,10) == false) break;
        //logfile.write("readbuffer=%s",readbuffer.c_str());
        delayed --;
        ackmessage(readbuffer);
    }

    return true;
}
bool ackmessage(const string& strreadbuffer)
{
    string result;
    string filename;
    getxmlbuffer(strreadbuffer,"result",result);
    getxmlbuffer(strreadbuffer,"filename",filename);

    if (result != "ok") return true; //等待下次传输

    if (starg.ptype == 1)
    {
        if (remove(filename.c_str()) != 0)
        {
            logfile.write("remove(%s) failed\n",filename.c_str());return false;
        }
    }
    

    if (starg.ptype == 2)
    {
        string bakfilename = filename;  //绝对路径
        replacestr(bakfilename,starg.clientpath,starg.clientbakpath,false);

        if (renamefile(filename,bakfilename) == false)
        {
            logfile.write("renamefile(%s %s) failed\n",filename.c_str(),bakfilename.c_str());
            return false;
        }
    }

    return true;
}

bool sendfile(const string& filename,const int filesize)
{
    cifile ifile;
    if (ifile.open(filename,ios::binary) == false)
    {
        logfile.write("ifile.open(%s,ios::binary) failed\n",filename.c_str());return false;
    }

    int totalbytes=0;
    int onread;
    char buffer[1000];int buflen = sizeof(buffer);

    while(filesize - totalbytes > 0)
    {
        if (filesize - totalbytes >= buflen) onread = buflen;
        else onread = filesize - totalbytes;

        ifile.read(buffer,onread);

        if (tcpclient.write(buffer,onread) == false)    //需要二进制
        {
            logfile.write("client.write(%s) failed\n",buffer);return false;
        }

        totalbytes = totalbytes + onread;
    }

    return true;
}