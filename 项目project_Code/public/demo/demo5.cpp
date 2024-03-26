#include "_public.h"

using namespace idc;

struct st_player
{
    char name[51];
    char no[6];
    bool striker;
    int age;
    double weight;
    long sal;
    char club[51];
}stplayer;

int main()
{
    memset(&stplayer,0,sizeof(st_player));

    string buffer = "messi~!~10~!~true~!~a30~!~68.5~!~2100000~!~Barc,elona";

    ccmdstr cmdstr(buffer, "~!~");

    for (int ii=0;ii<cmdstr.size();ii++)
    {
         cout << "cmdstr["<<ii<<"]=" << cmdstr[ii] << endl;
    }
    cout << cmdstr;

    cmdstr.getvalue(0,stplayer.name,50);
    cmdstr.getvalue(1,stplayer.no,5);
    cmdstr.getvalue(2,stplayer.striker);
    cmdstr.getvalue(3,stplayer.age);
    cmdstr.getvalue(4,stplayer.weight);
    cmdstr.getvalue(5,stplayer.sal);
    cmdstr.getvalue(6,stplayer.club,50);

    printf("name=%s,no=%s,striker=%d,age=%d,weight=%.1f,sal=%ld,club=%s\n",\
               stplayer.name,stplayer.no,stplayer.striker,stplayer.age,\
               stplayer.weight,stplayer.sal,stplayer.club);
}