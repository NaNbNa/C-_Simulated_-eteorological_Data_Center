#include "../_ftp.h"
using namespace idc;

int main(int argc,char* argv[])
{
    cftpclient ftp;

    if (ftp.login("192.168.240.131:21","test1","1") == false)
    {
        printf("ftp.login(192.168.240.131,test1/1 faile\n)");return -1;
    }

    if (ftp.mkdir("/project/tmp") == false)
    {
        printf("ftp.mkdir(/project/tmp) failed\n");
    }

    if (ftp.chdir("/project/tmp") == false) 
    {
        printf("ftp.chdir(/project/tmp) failed\n");
        return -1;
    }

    if (ftp.put("demo24.cpp","demo24.cpp") == true)
    {
        printf("put demo24.cpp ok\n");
    }
    else
        printf("put demo24.cpp failed\n");

    
}