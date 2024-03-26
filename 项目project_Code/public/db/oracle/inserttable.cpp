#include "_ooci.h"

using namespace idc;

int main(int argc,char* argv[])
{
    connection conn;

    if (conn.connecttodb("girl/a@snorcl11g_128","Simplified Chinese_China.AL32UTF8") != 0)
    {
        printf("connect database failed \n%d %s\n",conn.rc(),conn.message());return -1;
    }

    printf("connect database ok\n");


    sqlstatement stmt;
    stmt.connect(&conn);

    // stmt.prepare(" insert into girls(id,name,weight,btime,memo)
    //                             values(1,'西施',48.5,to_date('2000-01-01 12:30:35','yyyy-mm-dd hh24:mi:ss'),'中国排名第一美少女')
    // ");

    // if (stmt.execute() !=0)
    // {
    //     printf("stmt.execute() failed\n %s\n%s\n",stmt.sql(),stmt.message());return -1;
    // }

    // printf("insert girls %ld条记录 ok\n",stmt.rpc());
    struct st_girl{
    long id;char name[31];double weight;char btime[21];char memo[301];
    }stgirl;

    stmt.prepare("insert into girls(id,name,weight,btime,memo) \
                    values(:1,:2,:3,to_date(:4,'yyyy-mm-dd hh24:mi:ss'),:5)");
    
    stmt.bindin(1,stgirl.id);stmt.bindin(2,stgirl.name,30),stmt.bindin(3,stgirl.weight);
    stmt.bindin(4,stgirl.btime,20);stmt.bindin(5,stgirl.memo,300);

    for (int ii=10;ii<15;ii++)
    {
        stgirl.id =0; memset(stgirl.name,0,sizeof(stgirl.name));stgirl.weight = 0;memset(stgirl.btime,0,sizeof(stgirl.btime));
        memset(stgirl.memo,0,sizeof(stgirl.memo));

        stgirl.id = ii;
        sprintf(stgirl.name,"西施%05dgirl",ii);
        stgirl.weight = 45.35 + ii;
        sprintf(stgirl.btime,"2021-08-25 10:33:%02d",ii);
        sprintf(stgirl.memo,"这是第%d个女生的备注",ii);

        if (stmt.execute() != 0)
        {
            printf("stmt.execute() failed\n %s\n%s\n",stmt.sql(),stmt.message()); return -1;
        }

        printf ("成功插入%ld条记录\n",stmt.rpc());
    }

    conn.commit();

    return 0;
}