#ifndef IDCAPP_H
#define IDCAPP_H

#include "_public.h"
#include "_ooci.h"
using namespace idc;

class CZHOBTMIND
{
private:
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
    };

    connection &m_conn;
    clogfile &m_logfile;

    sqlstatement m_stmt;

    string m_buffer;
    struct st_zhobtmind m_zhobtmind;
public:
    CZHOBTMIND(connection& conn,clogfile& logfile):m_conn(conn),m_logfile(logfile){}

    ~CZHOBTMIND(){}

    bool splitbuffer(const string& strline,const bool& bisxml);

    bool inserttable();
};


#endif