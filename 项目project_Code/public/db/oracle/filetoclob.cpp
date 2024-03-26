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

    stmt.prepare("insert into  girls(id,name,memo1) values(21,'冰冰',empty_clob())");
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed\n%s\n%s\n",stmt.sql(),stmt.message());return -1;
    }

    stmt.prepare("select memo1 from girls where id = 21 for update");
    stmt.bindclob();
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed\n%s\n%s\n",stmt.sql(),stmt.message());return -1;
    }

    if (stmt.next() !=0 ) return 0;

    if (stmt.filetolob("/root/train/public/db/oracle/memo1_in.txt") != 0)
    {
        printf("filetolob() failed\n%s\n",stmt.message());return -1;
    }

    printf("load file to clob ok\n");

    conn.commit();
    return 0;
 
}