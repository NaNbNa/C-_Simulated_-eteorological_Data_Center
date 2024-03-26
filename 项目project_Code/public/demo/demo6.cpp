#include "_public.h"

using namespace idc;

int main()
{
    if (matchstr("_public.h","*.h,*.cpp") == true) 
        cout << "yes" <<endl;
    else 
        cout << "no" <<endl;
    
    if (matchstr("_public.h","*.H") == true) 
        cout << "yes" <<endl;
    else 
        cout << "no" <<endl;
    
    if (matchstr("_public.h","*p*k*.h") == true) 
        cout << "yes" <<endl;
    else 
        cout << "no" <<endl;
}