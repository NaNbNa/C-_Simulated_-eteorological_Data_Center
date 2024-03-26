#include "tools.h"

struct st_arg
{
    char connstr[101];
    char charset[51];
    char inifilename[301];
    char xmlpath[301];
    char xmlpathbak[301];
    char xmlpatherr[301];
    int timetvl;
    int timeout;
    char pname[51];
}starg;
struct  st_xmltotable
{
    char filename[101];
    char tname[31];
    int uptbz;
    char execsql[301];
}stxmltotable;

struct st_columns
{   
    char colname[31];
    char datatype[31];
    int collen;
    int pkseq;
};

vector<struct st_xmltotable> vxmltotable;

clogfile logfile;
connection conn;
ctcols tcols;
string strinsertsql;
string strupdatesql;

vector<string> vcolvalue;
sqlstatement stmtins,stmtupt;

ctimer timer;
int totalcount,inscount,uptcount;

cpactive pactive;

void help(char* argv[]);

void EXIT(int sig);
bool xmltoarg(const char* xmlbuffer);

bool _xmltodb();

int _xmltodb(const  string& fullfilename,const string& filename);

bool loadxmltotable();

bool findxmltotable(const  string& filename);

void crtsql();

void preparesql();

bool execsql();

bool splitbuffer(const string& strbuffer);

bool xmltobakerr(const string& fullfilename,const string& srcpath,const string& dstpath);

int main(int argc,char* argv[])
{
    if (argc != 3)
    {
        help(argv);return -1;
    }
    signal(2,EXIT);signal(15,EXIT);

    if (logfile.open(argv[1]) == false)
    {
        printf("logfile.open(%s) failed\n",argv[1]);return -1;
    }

    if (xmltoarg(argv[2]) == false)
    {
        logfile.write("xmltoarg(%s) failed\n",argv[2]);
    }

    _xmltodb();

}

bool _xmltodb()
{
    int ret;
    cdir dir;

    int icout = 50;
    while(true)
    {
        if (icout > 30)
        {   
            if (loadxmltotable()  == false) return false;
            icout =0;
        }
        else
            icout ++;

        if (dir.opendir(starg.xmlpath,"*.XML",10000,false,true) == false)
        {
            logfile.write("dir.opendir(%s) failed\n",starg.xmlpath);return false;
        }

        pactive.addpinfo(starg.timeout,starg.pname);

        if (conn.connecttodb(starg.connstr,starg.charset) != 0)
        {
            logfile.write("connn database(%s) faile\n%s\n",starg.connstr,conn.message());return false;
        }

        logfile.write("conn database(%s) ok\n",starg.connstr);
        
        while(true)
        {
            if (dir.readdir() == false) break;

            logfile.write("处理文件%s...",dir.m_ffilename.c_str());

            ret = _xmltodb(dir.m_ffilename,dir.m_filename);

            pactive.uptatime();

            if (ret == 0) 
            {
                logfile << "ok..--" << stxmltotable.tname << "--记录数=" << totalcount << "（插入记录数=" << inscount
                    << "更新记录数=" << uptcount << "),耗时=" << timer.elapsed() << "秒\n";

                if (xmltobakerr(dir.m_ffilename,starg.xmlpath,starg.xmlpathbak) == false) return false;
            }
            
            if ( (ret == 1) || (ret == 3) || (ret == 4))
            {
                if (ret == 1) logfile << "failed 入库参数不正确\n" ;
                if (ret == 3) logfile << "failed 待入库的表" << stxmltotable.tname << "不存在\n";
                if (ret == 4) logfile << "failed 执行入库前的sql语句失败\n";

                if (xmltobakerr(dir.m_ffilename,starg.xmlpath,starg.xmlpatherr) == false) return false;
            }
            
            if (ret == 2)
            {
                logfile << "failed 数据库错误\n" ; return false;
            }

            if (ret == 5)
            {
                logfile << "failed 打开xml文件失败\n"; return false;
            }

        }   

        if (dir.size() == 0)
            sleep(starg.timetvl);

        pactive.uptatime();
    }

    return true;
}

int _xmltodb(const string& fullfilename,const string& filename)
{
    timer.start();
    totalcount = inscount = uptcount = 0;

    if (findxmltotable(filename) == false)  return 1;

    if (tcols.allcols(conn,stxmltotable.tname) == false)  return 2;
    if (tcols.pkcols(conn,stxmltotable.tname) == false) return 2;

    if (tcols.m_allcols.size() == 0) return 3;
    
    crtsql();

    preparesql();

    if (execsql() == false) return 4;

    cifile ifile;
    if (ifile.open(fullfilename) == false) { conn.rollback() ;return 5; }

    string strbuffer;
    while(true)
    {
        if (ifile.readline(strbuffer,"<endl/>") == false) break;

        totalcount++;

        splitbuffer(strbuffer);

        if (stmtins.execute() != 0)
        {
            if (stmtins.rc() == 1)
            {
                if (stxmltotable.uptbz == 1)
                {
                    if (stmtupt.execute() != 0 )
                    {
                        logfile.write("%s",strbuffer.c_str());
                        logfile.write("stmtupt.execute() failed\n%s\n%s\n",stmtupt.sql(),stmtupt.message());

                        if ( (stmtupt.rc() == 3113) || (stmtupt.rc() == 3114) || (stmtupt.rc() == 3115) || (stmtupt.rc() == 16014)) return 2;
         
                    }
                    else
                        uptcount++;
                }
                
            }
            else
            {
                logfile.write("%s",strbuffer.c_str());
                logfile.write("stmtupt.execute() failed\n%s\n%s\n",stmtins.sql(),stmtins.message());

                if ( (stmtins.rc() == 3113) || (stmtins.rc() == 3114) || (stmtins.rc() == 3115) || (stmtins.rc() == 16014)) return 2;
            }
        }
        else
            inscount++;
    }

    conn.commit();

    return 0;
}

void crtsql()
{   
    string strinsertp1;
    string strinsertp2;
    int colseq =1;

    for (auto& aa: tcols.m_vallcols)
    {
        if (strcmp(aa.colname,"uptime") == 0) continue;

        strinsertp1 = strinsertp1 + aa.colname + ",";

        if (strcmp(aa.colname,"keyid") == 0)
        {
            strinsertp2 = strinsertp2 + sformat("SEQ_%s.nextval",stxmltotable.tname+2) + ",";
        }
        else 
        {
            if (strcmp(aa.datatype,"date") == 0)
            {
                strinsertp2 = strinsertp2 + sformat("to_date(:%d,'yyyymmddhh24miss')",colseq) +  ",";
            }
            else
                strinsertp2 = strinsertp2 + sformat(":%d",colseq) + ",";

            colseq ++ ;
        }

    }
    
    deleterchr(strinsertp1,',');    deleterchr(strinsertp2,',');

    sformat(strinsertsql,"insert into %s(%s) values(%s)",stxmltotable.tname,strinsertp1.c_str(),strinsertp2.c_str());

    //logfile.write("insertsql = %s\n",strinsertsql.c_str());

    if (stxmltotable.uptbz != 1) return;
    
    strupdatesql = sformat("update %s set ",stxmltotable.tname);

    colseq =1 ;
    for(auto& aa: tcols.m_vallcols)
    {
        if (aa.pkseq != 0) continue;

        if (strcmp(aa.colname,"keyid") == 0) continue;

        if (strcmp(aa.colname,"uptime") == 0)
        {
            strupdatesql =  strupdatesql + "uptime = sysdate";continue;
        }

        if (strcmp(aa.datatype,"date") != 0)
        {
            strupdatesql = strupdatesql + sformat("%s=:%d,",aa.colname,colseq);
        }
        else
        {
            strupdatesql = strupdatesql + sformat("%s=to_date(:%d,'yyyymmddhh24miss'),",aa.colname,colseq);
        }
        colseq ++ ;
    }

    deleterchr(strupdatesql,',');

    strupdatesql = strupdatesql + " where 1=1 ";

    for (auto& aa:tcols.m_vallcols)
    {
        if (aa.pkseq == 0) continue;

        if (strcmp(aa.datatype,"date") != 0)
        {
            strupdatesql = strupdatesql + sformat(" and %s=:%d ",aa.colname,colseq);
        }
        else 
            strupdatesql = strupdatesql + sformat(" and %s=to_date(:%d,'yyyymmddhh24miss') ",aa.colname,colseq);

        colseq ++;
    }

    //logfile.write("updatesql= %s\n",strupdatesql.c_str());

    return;
}

void preparesql()
{
    vcolvalue.resize(tcols.m_vallcols.size());

    stmtins.connect(&conn);
    stmtins.prepare(strinsertsql);

    // logfile.write("stmtinssql = %s\n",stmtins.sql());
    int colseq = 1;
    for (int ii=0;ii<tcols.m_vallcols.size();ii++)
    {   
        if ((strcmp(tcols.m_vallcols[ii].colname,"uptime") == 0) ||
            (strcmp(tcols.m_vallcols[ii].colname,"keyid") == 0)) continue;

        stmtins.bindin(colseq,vcolvalue[ii],tcols.m_vallcols[ii].collen);
        // logfile.write("stmtins.bindin(%d,vcolvalue[%d],%d)name=%s\n",colseq,ii,tcols.m_vallcols[ii].collen,tcols.m_vallcols[ii].colname);
        colseq ++ ;
    }
    
    if (stxmltotable.uptbz != 1) return;

    stmtupt.connect(&conn);
    stmtupt.prepare(strupdatesql);

    colseq =1;

    // logfile.write("stmtuptsql = %s\n",stmtupt.sql());
    for (int ii=0;ii<tcols.m_vallcols.size();ii++)
    {
        if (tcols.m_vallcols[ii].pkseq != 0) continue;

        if ((strcmp(tcols.m_vallcols[ii].colname,"uptime") == 0 )||
            (strcmp(tcols.m_vallcols[ii].colname,"keyid") == 0) ) continue;
        
        stmtupt.bindin(colseq,vcolvalue[ii],tcols.m_vallcols[ii].collen);
        // logfile.write("stmtupt.bindin(%d,vcolvalue[%d],%d)name=%s\n",colseq,ii,tcols.m_vallcols[ii].collen,tcols.m_vallcols[ii].colname);

        colseq ++ ;
    }

    for (int ii=0;ii<tcols.m_vallcols.size();ii++)
    {
        if (tcols.m_vallcols[ii].pkseq == 0) continue;
        
        stmtupt.bindin(colseq,vcolvalue[ii],tcols.m_vallcols[ii].collen);
        //logfile.write("stmtupt.bindin(%d,vcolvalue[%d],%d)name=%s\n",colseq,ii,tcols.m_vallcols[ii].collen,tcols.m_vallcols[ii].colname);

        colseq ++ ;
    }

    return;
}

bool execsql() 
{
    if (strlen(stxmltotable.execsql) == 0) return true;

    sqlstatement stmt(&conn);
    stmt.prepare(stxmltotable.execsql);
    if (stmt.execute() != 0)
    {
        logfile.write("stmt.execute() failed\n%s\n%s\n",stmt.sql(),stmt.message());return false;
    }

    return true;
}

bool loadxmltotable()
{
    vxmltotable.clear();

    cifile ifile;
    if (ifile.open(starg.inifilename) == false)
    {
        logfile.write("ifile.open(%s) failed\n",starg.inifilename);return false;
    }

    string strbuffer;

    while(true)
    {
        if (ifile.readline(strbuffer,"<endl/>") == false) break;

        memset(&stxmltotable,0,sizeof(st_xmltotable));

        getxmlbuffer(strbuffer,"filename",stxmltotable.filename,100);
        getxmlbuffer(strbuffer,"tname",stxmltotable.tname,30);
        getxmlbuffer(strbuffer,"uptbz",stxmltotable.uptbz);
        getxmlbuffer(strbuffer,"execsql",stxmltotable.execsql,300);

        vxmltotable.push_back(stxmltotable);
    }

    logfile.write("loadxmltotable(%s) ok\n",starg.inifilename);

    return true;
}

bool findxmltotable(const string& xmlfilename)
{
    for (auto& aa:vxmltotable)
    {
        if (matchstr(xmlfilename,aa.filename) == true)
        {
            stxmltotable = aa;
            return true;
        }
    }

    return false;
}

bool xmltoarg(const char* strxmlbuffer)
{
    memset(&starg,0,sizeof(struct st_arg));

    getxmlbuffer(strxmlbuffer,"connstr",starg.connstr,100);
    if (strlen(starg.connstr)==0) { logfile.write("connstr is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"charset",starg.charset,50);
    if (strlen(starg.charset)==0) { logfile.write("charset is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"inifilename",starg.inifilename,300);
    if (strlen(starg.inifilename)==0) { logfile.write("inifilename is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"xmlpath",starg.xmlpath,300);
    if (strlen(starg.xmlpath)==0) { logfile.write("xmlpath is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"xmlpathbak",starg.xmlpathbak,300);
    if (strlen(starg.xmlpathbak)==0) { logfile.write("xmlpathbak is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"xmlpatherr",starg.xmlpatherr,300);
    if (strlen(starg.xmlpatherr)==0) { logfile.write("xmlpatherr is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"timetvl",starg.timetvl);
    if (starg.timetvl< 2) starg.timetvl=2;   
    if (starg.timetvl>30) starg.timetvl=30;

    getxmlbuffer(strxmlbuffer,"timeout",starg.timeout);
    if (starg.timeout==0) { logfile.write("timeout is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"pname",starg.pname,50);
    if (strlen(starg.pname)==0) { logfile.write("pname is null.\n"); return false; }

    return true;
}

bool splitbuffer(const string& strbuffer)
{
    string strtemp;
    for (int ii =0;ii<tcols.m_vallcols.size();ii++)
    {
        getxmlbuffer(strbuffer,tcols.m_vallcols[ii].colname,strtemp,tcols.m_vallcols[ii].collen);

        if (strcmp(tcols.m_vallcols[ii].datatype,"date") == 0)
        {
            picknumber(strtemp,strtemp,false,false);
        }

        if (strcmp(tcols.m_vallcols[ii].datatype,"number") == 0)
        {
            picknumber(strtemp,strtemp,true,true);
        }

        vcolvalue[ii] = strtemp.c_str();
    }

    return true;
}

bool xmltobakerr(const string& fullfilename,const string& srcpath,const string& dstpath)
{
    string dstfilename = fullfilename;

    replacestr(dstfilename,srcpath,dstpath,false);

    if (renamefile(fullfilename,dstfilename) == false)
    {
        logfile.write("renamefile(%s,%s) failed\n",fullfilename.c_str(),dstfilename.c_str());return false;
    }

    return true;
}

void help(char* argv[])
{
    printf("\n");
    printf("Sample: /project/tools/bin/xmltodb /log/idc/xmltodb_vip.log "\
              "\"<connstr>idc/idcpwd@snorcl11g_131</connstr><charset>Simplified Chinese_China.AL32UTF8</charset>"\
              "<inifilename>/project/idc/ini/xmltodb.xml</inifilename>"\
              "<xmlpath>/idcdata/xmltodb/vip</xmlpath><xmlpathbak>/idcdata/xmltodb/vipbak</xmlpathbak>"\
              "<xmlpatherr>/idcdata/xmltodb/viperr</xmlpatherr>"\
              "<timetvl>5</timetvl><timeout>50</timeout><pname>xmltodb_vip</pname>\"\n\n");
}

void EXIT(int sig)
{
    signal(2,SIG_IGN);signal(15,SIG_IGN);

    printf("程序退出，sig=%d\n",sig);
    exit(0);
}