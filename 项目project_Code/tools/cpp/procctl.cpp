#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc,char* argv[])     //调度模块
{
    if (argc < 3)
    {
        printf("Using:   ./procctl timetvl program argv ...\n");
        printf("Example: /project/tools/bin/procctl 10 /usr/bin/tar zcvf /tmp/tmp.tgz /usr/include \n");
        printf("Example: /project/tools/bin/procctl 60 /project/idc/bin/crtsurfdata /project/idc/ini/stcode.ini /tmp/idc/surdata /log/idc/crtsurfdata.log csv,xml,json\n\n");

        printf("timetvl: 程序重启 间隔的秒数\n");
        return -1;
    }

    for( int ii=0;ii<64;ii++)
    {
        signal(ii,SIG_IGN);close(ii);
    }

    if (fork() !=0) exit(0);    //父进程退出，使得1进程管理子进程，不受shell控制

    signal(SIGCHLD,SIG_DFL);    //使得父进程可捕捉信号

    char *pargv[argc];
    for(int ii=2;ii<argc;ii++)
        pargv[ii-2] = argv[ii];
    pargv[argc - 2] = nullptr;

    while(true)
    {
        if (fork() == 0)
        {
            execv(argv[2],pargv);
            exit(0);
        }
        else
        {
            //父进程等待子进程结束
            // int status;
            // wait(&status);
            wait(nullptr);
            sleep(atoi(argv[1]));   //休眠timetvl秒，然后回到循环
        }
    }
}