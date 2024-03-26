#include "tools.h"


struct st_arg
{
    char localconnstr[101];
    char charset[51];
    char linktname[31];
    char localtname[31];
    char remotecols[1001];
    char localcols[10001];
    char  where[1001];
    char remoteconnstr[101];
    char remotetname[31];
    char localkeycol[31];
    char remotekeycol[31];
    int timetvl;
    int maxcount;
    int timeout;
    char pname[51];
}starg;

clogfile logfile;

connection connloc;       // 本地数据库
connection connrem; 
long maxkeyvalue=0;

cpactive pactive;

void _help();

void EXIT(int sig);

bool _xmltoarg(const char *strxmlbuffer);

bool _syncinc(bool& bcontinue);

bool loadmaxkey();

int main(int argc,char *argv[])
{
    if (argc!=3) { _help(); return -1; }

    signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

    if (logfile.open(argv[1])==false)
    {
        printf("打开日志文件失败（%s）。\n",argv[1]); return -1;
    }

    if (_xmltoarg(argv[2])==false) return -1;

    pactive.addpinfo(starg.timeout,starg.pname);

    if (connloc.connecttodb(starg.localconnstr,starg.charset) != 0)
    {
        logfile.write("connect database(%s) failed.\n%s\n",starg.localconnstr,connloc.message()); EXIT(-1);
    }

    pactive.uptatime();

    if (connrem.connecttodb(starg.remoteconnstr,starg.charset) != 0)
    {
        logfile.write("connrem database(%s) failed.\n%s\n",starg.remoteconnstr,connrem.message()); EXIT(-1);
    }

    pactive.uptatime();

    if ((strlen(starg.remotecols) == 0) || strlen(starg.localcols) == 0)
    {
        ctcols tcols;

        if (tcols.allcols(connloc,starg.localtname) == false)
        {
            logfile.write("表%s不存在\n",starg.localtname);EXIT(-1);
        }

        if (strlen(starg.remotecols) == 0) strcpy(starg.remotecols,tcols.m_allcols.c_str());
        if (strlen(starg.localcols) == 0) strcpy(starg.localcols,tcols.m_allcols.c_str());
    }

    bool bcontinue;

    while (true)
    {
        if (_syncinc(bcontinue) == false) EXIT(-1);

        if (bcontinue == false) sleep(starg.timetvl);

        pactive.uptatime();
    }
    

}

bool _syncinc(bool& bcontinue)
{
    bcontinue = false;

    ctimer timer;
    
    if (loadmaxkey() == false) return false;

    //select ---------------remote connect 
    sqlstatement stmtsel(&connrem);
    stmtsel.prepare("select rowid from %s where %s>:1 %s order by %s ",starg.remotetname,starg.remotekeycol,starg.where,starg.remotekeycol);

    stmtsel.bindin(1,maxkeyvalue);

    char remrowidvalue[21];
    memset(remrowidvalue,0,sizeof(remrowidvalue));
    stmtsel.bindout(1,remrowidvalue,20);

    string bindstr;
    for (int ii=1;ii<=starg.maxcount;ii++)
    {
        bindstr = bindstr + sformat(":%lu,",ii);
    }
    deleterchr(bindstr,',');

    // del ---------------insert =============local connect
    string strins = "insert into ";
    strins = strins + starg.localtname + " (" + starg.localcols + ") " + " select " + starg.remotecols \
            + " from " + starg.linktname + " " + " where rowid in (" +  bindstr + ")";

    sqlstatement stmtins(&connloc);
    stmtins.prepare(strins);
    
    char rowidvalues[starg.maxcount][21];
    memset(rowidvalues,0,sizeof(rowidvalues));
    for (int ii = 1;ii<=starg.maxcount;ii++)
    {
        stmtins.bindin(ii,rowidvalues[ii-1],20);
    }

    // -----------------------start--------

    int ccount =0;

    if (stmtsel.execute() != 0)
    {
        logfile.write("stmtsel execute() failed\n%s\n%s\n",stmtsel.sql(),stmtsel.message()); return false;
    }
    
    
    while(true)
    {
        memset(remrowidvalue,0,sizeof(remrowidvalue));
        if (stmtsel.next() != 0) break;
        
        strcpy(rowidvalues[ccount],remrowidvalue);

        ccount ++;

        if (ccount == starg.maxcount)
        {   
            if (stmtins.execute() != 0)
            {
                logfile.write("stmtins execute() failed\n%s\n%s\n",stmtins.sql(),stmtins.message()); return false;
            }

            //logfile.write("从表%s 分批同步到 表%s(%d rows) in %.2f sec.\n",starg.linktname,starg.localtname,ccount,timer.elapsed());

            connloc.commit();

            ccount = 0;
            memset(rowidvalues,0,sizeof(rowidvalues));

            pactive.uptatime();
        }
    }
    
    if (ccount > 0 )
    {   
        if (stmtins.execute() != 0)
        {
            logfile.write("stmtins execute() failed\n%s\n%s\n",stmtins.sql(),stmtins.message()); return false;
        }

        //logfile.write("从表%s 分批同步到 表%s(%d rows) in %.2f sec.\n",starg.linktname,starg.localtname,ccount,timer.elapsed());

        connloc.commit();

        pactive.uptatime();
    }

    if (stmtsel.rpc() > 0)
    {
        logfile.write("sync %s to %s(%d rows) in %.2fsec.\n",starg.linktname,starg.localtname,stmtsel.rpc(),timer.elapsed());logfile << " " << stmtsel.rpc() << "rows in " << timer.elapsed() << " sec\n";
        bcontinue = true;
    }

    return true;
}

bool loadmaxkey()
{
    maxkeyvalue = 0;
    sqlstatement stmt(&connloc);
    stmt.prepare("select max(%s) from %s ",starg.localkeycol,starg.localtname);
    
    stmt.bindout(1,maxkeyvalue);
    if (stmt.execute() != 0)
    {
        logfile.write("stmt.execute() failed\n%s\n%s\n",stmt.sql(),stmt.message());return false;
    }
    stmt.next();

    return true;
}
bool _xmltoarg(const char *strxmlbuffer)
{
    memset(&starg,0,sizeof(struct st_arg));

    getxmlbuffer(strxmlbuffer,"localconnstr",starg.localconnstr,100);
    if (strlen(starg.localconnstr)==0) { logfile.write("localconnstr is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"charset",starg.charset,50);
    if (strlen(starg.charset)==0) { logfile.write("charset is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"linktname",starg.linktname,30);
    if (strlen(starg.linktname)==0) { logfile.write("linktname is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"localtname",starg.localtname,30);
    if (strlen(starg.localtname)==0) { logfile.write("localtname is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"remotecols",starg.remotecols,1000);

    getxmlbuffer(strxmlbuffer,"localcols",starg.localcols,1000);

    getxmlbuffer(strxmlbuffer,"where",starg.where,1000);

    getxmlbuffer(strxmlbuffer,"remoteconnstr",starg.remoteconnstr,100);
    if (strlen(starg.remoteconnstr)==0) { logfile.write("remoteconnstr is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"remotetname",starg.remotetname,30);
    if (strlen(starg.remotetname)==0) { logfile.write("remotetname is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"remotekeycol",starg.remotekeycol,30);
    if (strlen(starg.remotekeycol)==0) { logfile.write("remotekeycol is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"localkeycol",starg.localkeycol,30);
    if (strlen(starg.localkeycol)==0) { logfile.write("localkeycol is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"maxcount",starg.maxcount);
    if (starg.maxcount==0) { logfile.write("maxcount is null.\n"); return false; }
    
    getxmlbuffer(strxmlbuffer,"timetvl",starg.timetvl);
    if (starg.timetvl<=0) { logfile.write("timetvl is null.\n"); return false; }
    if (starg.timetvl>30) starg.timetvl=30;   

    getxmlbuffer(strxmlbuffer,"timeout",starg.timeout);
    if (starg.timeout==0) { logfile.write("timeout is null.\n"); return false; }

    if (starg.timeout<starg.timetvl+10) starg.timeout=starg.timetvl+10;

    getxmlbuffer(strxmlbuffer,"pname",starg.pname,50);
    if (strlen(starg.pname)==0) { logfile.write("pname is null.\n"); return false; }

    return true;
}
void _help()
{
    printf("把T_ZHOBTMIND1@db131表中全部的记录增量同步到T_ZHOBTMIND2中。\n");
    printf("        /project/tools/bin/syncinc /log/idc/syncinc_ZHOBTMIND2.log "\
              "\"<localconnstr>idc/idcpwd@snorcl11g_131</localconnstr><remoteconnstr>idc/idcpwd@snorcl11g_131</remoteconnstr>"\
              "<charset>Simplified Chinese_China.AL32UTF8</charset>"\
              "<remotetname>T_ZHOBTMIND1</remotetname><linktname>T_ZHOBTMIND1@db131</linktname>"\
              "<localtname>T_ZHOBTMIND2</localtname>"\
              "<remotecols>obtid,ddatetime,t,p,u,wd,wf,r,vis,uptime,keyid</remotecols>"\
              "<localcols>stid,ddatetime,t,p,u,wd,wf,r,vis,uptime,recid</localcols>"\
              "<remotekeycol>keyid</remotekeycol><localkeycol>recid</localkeycol>"\
              "<maxcount>300</maxcount><timetvl>2</timetvl><timeout>50</timeout><pname>syncinc_ZHOBTMIND2</pname>\"\n\n");

    printf("把T_ZHOBTMIND1@db131表中满足\"and obtid like '57%%'\"的记录增量同步到T_ZHOBTMIND3中。\n");
    printf("       /project/tools/bin/syncinc /log/idc/syncinc_ZHOBTMIND3.log "\
              "\"<localconnstr>idc/idcpwd@snorcl11g_131</localconnstr><remoteconnstr>idc/idcpwd@snorcl11g_131</remoteconnstr>"\
              "<charset>Simplified Chinese_China.AL32UTF8</charset>"\
              "<remotetname>T_ZHOBTMIND1</remotetname><linktname>T_ZHOBTMIND1@db131</linktname>"\
              "<localtname>T_ZHOBTMIND3</localtname>"\
              "<remotecols>obtid,ddatetime,t,p,u,wd,wf,r,vis,uptime,keyid</remotecols>"\
              "<localcols>stid,ddatetime,t,p,u,wd,wf,r,vis,uptime,recid</localcols>"\
              "<where>and obtid like '54%%%%'</where>"\
              "<remotekeycol>keyid</remotekeycol><localkeycol>recid</localkeycol>"\
              "<maxcount>300</maxcount><timetvl>2</timetvl><timeout>30</timeout><pname>syncinc_ZHOBTMIND3</pname>\"\n\n");

}

void EXIT(int sig)
{
    logfile.write("程序退出，sig=%d\n\n",sig);

    connloc.disconnect();

    exit(0);
}