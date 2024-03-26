#include "_public.h"

using namespace idc;

int main()
{
    char str1[301];

    strcpy(str1,"name:messi,no:10,job:striker.");
    replacestr(str1,":","=");
    cout << "str1=" << str1 << "=" <<endl;

    strcpy(str1,"name:messi,no:10,job:striker.");
    replacestr(str1,"name:","");
    cout << "str1=" << str1 << "=" <<endl;

    strcpy(str1,"messi----10----striker");  
    replacestr(str1,"--","-",false);  
    cout << "str1=" << str1 << "=" <<endl;

    strcpy(str1,"messi----10----striker");  
    replacestr(str1,"--","-",true);
    cout << "str1=" << str1 << "=" <<endl;

    strcpy(str1,"messi-10-striker");  
    replacestr(str1,"-","--",true); 
    cout << "str1=" << str1 << "=" <<endl;

    ////////////////////////////
    cout << "--------------------------" <<endl;
    string str2;
    str2="name:messi,no:10,job:striker.";
    replacestr(str2,":","=");           
    cout << "str2=" << str2 << "=" <<endl;

    str2="name:messi,no:10,job:striker.";
    replacestr(str2,"name:","");
    cout << "str2=" << str2 << "=" <<endl;

    str2="messi----10----striker";  
    replacestr(str2,"--","-",false);              
    cout << "str2=" << str2 << "=\n";

    str2="messi----10----striker";  
    replacestr(str2,"--","-",true);                
    cout << "str2=" << str2 << "=\n";    

    str2="messi-10-striker";  
    replacestr(str2,"-","--",false);             
    cout << "str2=" << str2 << "=\n";     

    str2="messi-10-striker";  
    replacestr(str2,"-","--",true);                
    cout << "str2=" << str2 << "=\n";
}