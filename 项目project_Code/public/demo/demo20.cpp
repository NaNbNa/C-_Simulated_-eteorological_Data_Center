#include "_public.h"

using namespace idc;

int main()
{
    clogfile logfile(1);

    if (logfile.open("/tmp/log/demo42.log",ios::app,true) == false)
    {
        cout << "open /tmp/log/demo42.log failed \n";return -1;
    }
    else
       cout << "open /tmp/log/demo42.log sucess \n";
    

    logfile.write("程序开始运行\n");

    for( int ii=1;ii<500000;ii++)
    {
        logfile.write("这是第%d个超级女生\n",ii);
    }

    logfile.write("程序运行结束\n");

}