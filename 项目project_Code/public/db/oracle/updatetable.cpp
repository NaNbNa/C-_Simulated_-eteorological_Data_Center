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

    // stmt.prepare("
    //         update girls set name = '冰冰',weight = 45.2,btime = to_date('2008-01-02 12:32:11','yyyy-mm-dd hh24:mi:ss') where id = 10 ");

    // if (stmt.execute() != 0)
    // {
    //     printf("execute(%s) failed\n%s\n",stmt.sql(),stmt.message());return -1;
    // }

     stmt.prepare("\
            update girls set name = :1,weight = :2,btime = to_date(:3,'yyyy-mm-dd hh24:mi:ss') where id = :4 ");

    
    struct st_girl
    {
        int id;
        char name[31];
        double weight;
        char btime[21];
    }stgirl;

    stmt.bindin(1,stgirl.name,30);stmt.bindin(2,stgirl.weight);stmt.bindin(3,stgirl.btime,20);
    stmt.bindin(4,stgirl.id);

    for (int ii =10;ii<15;ii++)
    {
        memset(&stgirl,0,sizeof(st_girl));

        stgirl.id = ii;
        sprintf(stgirl.name,"咪咪%05d号",ii);
        stgirl.weight = 48.1 + (ii%10)/10.0;
        sprintf(stgirl.btime,"2024-3-20 12:00:%02d",ii);

        if (stmt.execute() != 0)
        {
            printf("execute(%s) failed\n%s\n",stmt.sql(),stmt.message());return -1;
        }

         printf("成功修改了%ld条数据\n",stmt.rpc());
    }
    

    printf("成功修改了%ld条数据\n",stmt.rpc());

    conn.commit();

    return 0;
}