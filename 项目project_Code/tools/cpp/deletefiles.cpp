#include "_public.h"

using namespace idc;
void EXIT(int sig);

cpactive pactive;

int main(int argc,char* argv[])
{
    if (argc !=4)
    {
        printf("Example: /project/tools/bin/deletefiles /tmp/idc/surfdata \"*.xml,*.json,*.csv\" 0.01 \n");
        printf("Example: /project/tools/bin/deletefiles /log/idc \"*.log.20*\" 0.02 \n");
        
        printf("/project/tools/bin/procctl 300 /project/tools/bin/deletefiles /tmp/idc/surfdata \"*.xml,*.json,*.csv\" 0.01 \n");
        printf("/project/tools/bin/procctl 300 /project/tools/bin/deletefiles /log/idc \"*.log.20*\" 0.02 \n");
        return -1;
    }
    closeioandsignal(true);
    signal(2,EXIT);signal(15,EXIT);

    pactive.addpinfo(30,"deletefiles");

    string stritimeout = ltime1("yyyymmddhh24miss",0-(int)(atof(argv[3])*24*60*60));
    //cout << "strtimeout=" << stritimeout <<endl;

    cdir dir;
    if (dir.opendir(argv[1],argv[2],10000,true) == false)
    {
        printf("cdir opendir(%s )failed\n",argv[1]);return -1;
    }

   

    while(dir.readdir() == true)
    {
        if (dir.m_mtime < stritimeout)
        {
            if (remove(dir.m_ffilename.c_str()) == 0)
            {
                cout << "remove" << dir.m_ffilename << "ok\n";
            }
            else
            {
                cout << "remove" << dir.m_ffilename << "failed\n";
            }
        }
    }


    return 0;
}

void EXIT(int sig)
{
    printf("sig=%d\n",sig);

    exit(0);
}