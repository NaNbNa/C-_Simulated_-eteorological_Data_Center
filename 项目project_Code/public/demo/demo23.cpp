#include "../_ftp.h"

using namespace idc;

int main()
{
    cftpclient ftp;

    if (ftp.login("192.168.240.131:21","test1","1") == false)
    {
        printf("ftp.login(192.168.240.131,test1/1 faile\n)");return -1;
    }

    if (ftp.nlist("/project/public222/*.h","/tmp/list/tmp.list") == false)
    {
        printf("ftp.nlist(/project/public/*.h,/tmp/list/tmp.list) failed\n");
        return -1;
    }

    cout << "ret=" << ftp.response() <<endl;

    cifile ifile;
    string strFileName;

    ifile.open("/tmp/list/tmp.list");
    while(true)
    {
        if (ifile.readline(strFileName) == false)   break;

        ftp.mtime(strFileName);
        ftp.size(strFileName);

        printf("filename=%s,mtime=%s\n",strFileName.c_str(),ftp.m_mtime.c_str(),ftp.m_size);
    }
}