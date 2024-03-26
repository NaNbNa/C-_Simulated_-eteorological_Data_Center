#include "_public.h"

using namespace idc;

void EXIT(int sig)
{
    printf("sig=%d\n",sig);

    exit(0);
}

cpactive pactive;

int main(int argc,char* argv[]) //timeout, sleep(time)
{
    closeioandsignal(true);
    //signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

    string name = "demo02" + to_string(getpid());
    pactive.addpinfo(atoi(argv[1]),name);
    //pactive.addpinfo(30,"demo02");

    while(1)
    {
        //sleep(10);
        sleep(atoi(argv[2]));
        pactive.uptatime();
    }

    return 0;
}