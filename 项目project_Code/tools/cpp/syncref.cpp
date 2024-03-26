#include "tools.h"


struct st_arg
{
    char localconnstr[101];
    char charset[51];
    char linktname[31];
    char localtname[31];
    char remotecols[1001];
    char localcols[10001];
    char  rwhere[1001];
    char lwhere[1001];
    int synctype;
    char remoteconnstr[101];
    char remotetname[31];
    char localkeycol[31];
    char remotekeycol[31];
    int keylen;
    int maxcount;
    int timeout;
    char pname[51];
}starg;
clogfile logfile;

connection connloc;       // 本地数据库
connection connrem; 

cpactive pactive;

void _help();

void EXIT(int sig);

bool _xmltoarg(const char *strxmlbuffer);

bool _syncref();

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

    _syncref();

}

bool _syncref()
{
    ctimer timer;
    
    if (starg.synctype == 1)
    {
        logfile.write("从表%s 一次同步到 表%s...",starg.linktname,starg.localtname);

        sqlstatement stmtdel(&connloc),stmtins(&connloc);

        string strdel = " delete from ",strins = "insert into ";
        strdel = strdel + starg.localtname + " " + starg.lwhere;
        strins = strins + starg.localtname + " (" + starg.localcols + ") " + " select " + starg.remotecols \
                + " from " + starg.linktname + " " + starg.rwhere;
        stmtdel.prepare(strdel);
        if (stmtdel.execute() != 0)
        {
            logfile.write("stmtdel execute() failed\n%s\n%s\n",stmtdel.sql(),stmtdel.message());return false;
        }

        stmtins.prepare(strins);
        if (stmtins.execute() != 0)
        {
            logfile.write("stmtins execute() failed\n%s\n%s\n",stmtins.sql(),stmtins.message());
            connloc.rollback();
            return false;
        }
        
        logfile << " " << stmtins.rpc() << "rows in " << timer.elapsed() << "sec.\n";
        
        connloc.commit();

        return true;
    }

    if (connrem.connecttodb(starg.remoteconnstr,starg.charset) != 0)
    {
        logfile.write("connrem database(%s) failed.\n%s\n",starg.remoteconnstr,connrem.message()); EXIT(-1);
    }

    pactive.uptatime();
    //select ---------------remote connect 
    sqlstatement stmtsel(&connrem);
    stmtsel.prepare("select %s from %s %s ",starg.remotekeycol,starg.remotetname,starg.rwhere);

    char remkeyvalue[starg.keylen + 1];
    memset(remkeyvalue,0,sizeof(remkeyvalue));
    stmtsel.bindout(1,remkeyvalue,starg.keylen);

    string bindstr;
    for (int ii=1;ii<=starg.maxcount;ii++)
    {
        bindstr = bindstr + sformat(":%lu,",ii);
    }
    deleterchr(bindstr,',');

    // del ---------------insert =============local connect
    string strdel = " delete from ",strins = "insert into ";
    strdel = strdel + starg.localtname + " " + " where " + starg.localkeycol + " in (" +  bindstr + ")";
    strins = strins + starg.localtname + " (" + starg.localcols + ") " + " select " + starg.remotecols \
            + " from " + starg.linktname + " " + " where " + starg.remotekeycol + " in (" +  bindstr + ")";

    
    sqlstatement stmtdel(&connloc),stmtins(&connloc);
    stmtdel.prepare(strdel);stmtins.prepare(strins);
    
    char keyvalues[starg.maxcount][starg.keylen + 1];
    memset(remkeyvalue,0,sizeof(keyvalues));
    for (int ii = 1;ii<=starg.maxcount;ii++)
    {
        stmtdel.bindin(ii,keyvalues[ii-1],starg.keylen);
        stmtins.bindin(ii,keyvalues[ii-1],starg.keylen);
    }

   
    // -----------------------start--------

    int ccount =0;

    if (stmtsel.execute() != 0)
    {
        logfile.write("stmtsel execute() failed\n%s\n%s\n",stmtsel.sql(),stmtsel.message()); return false;
    }

    logfile.write("从表%s 分批同步到 表%s...",starg.linktname,starg.localtname);
    while(true)
    {   
        memset(remkeyvalue,0,sizeof(remkeyvalue));
        if (stmtsel.next() != 0) break;

        strcpy(keyvalues[ccount],remkeyvalue);

        ccount ++;

        if (ccount == starg.maxcount)
        {   
            if (stmtdel.execute() != 0)
            {
                logfile.write("stmtdel execute() failed\n%s\n%s\n",stmtdel.sql(),stmtdel.message()); return false;
            }

            if (stmtins.execute() != 0)
            {
                logfile.write("stmtins execute() failed\n%s\n%s\n",stmtins.sql(),stmtins.message()); return false;
            }

            //logfile.write("从表%s 分批同步到 表%s(%d rows) in %.2f sec.\n",starg.linktname,starg.localtname,ccount,timer.elapsed());

            connloc.commit();

            ccount = 0;
            memset(keyvalues,0,sizeof(keyvalues));

            pactive.uptatime();
        }
    }
    
    if (ccount > 0 )
    {   
        if (stmtdel.execute() != 0)
        {
            logfile.write("stmtdel execute() failed\n%s\n%s\n",stmtdel.sql(),stmtdel.message()); return false;
        }

        if (stmtins.execute() != 0)
        {
            logfile.write("stmtins execute() failed\n%s\n%s\n",stmtins.sql(),stmtins.message()); return false;
        }

        //logfile.write("从表%s 分批同步到 表%s(%d rows) in %.2f sec.\n",starg.linktname,starg.localtname,ccount,timer.elapsed());

        connloc.commit();

        pactive.uptatime();
    }

    logfile << " " << stmtsel.rpc() << "rows in " << timer.elapsed() << " sec\n";

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

    getxmlbuffer(strxmlbuffer,"rwhere",starg.rwhere,1000);
    getxmlbuffer(strxmlbuffer,"lwhere",starg.lwhere,1000);

    getxmlbuffer(strxmlbuffer,"synctype",starg.synctype);
    if ( (starg.synctype!=1) && (starg.synctype!=2) ) { logfile.write("synctype is not in (1,2).\n"); return false; }

    if (starg.synctype==2)
    {
        getxmlbuffer(strxmlbuffer,"remoteconnstr",starg.remoteconnstr,100);
        if (strlen(starg.remoteconnstr)==0) { logfile.write("remoteconnstr is null.\n"); return false; }

        getxmlbuffer(strxmlbuffer,"remotetname",starg.remotetname,30);
        if (strlen(starg.remotetname)==0) { logfile.write("remotetname is null.\n"); return false; }

        getxmlbuffer(strxmlbuffer,"remotekeycol",starg.remotekeycol,30);
        if (strlen(starg.remotekeycol)==0) { logfile.write("remotekeycol is null.\n"); return false; }

        getxmlbuffer(strxmlbuffer,"localkeycol",starg.localkeycol,30);
        if (strlen(starg.localkeycol)==0) { logfile.write("localkeycol is null.\n"); return false; }

        getxmlbuffer(strxmlbuffer,"keylen",starg.keylen);
        if (starg.keylen==0) { logfile.write("keylen is null.\n"); return false; }

        getxmlbuffer(strxmlbuffer,"maxcount",starg.maxcount);
        if (starg.maxcount==0) { logfile.write("maxcount is null.\n"); return false; }
    }

    getxmlbuffer(strxmlbuffer,"timeout",starg.timeout);
    if (starg.timeout==0) { logfile.write("timeout is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"pname",starg.pname,50);
    if (strlen(starg.pname)==0) { logfile.write("pname is null.\n"); return false; }

    return true;
}
void _help()
{
    printf("不分批同步，把T_ZHOBTCODE1@db131同步到T_ZHOBTCODE2。\n");
    printf("        /project/tools/bin/syncref /log/idc/syncref_ZHOBTCODE2.log "\
              "\"<localconnstr>idc/idcpwd@snorcl11g_131</localconnstr><charset>Simplified Chinese_China.AL32UTF8</charset>"\
              "<linktname>T_ZHOBTCODE1@db131</linktname><localtname>T_ZHOBTCODE2</localtname>"\
              "<remotecols>obtid,cityname,provnname,lat,lon,height,uptime,keyid</remotecols>"\
              "<localcols>stid,cityname,provnname,lat,lon,height,uptime,recid</localcols>"\
              "<rwhere>where obtid like '57%%%%'</rwhere><lwhere>where stid like '57%%%%'</lwhere>"
              "<synctype>1</synctype><timeout>50</timeout><pname>syncref_ZHOBTCODE2</pname>\"\n\n");
    
    printf("分批同步，把T_ZHOBTCODE1@db131同步到T_ZHOBTCODE3。\n");
    printf("       /project/tools/bin/syncref /log/idc/syncref_ZHOBTCODE3.log "\
              "\"<localconnstr>idc/idcpwd@snorcl11g_131</localconnstr><charset>Simplified Chinese_China.AL32UTF8</charset>"\
              "<linktname>T_ZHOBTCODE1@db131</linktname><localtname>T_ZHOBTCODE3</localtname>"\
              "<remotecols>obtid,cityname,provnname,lat,lon,height,uptime,keyid</remotecols>"\
              "<localcols>stid,cityname,provnname,lat,lon,height,uptime,recid</localcols>"\
              "<rwhere>where obtid like '57%%%%'</rwhere>"\
              "<synctype>2</synctype><remoteconnstr>idc/idcpwd@snorcl11g_131</remoteconnstr>"\
              "<remotetname>T_ZHOBTCODE1</remotetname><remotekeycol>obtid</remotekeycol>"\
              "<localkeycol>stid</localkeycol><keylen>5</keylen>"\
              "<maxcount>10</maxcount><timeout>50</timeout><pname>syncref_ZHOBTCODE3</pname>\"\n\n");

    printf("分批同步，把T_ZHOBTMIND1@db131同步到T_ZHOBTMIND2。\n");
    printf("       /project/tools/bin/syncref /log/idc/syncref_ZHOBTMIND2.log "\
              "\"<localconnstr>idc/idcpwd@snorcl11g_131</localconnstr><charset>Simplified Chinese_China.AL32UTF8</charset>"\
              "<linktname>T_ZHOBTMIND1@db131</linktname><localtname>T_ZHOBTMIND2</localtname>"\
              "<remotecols>obtid,ddatetime,t,p,u,wd,wf,r,vis,uptime,keyid</remotecols>"\
              "<localcols>stid,ddatetime,t,p,u,wd,wf,r,vis,uptime,recid</localcols>"\
              "<rwhere>where ddatetime>sysdate-10/1440</rwhere>"\
              "<synctype>2</synctype><remoteconnstr>idc/idcpwd@snorcl11g_131</remoteconnstr>"\
              "<remotetname>T_ZHOBTMIND1</remotetname><remotekeycol>keyid</remotekeycol>"\
              "<localkeycol>recid</localkeycol><keylen>15</keylen>"\
              "<maxcount>10</maxcount><timeout>50</timeout><pname>syncref_ZHOBTMIND2</pname>\"\n\n");

}

void EXIT(int sig)
{
    logfile.write("程序退出，sig=%d\n\n",sig);

    connloc.disconnect();

    exit(0);
}