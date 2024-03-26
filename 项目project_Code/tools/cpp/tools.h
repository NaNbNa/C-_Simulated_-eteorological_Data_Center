#ifndef _TOOLS_H
#define _TOOLS_H

#include "_public.h"
#include "_ooci.h"

using namespace idc; 
class ctcols
{
private:
    struct st_columns
    {   
        char colname[31];
        char datatype[31];
        int collen;
        int pkseq;
    };
public:
    ctcols();

    vector<struct st_columns> m_vallcols;
    vector<struct st_columns> m_vpkcols;
    
    string m_allcols;
    string m_pkcols;

    void initdata();

    bool allcols(connection& conn,char * tablename);

    bool pkcols(connection& conn,char * tablename);
};

#endif