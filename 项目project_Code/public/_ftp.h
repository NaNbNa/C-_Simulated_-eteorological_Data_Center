#ifndef __FTP_H
#define __FTP_H

#include "_public.h"
#include "ftplib.h"

namespace idc
{

class cftpclient
{
private:
    netbuf* m_ftpconn;
public:
    unsigned int m_size;
    string m_mtime;

    bool m_connectfailed;
    bool m_loginfailed;
    bool m_optionfailed;    

    cftpclient();
    ~cftpclient();
    
    cftpclient(const cftpclient&) = delete;
    cftpclient& operator=(const cftpclient&) = delete;

    void initdata();

    bool login(const string& host, const string& usename,const string& passwd,const int imode=FTPLIB_PASSIVE);

    bool logout();

    bool mtime(const string& remotefilename);

    bool chdir(const string& remotedir);

    bool size(const string& remotefilename);

    bool mkdir(const string& remotedir);

    bool rmdir(const string& remotedir);

    bool nlist(const string& remotedir,const string& listfilename);

    bool get(const string& remotefilename,const string& localfilename,const bool bcheckmtime=true);

    bool put(const string& locfilename,const string& remotefilename,const bool bcheckmtime=true);

    bool site(const string& command);

    bool ftpdelete(const string& remotefilename);

    bool ftprename(const string& srcremotefilename,const string& dstremotefilename);

    char* response();

};

}

#endif