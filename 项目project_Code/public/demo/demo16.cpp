#include "_public.h"

using namespace idc;

int main()
{
    cofile ofile;
    if (ofile.open("/tmp/data/girl.xml",true) == false) 
    {    
        cout << "open(\"/tmp/data/girl.xml\") failed " <<endl;return -1;
    }

    sleep(10);
    ofile << "<data>" << "\n";

    ofile.writeline("<name>%s</name><age>%d</age><sc>%s</sc><yz>%s</yz><memo>%s</memo><endl/>\n",\
                           "妲已",28,"火辣","漂亮","商要亡，关我什么事。");
    ofile.writeline("<name>%s</name><age>25</age><sc>火辣</sc><yz>漂亮</yz><memo>1、中国排名第一的美女；\n"\
         "2、男朋友是范蠡；\n"\
         "3、老公是夫差，被勾践弄死了。</memo><endl/>\n","西施");
    
    ofile << "<data>\n";

    ofile.closeandrename();
}