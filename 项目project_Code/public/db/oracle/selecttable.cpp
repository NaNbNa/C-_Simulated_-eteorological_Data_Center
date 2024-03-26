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

    stmt.prepare("select id,name,weight,to_char(btime,'yyyy-mm-dd hh24:mi:ss'),memo from girls where id >=:1 and id <=:2");
    int min,max;
    stmt.bindin(1,min);stmt.bindin(2,max);

    

    struct st_girl
    {
        int id;
        char name[31];
        double weight;
        char btime[21];
        char memo[301];
    }stgirl;

    
    stmt.bindout(1,stgirl.id);
    stmt.bindout(2,stgirl.name,30);
    stmt.bindout(3,stgirl.weight);
    stmt.bindout(4,stgirl.btime,20);
    stmt.bindout(5,stgirl.memo,300);

    min = 1;max = 15;
    if (stmt.execute() != 0)
    {
        printf("execute() faile\n%s\n%s\n",stmt.sql(),stmt.message());
    }
    
    while(true)
    {
        memset(&stgirl,0,sizeof(stgirl));

        if (stmt.next() != 0) break;

        printf("id=%d,name=%s,weight=%.2f,btime=%s,memo=%s\n",stgirl.id,stgirl.name,stgirl.weight,stgirl.btime,stgirl.memo);
    }

    printf("查询了%ld条记录\n",stmt.rpc());

    return 0;
}