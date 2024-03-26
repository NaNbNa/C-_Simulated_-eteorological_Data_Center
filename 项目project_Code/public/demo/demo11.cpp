#include "_public.h"

using namespace idc;

int main()
{
    char strtime[21];
    memset(strtime,0,sizeof(strtime));

    strcpy(strtime,"2020-01-20 12:35:22");
    cout << "strtime=" << strtime <<endl;

    char s1[21];
    addtime(strtime,s1,0-1*24*60*60);
    cout << "s1=" << s1 <<endl;

    string s2;
    addtime(strtime,s2,1*24*60*60);
    cout << "s2=" << s2 <<endl;
}