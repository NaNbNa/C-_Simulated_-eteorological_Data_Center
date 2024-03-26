#include "_public.h"

using namespace idc;

int main()
{
    char str1[31];
    string str2;

    strcpy(str1,"  西施  ");
    //strcpy(str1,"  x  ");
    deletelchr(str1,' ');
    cout << "str1=" << str1 <<"="<<endl;

    str2 = "  西施  ";
    //str2 = "  x  ";
    deletelchr(str2,' ');
    cout << "str2=" << str2 <<"="<<endl;

    strcpy(str1,"  西施  ");
    // strcpy(str1,"  x  ");
    deleterchr(str1,' ');
    cout << "str1=" << str1 <<"="<<endl;

    str2 = "  西施  ";
    // str2 = "x ";
    deleterchr(str2,' ');
    cout << "str2=" << str2 <<"="<<endl;

    strcpy(str1,"  西施  ");
    // strcpy(str1,"  x  ");
    deletelrchr(str1,' ');
    cout << "str1=" << str1 <<"="<<endl;

    str2 = "  西施  ";
    // str2 = "x ";
    deletelrchr(str2,' ');
    cout << "str2=" << str2 <<"="<<endl;
}
