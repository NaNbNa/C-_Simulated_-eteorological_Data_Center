#include "idcapp.h"

bool CZHOBTMIND::splitbuffer(const string& strline,const bool& bisxml)
{
    memset(&m_zhobtmind,0,sizeof(st_zhobtmind));
    if (bisxml == true)
    {
        getxmlbuffer(strline,"obtid",m_zhobtmind.obtid,5);
        getxmlbuffer(strline,"ddatetime",m_zhobtmind.ddatetime,14);

        char temp[11];
        getxmlbuffer(strline,"t",temp,10);   if (strlen(temp) > 0) snprintf(m_zhobtmind.t,10,"%d",(int)(atof(temp)*10));
        getxmlbuffer(strline,"p",temp,10);   if (strlen(temp) > 0) snprintf(m_zhobtmind.p,10,"%d",(int)(atof(temp)*10));
        getxmlbuffer(strline,"u",temp,10);
        getxmlbuffer(strline,"wd",temp,10);
        getxmlbuffer(strline,"wf",temp,10);  if (strlen(temp) > 0) snprintf(m_zhobtmind.wf,10,"%d",(int)(atof(temp)*10));
        getxmlbuffer(strline,"r",temp,10);   if (strlen(temp) > 0) snprintf(m_zhobtmind.r,10,"%d",(int)(atof(temp)*10));
        getxmlbuffer(strline,"vis",temp,10); if (strlen(temp) > 0) snprintf(m_zhobtmind.vis,10,"%d",(int)(atof(temp)*10));
    }
    else
    {
        ccmdstr cmdstr;cmdstr.splittocmd(strline,",");
        cmdstr.getvalue(0,m_zhobtmind.obtid,5);
        cmdstr.getvalue(1,m_zhobtmind.ddatetime,14);

        char temp[11];
        cmdstr.getvalue(2,temp,10); if (strlen(temp) > 0) snprintf(m_zhobtmind.t,10,"%d",(int)(atof(temp)*10));
        cmdstr.getvalue(3,temp,10); if (strlen(temp) > 0) snprintf(m_zhobtmind.p,10,"%d",(int)(atof(temp)*10));
        cmdstr.getvalue(4,m_zhobtmind.u,10);
        cmdstr.getvalue(5,m_zhobtmind.wd,10);
        cmdstr.getvalue(6,temp,10); if (strlen(temp) > 0) snprintf(m_zhobtmind.wf,10,"%d",(int)(atof(temp)*10));
        cmdstr.getvalue(7,temp,10); if (strlen(temp) > 0) snprintf(m_zhobtmind.r,10,"%d",(int)(atof(temp)*10));
        cmdstr.getvalue(8,temp,10); if (strlen(temp) > 0) snprintf(m_zhobtmind.vis,10,"%d",(int)(atof(temp)*10));
    }

    m_buffer = strline;

    return true;
}

bool CZHOBTMIND::inserttable()
{
    if (m_stmt.isopen() == false)
    {
        m_stmt.connect(&m_conn);
        m_stmt.prepare("insert into T_ZHOBTMIND(obtid,ddatetime,t,p,u,wd,wf,r,vis,keyid) \
                        values(:1,to_date(:2,'yyyy-mm-dd hh24:mi:ss'),:3,:4,:5,:6,:7,:8,:9,SEQ_ZHOBTMIND.nextval)");

        m_stmt.bindin(1,m_zhobtmind.obtid,5);m_stmt.bindin(2,m_zhobtmind.ddatetime,14);
        m_stmt.bindin(3,m_zhobtmind.t,10);m_stmt.bindin(4,m_zhobtmind.p,10);
        m_stmt.bindin(5,m_zhobtmind.u,10);m_stmt.bindin(6,m_zhobtmind.wd,10);
        m_stmt.bindin(7,m_zhobtmind.wf,10);m_stmt.bindin(8,m_zhobtmind.r,10);
        m_stmt.bindin(9,m_zhobtmind.vis,10);
    }

    if (m_stmt.execute() != 0)
    {
        if (m_stmt.rc() != 1) //一行出错数据错误，不要退出，继续处理下一行
        {
            m_logfile.write("strbuffer=%s\n",m_buffer.c_str());
            m_logfile.write("m_stmt.execute() failed\n%s\n%s\n",m_stmt.sql(),m_stmt.message());
        }

        return false;
    }
    return true;
}
