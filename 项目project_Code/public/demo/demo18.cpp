#include "_public.h"

using namespace idc;

int main()
{
    cofile ofile;

    if (ofile.open("/tmp/data/girl.dat",true,ios::binary) == false)
    {
        cout << "open /tmp/data/girl.dat failed\n" <<endl;return -1;
    }

    struct st_girl
    {
        int bh;
        char name[21];
    }girl;

    memset(&girl,0,sizeof(girl));
    girl.bh = 8;
    strcpy(girl.name,"西施");

    ofile.write(&girl,sizeof(girl));
    
   

    ofile.closeandrename();
}