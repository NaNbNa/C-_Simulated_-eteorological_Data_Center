#include "_public.h"

using namespace idc;

int main(int argc,char* argv[])
{
    if (argc != 2)
    {
        printf("\n");
        printf("Example: ./checkproc /project/tools/bin/procctl 10 "  
                "/project/tools/bin/checkproc /tmp/log/checkproc.log\n\n");

        return -1;
    }

    closeioandsignal(true);

    clogfile logfile;
    if (logfile.open(argv[1]) == false)
    {
        printf("logfile open%s failed",argv[1]);return -1;
    }

    int shmid = -1;
    if( (shmid = shmget((key_t)SHMKEYP,MAXNUMP*sizeof(struct st_procinfo),0666|IPC_CREAT)) == -1)
    {
        logfile.write("创建，获取共享内存%x失败\n",SHMKEYP);return -1;
    }

    struct st_procinfo* shmptr = (struct st_procinfo*)shmat(shmid, 0,0);

    for (int ii=0;ii<MAXNUMP;ii++)
    {
        if (shmptr[ii].pid == 0) continue;

        // logfile.write("ii=%d,pid=%d,pname=%s,timeout=%d,atime=%d\n",\
        //              ii,shmptr[ii].pid,shmptr[ii].pname,shmptr[ii].timeout,shmptr[ii].atime);

        
        int iret = kill(shmptr[ii].pid,0);

        
        time_t now = time(0);
        if (now - shmptr[ii].atime < shmptr[ii].timeout) continue;

        st_procinfo tmp = shmptr[ii];

        logfile.write("进程pid=%d(%s)已经超时\n",shmptr[ii].pid,shmptr[ii].pname);

        
        kill(tmp.pid,15);
        
        for (int ii=0;ii<5;ii++)
        {
            sleep(1);
            iret = kill(tmp.pid,0);
            if (iret == -1) break;
        }
       
        if ( (iret == -1) )
        {
           logfile.write("进程pid=%d（%s）正常终止\n",tmp.pid,tmp.pname);
        }
        else
        {
            logfile.write("进程pid=%d（%s非正常终止，被杀死\n",tmp.pid,tmp.pname);
            kill(tmp.pid,9);
            memset(&shmptr[ii],0,sizeof(st_procinfo));
        }
    }

    shmdt(shmptr);
    return 0;
}