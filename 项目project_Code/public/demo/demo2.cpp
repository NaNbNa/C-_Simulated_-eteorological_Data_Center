#include "_public.h"

using namespace idc;

int main()
{
    char str1[31];
    string str2;

    
    strcpy(str1,"12abz45ABz8西施。");
    cout << "old-str1=" << str1 << "=" <<endl;
    toupper(str1);
    cout << "new-str1=" << str1 << "=" <<endl;

    strcpy(str1,"12abz45ABz8西施。");
    cout << "old-str1=" << str1 << "=" <<endl;
    tolower(str1);
    cout << "new-str1=" << str1 << "=" <<endl;

    str2 = "12abz45ABz8西施。";
    cout << "old-str2=" << str2 << "=" <<endl;
    toupper(str2);
    cout << "new-str2=" << str2 << "=" <<endl;

    str2 = "12abz45ABz8西施。";
    cout << "old-str2=" << str2 << "=" <<endl;
    tolower(str2);
    cout << "new-str2=" << str2 << "=" <<endl;
}