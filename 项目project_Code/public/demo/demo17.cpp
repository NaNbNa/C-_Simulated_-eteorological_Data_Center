#include "_public.h"

using namespace idc;

int main()
{
    cifile ifile;

    if (ifile.open("/tmp/data/girl.xml") == false)
    {
        cout << "open /tmp/data/girl.xml failed" <<endl;
        return -1;
    }

    string strline;
    while(true)
    {
        if (ifile.readline(strline,"<endl/>") == false) break;

        cout << strline <<endl;
    }
    ifile.close();
}