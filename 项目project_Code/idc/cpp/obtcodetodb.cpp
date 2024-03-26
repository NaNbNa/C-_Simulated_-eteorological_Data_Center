#include "_public.h"
#include "_ooci.h"

using namespace idc;

struct st_arg
{
    char obtid[11];
    char cityname[31];
    char provnname[31];
    char lat[11];
    char lon[11];
    char height[11];
};

connection conn;
clogfile logfile;
list<struct st_arg> arglist;
cpactive pactive;

void help();
void EXIT(int sig);

bool loadtolist(const string& loadfilename);
int main(int argc,char* argv[])
{
    if (argc != 5)
    {
        help();return -1;
    }

    
    closeioandsignal(true);
    signal(2,EXIT); signal(15,EXIT);

    pactive.addpinfo(10,"obtcodetodb");

    if (logfile.open(argv[4]) == false)
    {
        printf("logfile.open(%s) failed\n",argv[4]);return -1;
    }


    if (loadtolist(argv[1]) == false)
    {
        logfile.write("loadtolist(%s) failed\n%s",argv[1],conn.message()); EXIT(-1);
    }

    
    if (conn.connecttodb(argv[2],argv[3]) != 0)
    {
        logfile.write("conn.connecttodb(%s,%s) failed\n",argv[2],argv[3]);return -1;
    }
    logfile.write("connect database(%s,%s) sucess \n",argv[2],argv[3]);

    sqlstatement stmt;
    stmt.connect(&conn);
    stmt.prepare("\
        insert into T_ZHOBTCODE(obtid,cityname,provnname,lat,lon,height,keyid) \
                        values(:1,:2,:3,:4*100,:5*100,:6*10,SEQ_ZHOBTCODE.nextval)");
    
    struct st_arg starg;
    stmt.bindin(1,starg.obtid,10);stmt.bindin(2,starg.cityname,30);stmt.bindin(3,starg.provnname,30);
    stmt.bindin(4,starg.lat,10);stmt.bindin(5,starg.lon,10);stmt.bindin(6,starg.height,10);

    sqlstatement stmtup;
    stmtup.connect(&conn);
    stmtup.prepare("update T_ZHOBTCODE set cityname=:1, provnname=:2,lat=:3,lon=:4,height=:5,uptime=sysdate \
                    where obtid = :6");
    stmtup.bindin(1,starg.cityname,30);stmtup.bindin(2,starg.provnname,30);stmtup.bindin(6,starg.obtid,10);
    stmtup.bindin(3,starg.lat,10);stmtup.bindin(4,starg.lon,10);stmtup.bindin(5,starg.height,10);

    int starglen = sizeof(st_arg);

    int inscount =0,uptcount =0;
    ctimer timer;
    for (auto& stcode:arglist)
    {   
        memset(&starg,0,starglen);
        starg = stcode;
        
        if (stmt.execute() != 0)
        {
            if (stmt.rc() == 1)
            {
                if (stmtup.execute() != 0)
                {
                    logfile.write("stmtup.execute() failed\n%s\%s\n",stmtup.sql(),stmtup.message());EXIT(-1);
                }
                else 
                    uptcount++;
            }
            else
            {
                logfile.write("stmt.execute() failed\n%s\%s\n",stmt.sql(),stmt.message());EXIT(-1);
            }
        }
        else 
            inscount++;
    }

    logfile.write("总记录数=%d，插入=%d，更新=%d，耗时=%.2f秒\n",arglist.size(),inscount,uptcount,timer.elapsed());

    conn.commit();
    return 0;
}

void help()
{
    printf("\n");

    printf("Using: /project/tools/bin/procctl 10 /project/idc/bin/obtcodetodb " \
            "/project/idc/ini/stcode.ini  \"idc/idcpwd@snorcl11g_131\"  \"Simplified Chinese_China.AL32UTF8\" "\
            "/log/idc/obtcodetodb.log\n\n"\
            );
}

void EXIT(int sig)
{
    signal(2,SIG_IGN);signal(15,SIG_IGN);
    logfile.write("程序退出, sig=%d\n",sig);

    //conn.rollback();conn.disconnect();
    exit(0);
}
bool loadtolist(const string& filename)
{
    cifile ifile;
    if (ifile.open(filename,ios::in) == false)
    {
        logfile.write("ifile.open(%s,ios::in) failed\n",filename.c_str());
    }

    struct st_arg starg;
    string buffer;
    ccmdstr cmdstr;

    while(true)
    {
        if (ifile.readline(buffer) == false) break;
        
        cmdstr.splittocmd(buffer,",");
        if (cmdstr.size() != 6) continue;;

        memset(&starg,0,sizeof(st_arg));
        cmdstr.getvalue(0,starg.provnname,30);
        cmdstr.getvalue(1,starg.obtid,5);
        cmdstr.getvalue(2,starg.cityname,30);
        cmdstr.getvalue(3,starg.lat,10);
        cmdstr.getvalue(4,starg.lon,10);
        cmdstr.getvalue(5,starg.height,10);

        arglist.push_back(std::move(starg));
    }
    
    return true;
}
