// #include "_public.h"
// #include "_ooci.h"
#include "idcapp.h"
using namespace idc;

connection conn;
clogfile logfile;
void EXIT(int sig);
void help();

bool _botmindtodb(const char* pathname,const char* connstr,const char* charset);

int main(int argc,char* argv[])
{
    if (argc != 5)
    {
        help();return -1;
    }
    
    closeioandsignal();
    signal(2,EXIT);signal(15,EXIT);

    if (logfile.open(argv[4]) == false)
    {
        printf("logfile.open(%s) failed\n",argv[4]);
    }

    _botmindtodb(argv[1],argv[2],argv[3]);
}

void help()
{
    printf("\n");

    printf("Using: /project/idc/bin/obtmindtodb2 /idcdata/surfdata/temp" \
            " \"idc/idcpwd@snorcl11g_131\" \"Simplified Chinese_China.AL32UTF8\" /log/idc/obtmindtodb2.log\n\n");
}

void EXIT(int sig)
{
    signal(2,SIG_IGN);signal(15,SIG_IGN);
    printf("程序退出，sig=%d\n",sig);

    exit(0);
}

bool _botmindtodb(const char* pathname,const char* connstr,const char* charset)
{
    cdir dir;
    //if (dir.opendir(pathname,"*.xml") == false)
    if (dir.opendir(pathname,"*.xml,*.csv") == false)
    {
        logfile.write("dir.opendir(%s,'.xml') failed\n",pathname);return false;
    }

    CZHOBTMIND ZHOBTMIND(conn,logfile);

    while(true)
    {
        if (dir.readdir() == false) break;

        if (conn.isopen() == false)
        {
            if (conn.connecttodb(connstr,charset)!= 0)
            {
                logfile.write("conn.connect database(%s,%s) failed\n",connstr,charset);return false;
            }
            logfile.write("connect database(%s,%s) ok\n",connstr,charset);
        }
        

        cifile  ifile;
        if (ifile.open(dir.m_ffilename) == false)
        {
            logfile.write("ifile.open(%s) failed\n",dir.m_ffilename.c_str());return false;
        }

        string buffer;

        int totalcount =0,insertcount =0;ctimer timer;
        bool bisxml = false;    //false-csv,true-xml
        if (matchstr(dir.m_filename,"*.xml") == true) bisxml = true;
        if (bisxml == false) ifile.readline(buffer);
        while(true)
        {
            if (bisxml == true)
            {
                if (ifile.readline(buffer,"<endl/>") == false) break;
            }
            else
            {
                if (ifile.readline(buffer,"") == false) break;
            }

            totalcount++;

            ZHOBTMIND.splitbuffer(buffer,bisxml);

            if (ZHOBTMIND.inserttable() == true) insertcount++;
            
        }
        ifile.closeandremove();
        conn.commit();
        logfile.write("已处理文件:%s（处理记录数=%d,插入记录数=%d),耗时=%.2f秒\n",
                        dir.m_filename.c_str(),totalcount,insertcount,timer.elapsed());
    }

    return true;
}
