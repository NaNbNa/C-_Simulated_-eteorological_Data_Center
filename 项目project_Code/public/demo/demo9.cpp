#include "_public.h"

using namespace idc;

int main()
{
    char strtime1[21];
    memset(strtime1,0,sizeof(strtime1));
    string strtime2;

    ltime(strtime1,"yyyy-mm-dd hh24:mi:ss");
    cout << "strtime1=" << strtime1 <<endl;

    ltime(strtime1,"yyyy-mm-dd hh24:mi:ss",30);
    cout << "strtime1=" << strtime1 <<endl;

    ltime(strtime1,"yyyy-mm-dd hh24:mi:ss",-30);
    cout << "strtime1=" << strtime1 <<endl;

    cout << "-----------------" <<endl;
    ltime(strtime2,"yyyy-mm-dd hh24:mi:ss");
    cout << "strtime2=" << strtime2 <<endl;

    ltime(strtime2,"yyyy-mm-dd hh24:mi:ss",30);
    cout << "strtime2=" << strtime2 <<endl;

    ltime(strtime2,"yyyy-mm-dd hh24:mi:ss",-30);
    cout << "strtime2=" << strtime2 <<endl;

    strtime2 = ltime1("yyyy-mm-dd hh24:mi:ss",60);
    cout << "strtime2=" << strtime2 <<endl;
}