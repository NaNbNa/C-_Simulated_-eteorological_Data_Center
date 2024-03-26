#include "_public.h"

using namespace idc;

int main(int argc,char* argv[])
{
    if (argc !=3)
    {
        cout << "Using: ./demo15 pathname matchstr\n" ;
        cout << "Sample: ./demo15 /project/public \"*h,*.cpp\" \n";
        return -1;
    }

    cdir dir;

    if(dir.opendir(argv[1],argv[2],100,false,true) == false)
    {
        cout << sformat("dir.opendir(%s) failed\n",argv[1]);return -1;
    }

    while(dir.readdir() == true)
    {
        cout << "filename=" << dir.m_ffilename << ",mtime=" <<dir.m_mtime << ",size=" << dir.m_filesize <<endl;
    }
    
}