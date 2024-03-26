#include "_public.h"

using namespace idc;

int main()
{
    cifile ifile;

    if (ifile.open("/tmp/data/girl.dat",ios::binary) == false)
    {
        cout << "open(/tmp/data/girl.dat,ios::binary) failed" <<endl;
    }

    struct st_girl
    {
        int bh;
        char name[21];
    }girl;

    memset(&girl,0,sizeof(girl));

    ifile.read(&girl,sizeof(girl));

    cout << "girl.bh=" << girl.bh << "girl.name=" << girl.name <<endl;

    ifile.close();

}