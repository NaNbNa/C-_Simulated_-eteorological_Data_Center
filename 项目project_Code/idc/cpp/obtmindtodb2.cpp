#include "_public.h"
#include "_ooci.h"

using namespace idc;

struct st_zhobtmind
{
    char obtid[11];
    char ddatetime[21];
    char t[11];
    char p[11];
    char u[11];
    char wd[11];
    char wf[11];
    char r[11];
    char vis[11];
}stzhobtmind;

clogfile logfile;
connection conn;
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

    printf("Using: /project/idc/bin/obtmindtodb /idcdata/surfdata" \
            " \"idc/idcpwd@snorcl11g_131\" \"Simplified Chinese_China.AL32UTF8\" /log/idc/obtmindtodb.log\n\n");
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
    if (dir.opendir(pathname,"*.xml") == false)
    {
        logfile.write("dir.opendir(%s,'.xml') failed\n",pathname);return false;
    }

    sqlstatement stmt;
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
        stmt.connect(&conn);
        stmt.prepare("insert into T_ZHOBTMIND(obtid,ddatetime,t,p,u,wd,wf,r,vis,keyid) \
                     values(:1,to_date(:2,'yyyy-mm-dd hh24:mi:ss'),:3,:4,:5,:6,:7,:8,:9,SEQ_ZHOBTMIND.nextval)");

        stmt.bindin(1,stzhobtmind.obtid,5);stmt.bindin(2,stzhobtmind.ddatetime,14);
        stmt.bindin(3,stzhobtmind.t,10);stmt.bindin(4,stzhobtmind.p,10);
        stmt.bindin(5,stzhobtmind.u,10);stmt.bindin(6,stzhobtmind.wd,10);
        stmt.bindin(7,stzhobtmind.wf,10);stmt.bindin(8,stzhobtmind.r,10);
        stmt.bindin(9,stzhobtmind.vis,10);

        cifile  ifile;
        if (ifile.open(dir.m_ffilename) == false)
        {
            logfile.write("ifile.open(%s) failed\n",dir.m_ffilename.c_str());return false;
        }

        string buffer;
        int stzhobtmindlen = sizeof(st_zhobtmind);

        int totalcount =0,insertcount =0;ctimer timer;
        while(true)
        {
            if (ifile.readline(buffer,"<endl/>") == false) break;

            totalcount++;

            memset(&stzhobtmind,0,sizeof(stzhobtmindlen));
            getxmlbuffer(buffer,"obtid",stzhobtmind.obtid,5);
            getxmlbuffer(buffer,"ddatetime",stzhobtmind.ddatetime,14);

            char temp[11];
            getxmlbuffer(buffer,"t",temp,10);   if (strlen(temp) > 0) snprintf(stzhobtmind.t,10,"%d",(int)(atof(temp)*10));
            getxmlbuffer(buffer,"p",temp,10);   if (strlen(temp) > 0) snprintf(stzhobtmind.p,10,"%d",(int)(atof(temp)*10));
            getxmlbuffer(buffer,"u",stzhobtmind.u,10);
            getxmlbuffer(buffer,"wd",stzhobtmind.wd,10);
            getxmlbuffer(buffer,"wf",temp,10);  if (strlen(temp) > 0) snprintf(stzhobtmind.wf,10,"%d",(int)(atof(temp)*10));
            getxmlbuffer(buffer,"r",temp,10);   if (strlen(temp) > 0) snprintf(stzhobtmind.r,10,"%d",(int)(atof(temp)*10));
            getxmlbuffer(buffer,"vis",temp,10); if (strlen(temp) > 0) snprintf(stzhobtmind.vis,10,"%d",(int)(atof(temp)*10));

            if (stmt.execute() != 0)
            {
                if (stmt.rc() != 1) //一行出错数据错误，不要退出，继续处理下一行
                {
                    logfile.write("strbuffer=%s\n",buffer.c_str());
                    logfile.write("stmt.execute() failed\n%s\n%s\n",stmt.sql(),stmt.message());
                }
            }
            else
                insertcount++;
        }
        
        logfile.write("已处理文件数%s（处理记录数=%d,插入记录数=%d),耗时=%.2f秒\n",
                    dir.m_ffilename.c_str(),totalcount,insertcount,timer.elapsed());

        ifile.closeandremove();
        conn.commit();
    }

    return true;
}