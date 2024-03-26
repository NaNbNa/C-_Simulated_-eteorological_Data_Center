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

    //int c1 = 0;double c2 = 0;

    string s1,s2;
    //s1 = nullptr; s2 = nullptr;
    s1 = "";
    s2 = "";
    stmt.prepare("insert into tt values(:1,:2)");
    // stmt.bindin(1,c1,5);stmt.bindin(2,c2,5);
    stmt.bindin(1,s1,5);stmt.bindin(2,s2,5);
    printf("sql =%s=\n",stmt.sql());

    stmt.execute();

    conn.commit();

    return 0;
}