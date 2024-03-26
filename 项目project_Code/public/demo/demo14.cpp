#include "_public.h"

using namespace idc;

int main()
{
    if (renamefile("/project/public/lib_public.so","/tmp/aaa/bbb/ccc/lib_public.so") == false)
    {
        cout << "renamefile(/project/public/lib_public.so)" << errno << strerror(errno) <<endl;
    }

    if (copyfile("/project/public/libftp.a","/tmp/aaa/ddd/ccc/libftp.a") == false)
    {
        cout << "copyfile(/project/public/libftp.a)" << errno << strerror(errno) <<endl;
    }

    printf("size=%d\n", filesize("/project/public/_public.h"));

    setmtime("/project/public/_public.h","2024-01-05 13:37:29");

    string mtime;
    filemtime("/project/public/_public.h",mtime,"yyyy-mm-dd hh24:mi:ss");
    cout << "mtime=" << mtime <<endl;

    filemtime("/project/public/_public.h",mtime);
    cout << "mtime="<< mtime <<endl;
}