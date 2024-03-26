#include "_public.h"
#include "_ooci.h"

using namespace idc;

struct st_arg
{
    char connstr[101];
    char charset[51];
    char selectsql[1024];
    char fieldstr[501];
    char fieldlen[501];
    char bfilename[31];
    char efilename[31];
    char outpath[256];
    int maxcount;
    char starttime[52];
    char incfield[31];
    char connstr1[101];
    int timeout;
    char incfilename[256];
    char pname[51];
}starg;

clogfile logfile;
ccmdstr fieldname,fieldlen;
connection conn;
cpactive pactive;

long imaxincvalue;
int incfieldpos = -1;


void help();
void EXIT(int sig);
bool xmltoarg(const string& xmlbuffer);\
bool instarttime();
bool _dminingoracle();
bool readincfield();
bool writeincfield();

int main(int argc,char* argv[])
{
    if (argc != 3)
    {
        help();return -1;
    }

    //closeioandsignal(true);
    signal(2,EXIT);signal(15,EXIT);

    if (logfile.open(argv[1]) == false)
    {
        printf("打开日志失败 (%S)\n",argv[1]);EXIT(-1);
    }

    if (xmltoarg(argv[2]) == false)
    {
        logfile.write("xmltoarg(%s) failed\n",argv[2]);EXIT(-1);
    }

    if (instarttime() == false) return 0;

    pactive.addpinfo(starg.timeout,starg.pname);

    if (conn.connecttodb(starg.connstr,starg.charset) != 0)
    {
        logfile.write("connect databse(%s) failed\n",starg.connstr);EXIT(-1);
    }
    logfile.write("connect database(%s) ok\n",starg.connstr);

    pactive.uptatime();

    if (readincfield() == false) { logfile.write("readincfield() failed\n"); EXIT(-1); }

    _dminingoracle();

    return 0;
}

bool _dminingoracle()
{   
    sqlstatement stmt(&conn);
    stmt.prepare(starg.selectsql);

    int fieldnamelen = fieldname.size();
    string strfieldvalue[fieldnamelen];
    for (int ii=1;ii<=fieldnamelen;ii++)
    {   
        stmt.bindout(ii,strfieldvalue[ii-1],atoi(fieldlen[ii-1].c_str()));
    }
    
    if (strlen(starg.incfield) != 0) stmt.bindin(1,imaxincvalue);

    if (stmt.execute() != 0)
    {
        logfile.write("stmt.execute() failed\n%s\n%s\n",stmt.sql(),stmt.message());return false;
    }

    pactive.uptatime();

    cofile ofile; 
    string strxmlfilename;
    int iseq =1;
    while(true)
    {   
        if (stmt.next() != 0) break;

        if (ofile.isopen() == false)
        {
            
            sformat(strxmlfilename,"%s/%s_%s_%s_%d.xml",
                        starg.outpath,starg.bfilename,ltime1("yyyymmddhh24miss").c_str(),starg.efilename,iseq++);
            if (ofile.open(strxmlfilename) == false)
            {
                logfile.write("ofile.open(%s) failed\n",strxmlfilename.c_str());return false;
            }
            ofile.writeline("<data>\n");
        }   

        for (int ii=0;ii<fieldnamelen;ii++)
        {
            ofile.writeline("<%s>%s</%s>",fieldname[ii].c_str(),
                            strfieldvalue[ii].c_str(),fieldname[ii].c_str());
        }
        ofile.writeline("<endl/>\n");

        if ((starg.maxcount > 0) && ( (stmt.rpc()%starg.maxcount) == 0) )
        {
            ofile.writeline("</data>\n");
            if (ofile.closeandrename() == false)
            {
                logfile.write("ofile.closeandrename(%s) failed\n",strxmlfilename.c_str());return false;
            }

            logfile.write("生成文件%s(%d) \n",strxmlfilename.c_str(),starg.maxcount);    

            pactive.uptatime();
        }

        if((strlen(starg.incfield) != 0) && (imaxincvalue < atol(strfieldvalue[incfieldpos].c_str())))
        {
            imaxincvalue = atol(strfieldvalue[incfieldpos].c_str());
        }
    }

    if (ofile.isopen() == true) 
    {
        ofile.writeline("</data>\n");
        if (ofile.closeandrename() == false)
        {
            logfile.write("ofile.closeandrename() failed\n");return false;
        }

        if (starg.maxcount == 0)
            logfile.write("生成文件%s(%d) \n",strxmlfilename.c_str(),stmt.rpc());
        else
            logfile.write("生成文件%s(%d) \n",strxmlfilename.c_str(),stmt.rpc()%starg.maxcount);

    }

    if (stmt.rpc() > 0) writeincfield();

    return true;
}

bool readincfield()
{
    imaxincvalue = 0;
    if (strlen(starg.incfield) == 0) return true;

    for (int ii=0;ii<fieldname.size();ii++)
    {
        if (fieldname[ii] == starg.incfield) { incfieldpos = ii; break; }
    }

    if (incfieldpos == -1) 
    {
        logfile.write("递增字段名%s不在了列表中\n",starg.incfield);return false;
    }

    if (strlen(starg.connstr1) != 0)
    {
        connection conn1;
        if (conn1.connecttodb(starg.connstr1,starg.charset) != 0)
        {
            logfile.write("connstr1 database(%s) failed\n%s\n",starg.connstr1,conn.message());return false;
        }

        pactive.uptatime();

        sqlstatement stmt(&conn);
        stmt.prepare("select maxincvalue from T_MAXINCVALUE where pname = :1");
        stmt.bindin(1,starg.pname);
        stmt.bindout(1,imaxincvalue);
        stmt.execute();
        stmt.next();
    }
    else
    {
        cifile ifile;
        if(ifile.open(starg.incfilename) == false) return true;

        string strtemp;
        ifile.readline(strtemp);
        imaxincvalue = atoi(strtemp.c_str());
    }

    logfile.write("上次已经抽取数据的位置是 (%s=%d) \n",starg.incfield,imaxincvalue);

    return true;
}

bool writeincfield()
{
    if (strlen(starg.incfield) == 0) return true;

    if (strlen(starg.connstr1) != 0)
    {
        connection conn1;
        if (conn1.connecttodb(starg.connstr1,starg.charset) != 0)
        {
            logfile.write("conn1 database(%s) failed\n%s\n",starg.connstr1,conn.message());
        }

        pactive.uptatime();
        
        sqlstatement stmt(&conn1);
        stmt.prepare("update T_MAXINCVALUE set maxincvalue=:1 where pname = :2");
        stmt.bindin(1,imaxincvalue);stmt.bindin(2,starg.pname);

        if (stmt.execute() != 0)
        {
            if (stmt.rc() == 942)
            {
                conn1.execute("create table T_MAXINCVALUE(pname varchar2(50), maxincvalue number(15), primary key(pname) )");
                conn1.execute("insert into T_MAXINCVALUE values('%s',%ld)",starg.pname,imaxincvalue);
                conn1.commit();
                return true;
            }
            else
            {
                logfile.write("stmt.execute() failed\n%s\n%s\n",stmt.sql(),stmt.message()); return false;
            }
        }
        else
        {
            if (stmt.rpc() == 0)
            {
             conn1.execute("insert into T_MAXINCVALUE values('%s',%ld)",starg.pname,imaxincvalue);
            }

            conn1.commit();
        }
    }
    else
    {
        cofile ofile;
        if (ofile.open(starg.incfilename,false) == false)
        {
            logfile.write("ofile.open(%s) failed\n",starg.incfilename);
        }

        ofile.writeline("%ld",imaxincvalue);
    }

    return true;
}

bool instarttime()
{
    if (strlen(starg.starttime) != 0)
    {
        string strhh24 = ltime1("hh24");
        if (strstr(starg.starttime,strhh24.c_str()) == 0) return false;
    }
    return true;
}

bool xmltoarg(const string& xmlbuffer)
{
    memset(&starg,0,sizeof(st_arg));

    getxmlbuffer(xmlbuffer,"connstr",starg.connstr,100);
    if (strlen(starg.connstr) == 0) { logfile.write("connstr is null\n"); return false;}

    getxmlbuffer(xmlbuffer,"charset",starg.charset,50);
    if (strlen(starg.charset) == 0) { logfile.write("charset is null\n"); return false;}

    getxmlbuffer(xmlbuffer,"selectsql",starg.selectsql,1000);
    if (strlen(starg.selectsql) == 0) { logfile.write("selectsql is null\n"); return false; }

    getxmlbuffer(xmlbuffer,"fieldstr",starg.fieldstr,500);
    if (strlen(starg.selectsql) == 0) { logfile.write("selectsql is null\n"); return false; }

    getxmlbuffer(xmlbuffer,"fieldlen",starg.fieldlen,500);
    if (strlen(starg.fieldlen) == 0) { logfile.write("fieldlen is null\n"); return false; }

    getxmlbuffer(xmlbuffer,"bfilename",starg.bfilename,30);
    if (strlen(starg.bfilename) == 0) { logfile.write("bfilename is null\n"); return false; }

    getxmlbuffer(xmlbuffer,"efilename",starg.efilename,30);
    if (strlen(starg.efilename) == 0) { logfile.write("efilename is null\n"); return false; }

    getxmlbuffer(xmlbuffer,"outpath",starg.outpath,255);
    if (strlen(starg.outpath) == 0) { logfile.write("outpath is null\n"); return false; }

    getxmlbuffer(xmlbuffer,"maxcount",starg.maxcount);

    getxmlbuffer(xmlbuffer,"starttime",starg.starttime,50);

    getxmlbuffer(xmlbuffer,"incfield",starg.incfield,30);

    getxmlbuffer(xmlbuffer,"incfilename",starg.incfilename,255);

    getxmlbuffer(xmlbuffer,"connstr1",starg.connstr1,100);

    getxmlbuffer(xmlbuffer,"timeout",starg.timeout);
    if (starg.timeout == 0) { logfile.write("timeout is null\n"); return false;}

    getxmlbuffer(xmlbuffer,"pname",starg.pname,50);
    if (strlen(starg.pname) == 0) { logfile.write("pname is null\n"); return false; }

    
    fieldname.splittocmd(starg.fieldstr,",");
    
    fieldlen.splittocmd(starg.fieldlen,",");

    if (fieldlen.size() != fieldname.size())
    {
        logfile.write("fieldstr和fieldlen的元素个数不一致\n");return false;
    }

    if (strlen(starg.incfield) > 0)
    {
        if (strlen(starg.incfilename) == 0 && (strlen(starg.connstr1) == 0))
        {
            logfile.write("如果是增量抽取，incfilename和connstr1必二选一，不能都为空\n");return false;
        }
    }

    return true;
}


void help()
{
    printf ("\n");
    printf ("Using: /project/tools/bin/dminingoracle /log/idc/dminingoracle_ZHOBTCODE.log "\
            " \"<connstr>idc/idcpwd@snorcl11g_131</connstr><charset>Simplified Chinese_China.AL32UTF8</charset>"\
            "<selectsql>select obtid,cityname,provnname,lat,lon,height from T_ZHOBTCODE where obtid like '5%%%%'</selectsql>"\
            "<fieldstr>obtid,cityname,provnname,lat,lon,height</fieldstr><fieldlen>5,30,30,10,10,10</fieldlen> "\
            "<bfilename>ZHOBTCODE</bfilename><efilename>togxpt</efilename><outpath>/idcdata/dmindata</outpath>"\
            "<timeout>30</timeout><pname>dminingoracle_ZHOBTCODE</pname>\" \n\n"
            );
    printf ("Using: /project/tools/bin/dminingoracle /log/idc/dminingoracle_ZHOBTMIND.log "\
            " \"<connstr>idc/idcpwd@snorcl11g_131</connstr><charset>Simplified Chinese_China.AL32UTF8</charset>"\
            "<selectsql>select obtid,to_char(ddatetime,'yyyy-mm-dd hh24:mi:ss'),t,p,u,wd,wf,r,vis,keyid from T_ZHOBTMIND where keyid >:1 and obtid like '5%%%%'</selectsql>"\
            "<fieldstr>obtid,ddatetime,t,p,u,wd,wf,r,vis,keyid</fieldstr><fieldlen>5,19,8,8,8,8,8,8,8,15</fieldlen>"\
            "<bfilename>ZHOBTMIND</bfilename><efilename>togxpt</efilename><outpath>/idcdata/dmindata</outpath>"\
            "<starttime></starttime><incfield>keyid</incfield>"\
            "<incfilename>/idcdata/dmining/dminingoracle_ZHOBTMIND_togxpt.keyid</incfilename>"\
            "<timeout>30</timeout><pname>dminingoracle_ZHOBTMIND</pname>"\
            "<maxcount>1000</maxcount><connstr1>scott/a@snorcl11g_131</connstr1>\" \n\n"
            );
}
void EXIT(int sig)
{
    signal(2,SIG_IGN);signal(15,SIG_IGN);

    logfile.write("程序退出,sig=%d\n",sig);

    exit(0);
}

