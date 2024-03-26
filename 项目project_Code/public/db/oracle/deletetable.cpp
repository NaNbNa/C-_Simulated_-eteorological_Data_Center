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

    // stmt.prepare("delete from girls where id = 10");

    // if (stmt.execute() != 0)
    // {
    //     printf("execute(%s) failed\n%s\n",stmt.sql(),stmt.message());return -1;
    // }

    struct st_girl
    {
        int id1;
        int id2;
    }stgirl;
    
    stmt.prepare("delete from girls where id >= :1 and id <=:2");
    stmt.bindin(1,stgirl.id1);
    stmt.bindin(2,stgirl.id2);
   
    memset(&stgirl,0,sizeof(st_girl));
    stgirl.id1= 11;stgirl.id2 = 13;

    if (stmt.execute() != 0)
    {
        printf("execute(%s) failed\n%s\n",stmt.sql(),stmt.message());return -1;
    }

    printf("成功删除了%ld条数据\n",stmt.rpc());

    conn.commit();

    return 0;
}