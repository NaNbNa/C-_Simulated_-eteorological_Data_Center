#include "_public.h"

using namespace idc;

int main()
{
    char str1[31];
    memset(str1,0,sizeof(str1));
    string str2;

    strcpy(str1,"iab+12.3xy");
    picknumber(str1,str1);
    printf("str1=%s=\n",str1);

    str2 = "iab+12.3xy";
    picknumber(str2,str2);
    cout << "str2=" << str2 << "=" <<endl;

    strcpy(str1,"iab+12.3xy");
    picknumber(str1,str1,true,false);
    printf("str1=%s=\n",str1);

    str2 = "iab+12.3xy";
    picknumber(str2,str2,true,false);
    cout << "str2=" << str2 << "=" <<endl;

    strcpy(str1,"iab+12.3xy");
    picknumber(str1,str1,true,true);
    printf("str1=%s=\n",str1);

    str2 = "iab+12.3xy";
    picknumber(str2,str2,true,true);
    cout << "str2=" << str2 << "=" <<endl;
}