#include "../_ftp.h"
using namespace idc;

int main(int argc,char* argv[])
{
    cftpclient ftp;

    if (ftp.login("192.168.240.131:21","test1","1") == false)
    {
        printf("ftp.login(192.168.240.131,test1/1 faile\n)");return -1;
    }


    if (ftp.chdir("/project/tmp") == false) 
    {
        printf("ftp.chdir(/project/tmp) failed\n");
        return -1;
    }

    if (ftp.get("/project/tmp/demo24.cpp","/tmp/test1/demo24.cpp") == true)
    {
        printf("get demo24.cpp ok\n");
    }
    else
        printf("get demo24.cpp failed\n");

}