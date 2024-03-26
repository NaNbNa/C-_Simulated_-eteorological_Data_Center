#include "_ftp.h"


namespace idc
{
    cftpclient::cftpclient()
    {
        m_ftpconn = 0;

        initdata();

        FtpInit();

        m_connectfailed = false;
        m_loginfailed = false;
        m_optionfailed = false;
    }

    cftpclient::~cftpclient()
    {
        logout();
    }

    void cftpclient::initdata()
    {
        m_size = 0;

        m_mtime.clear();
    }

    bool cftpclient::login(const string& host, const string& username,const string& passwd,const int imode)
    {
        if (m_ftpconn != 0)
        {
            FtpQuit(m_ftpconn);m_ftpconn = 0;
        }

        m_connectfailed = m_loginfailed = m_optionfailed = false;

        if (FtpConnect(host.c_str(),&m_ftpconn) == false)
        {
            m_connectfailed = true; return false;
        }

        if (FtpLogin(username.c_str(),passwd.c_str(),m_ftpconn) == false)
        {
            m_loginfailed = true;return false;
        }

        if (FtpOptions(FTPLIB_CONNMODE,(long)imode,m_ftpconn) == false)
        {
            m_optionfailed = true;return false;
        }

        return true;
    }

    bool cftpclient::logout()
    {
        if (m_ftpconn == 0) return false;

        FtpQuit(m_ftpconn);

        m_ftpconn = 0;

        return true;
    }

    bool cftpclient::get(const string& remotefilename,const string& localfilename,const bool bcheckmtime)
    {
        if (m_ftpconn == 0) return false;

        newdir(localfilename);

        string strlocalfilenametmp = localfilename + ".tmp";

        if (mtime(remotefilename) == false) return false;

        if (FtpGet(strlocalfilenametmp.c_str(),remotefilename.c_str(),FTPLIB_IMAGE,m_ftpconn) == false)
            return false;

        if (bcheckmtime == true)
        {
            string strmtime = m_mtime;

            if (mtime(remotefilename) == false) return false;

            if (m_mtime != strmtime) return false;
        }   

        setmtime(strlocalfilenametmp,m_mtime);

        if (rename(strlocalfilenametmp.c_str(),localfilename.c_str()) != 0 ) return false;

        return true;
    }

    bool cftpclient::mtime(const string& remotefilename)
    {
        if (m_ftpconn == 0) return false;

        m_mtime.clear();

        string strmtime;
        strmtime.resize(14);

        if (FtpModDate(remotefilename.c_str(),&strmtime[0],14,m_ftpconn) == false) return false;

        addtime(strmtime,m_mtime,0+8*60*60,"yyyymmddhh24miss"); //UTC+8

        return true;
    }

    bool cftpclient::size(const string& remotefilename)
    {
        if (m_ftpconn == 0) return false;

        m_size = 0;

        if (FtpSize(remotefilename.c_str(),&m_size,FTPLIB_IMAGE,m_ftpconn) == false) return false;

        return true;
    }

    bool cftpclient::mkdir(const string& remotedir)
    {
        if (m_ftpconn == 0) return false;

        if (FtpMkdir(remotedir.c_str(),m_ftpconn) == false) return false;

        return true;
    }

    bool cftpclient::chdir(const string& remotedir)
    {
        if (m_ftpconn == 0) return false;

        if (FtpChdir(remotedir.c_str(),m_ftpconn) == false) return false;

        return true;
    }
    bool cftpclient::rmdir(const string& remotedir)
    {
        if (m_ftpconn == 0) return false;

        if (FtpRmdir(remotedir.c_str(),m_ftpconn) == false) return false;
        
        return true;
    }

    bool cftpclient::nlist(const string& remotedir,const string& listfilename)
    {
        if (m_ftpconn == 0) return false;

        newdir(listfilename.c_str());

        if (FtpNlst(listfilename.c_str(),remotedir.c_str(),m_ftpconn) == false) return false;

        return true;
    }

    bool cftpclient::put(const string& localfilename,const string& remotefilename,const bool bchecksize)
    {
        if (m_ftpconn == 0) return false;

        string remotefilenametmp = remotefilename + ".tmp";

        string filetime1,filetime2;
        filemtime(localfilename,filetime1);

        if (FtpPut(localfilename.c_str(),remotefilenametmp.c_str(),FTPLIB_IMAGE,m_ftpconn) == false)
            return false;
        
         filemtime(localfilename,filetime2);
        if (filetime1 != filetime2) { ftpdelete(remotefilenametmp); return false;}

        if (FtpRename(remotefilenametmp.c_str(),remotefilename.c_str(),m_ftpconn) == false)
            return false;

        if (bchecksize == true)
        {
            if (size(remotefilename) == false) return false;

            if (m_size != filesize(localfilename)) 
            {
                ftpdelete(remotefilename);return false;
            }
        }

        return true;
    }

    bool cftpclient::ftpdelete(const string& remotefilename)
    {
        if (m_ftpconn == 0) return false;

        if (FtpDelete(remotefilename.c_str(),m_ftpconn) == false) return false;

        return true;
    }

    bool cftpclient::ftprename(const string& srcremotefilename,const string& dstremotefilename)
    {
        if (m_ftpconn == 0) return false;

        if (FtpRename(srcremotefilename.c_str(),dstremotefilename.c_str(),m_ftpconn) == false)
            return false;

        return true;
    }

    bool cftpclient::site(const string& command)
    {
        if (m_ftpconn == 0)

        if (FtpSite(command.c_str(),m_ftpconn) == false) return false;

        return true;
    }

    char* cftpclient::response()
    {
        if (m_ftpconn == 0) return nullptr;

        return FtpLastResponse(m_ftpconn);
    }

}