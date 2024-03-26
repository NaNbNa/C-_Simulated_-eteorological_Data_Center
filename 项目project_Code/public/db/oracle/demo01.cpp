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

    string username,passwd;
    int ccount = 0;

    username = "test1";
    passwd = "abc";

    stmt.prepare("select count(*) from operator where username='%s' and passwd='%s'",username.c_str(),passwd.c_str());
    stmt.bindout(1,ccount);

    printf ("sql= %s\n",stmt.sql());
    if (stmt.execute() != 0)
    {
        printf("execute() failed\n%s\n",stmt.message());return -1;
    }

    stmt.next();

    if (ccount == 0)
    {
        printf("登录失败\n");
    }
    else 
        printf("登录成功\n");

    return 0;
}