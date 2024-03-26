#include "_public.h"

using namespace idc;

struct st_arg
{
    int clienttype;    //1上传？还是2下载
    // char ip[31];
    // int port;
    char clientpath[256];  
    int ptype;
    bool andchild;  //子目录
    char srvbakpath[256];     //0--增量，1--删除，2--备份
    char matchname[256];
    char srvpath[256];
    int timetvl;        //程序不退出，这是睡眠时间
    int timeout;
    char pname[51];
}starg;

ctcpserver tcpserver;
clogfile logfile;
string readbuffer;
string writebuffer;

cpactive pactive;

void EXIT(int sig);
void CHLDEXIT(int sig);

bool clientlogin();
bool recvfilemain();

bool sendfilemain();
bool _tcpputfiles(bool& bcontinue);
bool ackmessage(const string& strreadbuffer);
bool activetest();

bool recvfile(const string& filename,const string& mtime,const int filesize);
bool sendfile(const string& filename,const int filesize);

int main(int argc,char* argv[])
{
    if (argc !=3)
    {
        printf("\nExample: ./fileserver 5005 /log/idc/fileserver.log\n");
        return -1;
    }

    signal(2,EXIT);signal(15,EXIT);

    if (logfile.open(argv[2]) == false)
    {
        logfile.write("logfile.open(%s) failed\n",argv[2]);return -1;
    }

    if (tcpserver.initserver(atoi(argv[1])) == false)
    {
        logfile.write("tcpserver.initserver(%s) failed\n",argv[1]);return -1;
    }

    while(true)
    {
        if (tcpserver.accept() == false)
        {
            logfile.write("tcpserver.accept() failed\n");break;
        }

        if (fork() >0) { tcpserver.closeclient(); continue;}
        signal(2,CHLDEXIT);signal(15,CHLDEXIT);
        tcpserver.closelisten();

        if (clientlogin() == false) 
        {
            logfile.write("client login() failed\n");CHLDEXIT(-1);
        }

        pactive.addpinfo(starg.timeout,starg.pname);
        
        if (starg.clienttype == 1)  recvfilemain(); //get
        
        if (starg.clienttype == 2) sendfilemain();   //put

        pactive.uptatime();
        CHLDEXIT(0);
    }

   

}

void EXIT(int sig)
{
    signal(2,SIG_IGN); signal(15,SIG_IGN);

    logfile.write("程序退出，sig=%d\n",sig);

    tcpserver.closelisten();

    kill(0,15);
    exit(0);
}

void CHLDEXIT(int sig)
{
    signal(2,SIG_IGN); signal(15,SIG_IGN);

    logfile.write("程序退出，sig=%d\n",sig);

    tcpserver.closeclient();
    exit(0);
}

bool recvfilemain()
{
    while(true)
    {
        pactive.uptatime();

        if (tcpserver.read(readbuffer,starg.timetvl + 10) == false) //读取报文
        {
            logfile.write("客户端断开连接\n"); break;
        }
        //logfile.write("接受: %s ...ok\n",readbuffer.c_str());


        if (readbuffer == "<active>ok</active>")    //处理心跳
        {
            writebuffer = "ok";
            if (tcpserver.write(writebuffer) == false)
            {
                logfile.write("tcpserver.write() failed\n");break;
            }
            //logfile.write("发送: %s ...ok\n",writebuffer.c_str());
        }

        if (readbuffer.find("filename") != string::npos)    //下载文件
        {
            string clientfilename;
            string mtime;
            int filesize;
            getxmlbuffer(readbuffer,"filename",clientfilename);
            getxmlbuffer(readbuffer,"mtime",mtime);
            getxmlbuffer(readbuffer,"size",filesize);

            string serverfilename;
            serverfilename = clientfilename;
         
            replacestr(serverfilename,starg.clientpath,starg.srvpath,false);
            //下载客户端文件
            logfile.write("接收文件(%s,%s,%d) ...",serverfilename.c_str(),mtime.c_str(),filesize);
            if (recvfile(serverfilename,mtime,filesize) == false)
            {
                logfile << " failed \n";tcpserver.closeclient();
                sformat(writebuffer,"<filename>%s</filename><result>failed</result>",clientfilename.c_str());
            }
            else
            { 
                sformat(writebuffer,"<filename>%s</filename><result>ok</result>",clientfilename.c_str());
                logfile << " ok\n";
            }

            
            //logfile.write("发送%s ...",writebuffer.c_str());
            if (tcpserver.write(writebuffer) == false)
            {
                logfile << "failed \n";return false;
            }
            //ogfile << "ok \n";
        }
    }
    
    return true;
}

bool sendfilemain()
{
    bool bcontinue = true;
    while(true)
    {

        if (_tcpputfiles(bcontinue) == false) 
        {
            logfile.write("_tcpputfiles() failed\n");CHLDEXIT(-1);
        }

        if (bcontinue == false)     //如果本次循环有上传文件，就不休眠
            sleep(starg.timetvl);

        if (activetest() == false ) break;

        pactive.uptatime();
    }

    return true;
}

bool _tcpputfiles(bool& bcontinue)
{
    bcontinue = false;
    cdir dir;

    if (dir.opendir(starg.srvpath,starg.matchname,10000,starg.andchild) == false)
    {
        logfile.write("cdir opendir(%s) false\n",starg.srvpath);return false;
    }

    int delayed = 0; //未接受确认报文数

    while(dir.readdir() == true)
    {
        bcontinue = true;
        sformat(writebuffer,"<filename>%s</filename><mtime>%s</mtime><size>%d</size>",\
                            dir.m_ffilename.c_str(),dir.m_mtime.c_str(),dir.m_filesize);
        
        if (tcpserver.write(writebuffer) == false)  //文本发送---文件信息
        {
            logfile.write("client send(%s) failed\n",writebuffer.c_str());return false;
        }
        

        //发送文件内容---------------二进制发送
        logfile.write("发送文件 (%s,%d) ...",dir.m_ffilename.c_str(),dir.m_filesize);
        if (sendfile(dir.m_ffilename,dir.m_filesize) == false)
        {
            logfile <<  "failed\n";return false;
        }
        logfile << "ok \n";delayed ++ ;
        
        pactive.uptatime();

        while(delayed > 0)
        {
            if (tcpserver.read(readbuffer,-1) == false) break;
            //logfile.write("readbuffer=%s",readbuffer.c_str());
            delayed--;
            ackmessage(readbuffer);
        }
    }

    while(delayed > 0)
    {
        if (tcpserver.read(readbuffer,10) == false) break;
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
        replacestr(bakfilename,starg.srvpath,starg.srvbakpath,false);

        if (renamefile(filename,bakfilename) == false)
        {
            logfile.write("renamefile(%s %s) failed\n",filename.c_str(),bakfilename.c_str());
            return false;
        }
    }

    return true;
}

bool activetest()
{
    writebuffer = "<active>ok</active>";
    // logfile.write("发送: %s...",writebuffer.c_str());
    if (tcpserver.write(writebuffer) == false)
    {
        logfile << " ailed\n";return false;
    }
    // logfile << " ok\n";

    // logfile.write("接受: %s...",readbuffer.c_str());
    if (tcpserver.read(readbuffer,10) == false) 
    {
        logfile << " ailed\n";return false;
    }
    // logfile << " ok\n";

    return true;
}

bool clientlogin() 
{
    if (tcpserver.read(readbuffer,10) == false)
    {
        logfile.write("server read(%s) failed\n",readbuffer.c_str());return false;
    }

    //logfile.write("客戶端登陸：xml=%s\n",readbuffer.c_str());
    memset(&starg,0,sizeof(st_arg));
    getxmlbuffer(readbuffer,"clienttype",starg.clienttype);
    getxmlbuffer(readbuffer,"srvpath",starg.srvpath,255);
    getxmlbuffer(readbuffer,"clientpath",starg.clientpath,255);

    getxmlbuffer(readbuffer,"timetvl",starg.timetvl);
    getxmlbuffer(readbuffer,"timeout",starg.timeout);
    getxmlbuffer(readbuffer,"pname",starg.pname,50);
    getxmlbuffer(readbuffer,"matchname",starg.matchname,255);
    getxmlbuffer(readbuffer,"srvbakpath",starg.srvbakpath,255);
    getxmlbuffer(readbuffer,"andchild",starg.andchild);
    getxmlbuffer(readbuffer,"ptype",starg.ptype);
    //logfile.write("srvpath=%s\n",starg.srvpath);

    if ( (starg.clienttype != 1) && starg.clienttype != 2)
    {
        writebuffer = "false";
    }
    else 
        writebuffer = "ok";
    
    if (tcpserver.write(writebuffer) == false)
    {
        logfile.write("server send(%s) failed\n",writebuffer.c_str());return false;
    }

    logfile.write("客戶端%s 登陸n %s\n发送内容%s\n",tcpserver.getip(),writebuffer.c_str(),readbuffer.c_str());
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

        if (tcpserver.read(buffer,onread) == false)
        {
            logfile.write("server read(%s) failed\n",buffer);return false;
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

        if (tcpserver.write(buffer,onread) == false)    //需要二进制
        {
            logfile.write("client.write(%s) failed\n",buffer);return false;
        }

        totalbytes = totalbytes + onread;
    }

    return true;
}