#include "_public.h"

using namespace idc;

int main()
{
    string buffer = "<name>梅西</name><no>10</no><striker>true</striker><age>30</age><weight>68.5</weight><sal>21000000</sal><club>Barcelona</club>";

    struct st_player
    {
        string name;
        char no[6];
        bool striker;
        int age;
        double weight;
        long sal;
        char club[51];
    }stplayer;

    getxmlbuffer(buffer,"name",stplayer.name);
    cout << "name=" << stplayer.name <<endl;

    getxmlbuffer(buffer,"no",stplayer.no,5);
    cout << "no=" << stplayer.no <<endl;

    getxmlbuffer(buffer,"striker",stplayer.striker);
    cout << "striker=" << stplayer.striker <<endl;

    getxmlbuffer(buffer,"age",stplayer.age);
    cout << "age=" << stplayer.age <<endl;

    getxmlbuffer(buffer,"weight",stplayer.weight);
    cout << "weight=" << stplayer.weight <<endl;

    getxmlbuffer(buffer,"sal",stplayer.sal);
    cout << "sal=" << stplayer.sal <<endl;

    getxmlbuffer(buffer,"club",stplayer.club);
    cout << "club=" << stplayer.club <<endl;
}