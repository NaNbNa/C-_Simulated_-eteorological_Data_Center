#include "_public.h"

using namespace idc;

int main()
{
    string strtime;
    strtime = "2020-01-01 12:35:22";

    time_t ttime;
    ttime = strtotime(strtime);
    cout << "ttime=" << ttime <<endl;

    char s1[21];
    timetostr(ttime,s1,"yyyy-mm-dd hh24:mi:ss");    
    cout << "s1=" << s1 <<endl;

    string s2;
    timetostr(ttime,s2,"yyyy-mm-dd hh24:mi:ss");
    cout << "s2=" << s2 <<endl;
}