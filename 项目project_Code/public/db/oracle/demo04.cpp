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

    struct st_girl
    {
        long id;
        char name[31];
        double weight;
        char btime[21];
        char memo[301];
    }stgirl;

    stmt.prepare("\
    begin\
    delete from girls where id =:1;\
    insert into girls(id,name) values(:2,:3);\
    update girls set weight = :4 where id =:5;\
    end;");

    stmt.bindin(1,stgirl.id);
    stmt.bindin(2,stgirl.id);
    stmt.bindin(3,stgirl.name,30);
    stmt.bindin(4,stgirl.weight);
    stmt.bindin(5,stgirl.id);

    memset(&stgirl,0,sizeof(st_girl));
    stgirl.id = 10;
    strcpy(stgirl.name,"冰冰10");
    stgirl.weight = 49.5;

    printf("sql =%s=\n",stmt.sql());
    if (stmt.execute() != 0)
    {
        printf ("execute() failed\n%s\n",stmt.message());
    }

    printf("影响了%ld条记录\n",stmt.rpc());
    conn.commit();

    return 0;
}