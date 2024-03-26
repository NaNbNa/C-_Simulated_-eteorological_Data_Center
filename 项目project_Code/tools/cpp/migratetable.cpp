#include "tools.h"

using namespace idc;

struct st_arg
{
    char connstr[101];
    char tname[31];
    char totname[31];
    char keycol[31];
    char where[1001];
    int maxcount;
    char starttime[31];
    int timeout;
    char pname[51];
}starg;

clogfile logfile;
connection conn; 
cpactive pactive;

bool _xmltoarg(const char *strxmlbuffer);

void _help();

void EXIT(int sig);

bool _migratetable();

bool instarttime();

int main(int argc,char *argv[])
{
    if (argc!=3) { _help(); return -1; }

    //closeioandsignal(true);
    signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

    if (logfile.open(argv[1])==false)
    {
        printf("打开日志文件失败（%s）。\n",argv[1]); return -1;
    }

    if (_xmltoarg(argv[2]) == false) return -1;

    if (instarttime()==false) return 0;

    pactive.addpinfo(starg.timeout,starg.pname);

    if (conn.connecttodb(starg.connstr,"Simplified Chinese_China.AL32UTF8",false) != 0)  
    {
        logfile.write("connect database(%s) failed.\n%s\n",starg.connstr,conn.message()); EXIT(-1);
    }

   _migratetable();

    return 0;
}

bool _migratetable()
{
    ctimer timer;

    sqlstatement stmtsel(&conn);
    //select sql--------------------
    stmtsel.prepare("select %s from %s %s ",starg.keycol,starg.tname,starg.where);

    char keycol[21];stmtsel.bindout(1,keycol,20);

    sqlstatement stmtdel(&conn);
    
    //delsql----------------
    string strdel = sformat(" delete from %s where %s in ( ",starg.tname,starg.keycol);
    for (int ii=1;ii<=starg.maxcount;ii++)
    {
        strdel = strdel + sformat(":%lu,",ii);
    }
    deleterchr(strdel,',');strdel = strdel + " )";

    stmtdel.prepare(strdel);

    char keyvalues[starg.maxcount][21];
    memset(keyvalues,0,sizeof(keyvalues));

    for (int ii =1;ii<=starg.maxcount;ii++)
    {
        stmtdel.bindin(ii,keyvalues[ii-1],20);
    }

    ctcols tcols;
    if (tcols.allcols(conn,starg.tname) == false) return false;
    //insertsql-------------------------------------
    string strins = " insert into ";
    strins = strins + starg.totname + " select " + tcols.m_allcols + " from " + starg.tname + " where " + starg.keycol + " in (";

    for (int ii=1;ii<=starg.maxcount;ii++)
    {
        strins = strins + sformat(":%lu,",ii);
    }
    deleterchr(strins,',');strins = strins + " )";

    sqlstatement stmtins(&conn);stmtins.prepare(strins);
    for (int ii =1;ii<=starg.maxcount;ii++)
    {
        stmtins.bindin(ii,keyvalues[ii-1],20);
    }

    //start-----------------------------
    if (stmtsel.execute() != 0)
    {
        logfile.write("stmtsel execute() failed\n%s\n%s\n",stmtsel.sql(),stmtsel.message());return false;
    }

    pactive.uptatime();
  
    int vcount = 0;

    while(true)
    {
        memset(keycol,0,sizeof(keycol));
        if (stmtsel.next() != 0) break;

        strcpy(keyvalues[vcount],keycol);

        vcount++;

        if (vcount == starg.maxcount) 
        {
            if (stmtins.execute() != 0)
            {
                logfile.write("stmtins execute() failed\n%s\n%s\n",stmtins.sql(),stmtins.message());return false;
            }

            if (stmtdel.execute() != 0)
            {
                logfile.write("stmtdel execute() failed\n%s\n%s\n",stmtdel.sql(),stmtdel.message());return false;
            }

            conn.commit();

            pactive.uptatime();

            vcount = 0;
            memset(keyvalues,0,sizeof(keyvalues));
        }

    }

    if (vcount > 0)
    {
        if (stmtins.execute() != 0)
        {
            logfile.write("stmtins execute() failed\n%s\n%s\n",stmtins.sql(),stmtins.message());return false;
        }

        if (stmtdel.execute() != 0)
        {
            logfile.write("stmtdel execute() failed\n%s\n%s\n",stmtdel.sql(),stmtdel.message());return false;
        }

        conn.commit();
    }

    if (stmtsel.rpc() > 0)
        logfile.write("表：%s migrate 表：%s,迁移记录数=%d，耗时=%.2f秒\n",
                    starg.tname,starg.totname,stmtsel.rpc(),timer.elapsed());
    
    return true;
}

bool instarttime()
{
    if (strlen(starg.starttime) == 0) return true;

    string strhh24 = ltime1("hh24");
    if (strstr(starg.starttime,strhh24.c_str()) == 0) return false;
    
    return true;
}
void _help()
{
    printf("\n");
    // printf("Sample: /project/tools/bin/migratetable /log/idc/migratetable_ZHOBTMIND1.log "\
    //                      "\"<connstr>idc/idcpwd@snorcl11g_131</connstr><tname>T_ZHOBTMIND1</tname>"\
    //                      "<totname>T_ZHOBTMIND1_HIS</totname><keycol>rowid</keycol><where>where ddatetime<sysdate-0.03</where>"\
    //                      "<maxcount>10</maxcount><starttime>22,23,00,01,02,03,04,05,06,13</starttime>"\
    //                      "<timeout>120</timeout><pname>migratetable_ZHOBTMIND1</pname>\"\n\n");
    printf("Sample: /project/tools/bin/migratetable /log/idc/migratetable_ZHOBTMIND1.log "\
                         "\"<connstr>idc/idcpwd@snorcl11g_131</connstr><tname>T_ZHOBTMIND1</tname>"\
                         "<totname>T_ZHOBTMIND1_HIS</totname><keycol>rowid</keycol><where>where ddatetime<sysdate-0.03</where>"\
                         "<maxcount>10</maxcount><starttime></starttime>"\
                         "<timeout>120</timeout><pname>migratetable_ZHOBTMIND1</pname>\"\n\n");
}

void EXIT(int sig)
{
    logfile.write("程序退出，sig=%d\n\n",sig);

    conn.disconnect();

    exit(0);
}

bool _xmltoarg(const char *strxmlbuffer)
{
    memset(&starg,0,sizeof(struct st_arg));

    getxmlbuffer(strxmlbuffer,"connstr",starg.connstr,100);
    if (strlen(starg.connstr)==0) { logfile.write("connstr is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"tname",starg.tname,30);
    if (strlen(starg.tname)==0) { logfile.write("tname is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"totname",starg.totname,30);
    if (strlen(starg.totname)==0) { logfile.write("totname is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"keycol",starg.keycol,30);
    if (strlen(starg.keycol)==0) { logfile.write("keycol is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"where",starg.where,1000);
    if (strlen(starg.where)==0) { logfile.write("where is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"starttime",starg.starttime,30);

    getxmlbuffer(strxmlbuffer,"maxcount",starg.maxcount);
    if (starg.maxcount==0) { logfile.write("maxcount is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"timeout",starg.timeout);
    if (starg.timeout==0) { logfile.write("timeout is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"pname",starg.pname,50);
    if (strlen(starg.pname)==0) { logfile.write("pname is null.\n"); return false; }

    return true;
}