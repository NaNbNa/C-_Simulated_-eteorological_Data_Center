#include "_public.h"

using namespace idc;

int main()
{
    if (newdir("/tmp/aaa/bbb/ccc/ddd",false) == false) cout << "false" <<endl;

    if (newdir("/tmp/111/222/333/data.xml",true) == false) cout << "false" <<endl;
}