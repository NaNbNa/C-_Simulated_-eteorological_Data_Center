#include "_public.h"

using namespace idc;

int main()
{
    int bh =1;
    char name[31];strcpy(name,"西施");
    double weight = 48.2;
    string yz = "漂亮";

    char s1[100];
    int len = snprintf(s1,100,"编号=%2d,姓名=%2s,体重=%.2f,颜值=%s",bh,name,weight,yz.c_str());
    cout << "s1=" << s1 <<endl;
    // snprintf(s1,100,"编号=%03d,姓名=%2s,体重=%.3f,颜值=%s",bh,name,weight,yz.c_str());
    // cout << "s1=" << s1 <<endl;
    printf("len=%d\n",len);
    printf("strlen(s1)=%d\n",strlen(s1));

    string s2;
    s2 = "编号=" + to_string(bh) + "姓名=" + name + "体重=" + to_string(weight) + "颜值=" + yz;
    cout << "s2=" << s2 <<endl;

    s2 = sformat("编号=%2d,姓名=%2s,体重=%.2f,颜值=%s",bh,name,weight,yz.c_str());
    cout << "s2=" << s2 <<endl;

    s2 = "";
    cout << s2 << "---------------" <<endl;
    sformat(s2,"编号=%2d,姓名=%2s,体重=%.2f,颜值=%s",bh,name,weight,yz.c_str());
    cout << "s2=" << s2 <<endl;
 
}