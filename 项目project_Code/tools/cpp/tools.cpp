#include "tools.h"

ctcols::ctcols()
{
    initdata();
}

void ctcols::initdata()
{
    m_vallcols.clear();
    m_vpkcols.clear();
    m_allcols.clear();
    m_pkcols.clear();
}


bool ctcols::allcols(connection& conn,char * tablename)
{
        m_vallcols.clear();
    m_allcols.clear();

    struct st_columns stcolumns;
    
    sqlstatement stmt(&conn);
    stmt.prepare("\
        select lower(column_name),lower(data_type),data_length from User_TAB_COLUMNS \
        where table_name = upper(:1) order by column_id");
    stmt.bindin(1,tablename,30);stmt.bindout(1,stcolumns.colname,30);
    stmt.bindout(2,stcolumns.datatype,30);stmt.bindout(3,stcolumns.collen);

    if (stmt.execute() != 0) return false;

    while(true)
    {
        memset(&stcolumns,0,sizeof(struct st_columns));

        if (stmt.next() != 0) break;

        if (strcmp(stcolumns.datatype,"char") == 0)         strcpy(stcolumns.datatype,"char");
        if (strcmp(stcolumns.datatype,"nchar") == 0)        strcpy(stcolumns.datatype,"char");
        if (strcmp(stcolumns.datatype,"varchar2") == 0)     strcpy(stcolumns.datatype,"char");
        if (strcmp(stcolumns.datatype,"nvarchar2") == 0)    strcpy(stcolumns.datatype,"char");
        if (strcmp(stcolumns.datatype,"rowid") == 0)     {  strcpy(stcolumns.datatype,"char"); stcolumns.collen = 18; }

        if (strcmp(stcolumns.datatype,"date") == 0)      {stcolumns.collen = 14;}

        if (strcmp(stcolumns.datatype,"number") == 0)          strcpy(stcolumns.datatype,"number");
        if (strcmp(stcolumns.datatype,"integer") == 0)         strcpy(stcolumns.datatype,"number");
        if (strcmp(stcolumns.datatype,"float") == 0)           strcpy(stcolumns.datatype,"number");

        if ((strcmp(stcolumns.datatype,"number") != 0) &&
            (strcmp(stcolumns.datatype,"char") != 0)    &&
            (strcmp(stcolumns.datatype,"date") != 0))   continue;

        if (strcmp(stcolumns.datatype,"number") == 0) stcolumns.collen =  22;

        m_allcols = m_allcols + stcolumns.colname + ",";

        m_vallcols.push_back(stcolumns);
    }

    if (stmt.rpc() > 0) deleterchr(m_allcols,',');

    return true;
}

bool ctcols::pkcols(connection& conn,char * tablename)
{
    m_pkcols.clear();
    m_vpkcols.clear();

    struct st_columns stcolumns;

    sqlstatement stmt(&conn);
    stmt.prepare("select lower(column_name),position from USER_CONS_COLUMNS\
                where table_name  = upper(:1) \
                and constraint_name = (select constraint_name from USER_CONSTRAINTS \
                    where table_name = upper(:2) and constraint_type = 'P' and generated = 'USER NAME') \
                order by position");
    stmt.bindin(1,tablename,30);stmt.bindin(2,tablename,30);
    stmt.bindout(1,stcolumns.colname,30);stmt.bindout(2,stcolumns.pkseq);

    if (stmt.execute() != 0) return false;

    while(true)
    {
        memset(&stcolumns,0,sizeof(struct st_columns));

        if (stmt.next() != 0) break;

        m_pkcols = m_pkcols + stcolumns.colname + ",";
        m_vpkcols.push_back(stcolumns);
    }

    if (stmt.rpc() > 0) deleterchr(m_pkcols,',');

    for(auto& aa:m_vpkcols)
    {
        for (auto&bb:m_vallcols)
        {
            if (strcmp(aa.colname,bb.colname) == 0)
            {
                bb.pkseq = aa.pkseq;break;
            }
        } 
    }

    return true;
}