#include "_public.h"
#include "_ftp.h"

using namespace idc;

struct st_arg
{
    char host[31];
    int mode;
    char username[31];
    char password[31];
    char remotepath[256];
    char localpath[256];
    char matchname[256];
    int ptype;
    char remotepathbak[256];
    char okfilename[256];
    bool checkmtime;
    int timeout;
    char pname[51]; //进程名字
}starg;

struct st_fileinfo
{
    string filename;
    string mtime;
    st_fileinfo() = default;
    st_fileinfo(const string& in_filename,const string& in_mtime):filename(in_filename),mtime(in_mtime){}
    void clear(){filename.clear();mtime.clear();}
};
void EXIT(int sig);

void _help();

bool xmltoarg(const char* strxmlbuffer);

bool loadlistfile();

bool loadokfile();

bool compmap();

bool writetookfile();

bool appendtookfile(const st_fileinfo& stfileinfo);

map<string,string> mfromok;
list<st_fileinfo> vfromnlist;
list<st_fileinfo> vtook;
list<st_fileinfo> vdownload;

clogfile logfile;
cftpclient ftp;

cpactive pactive;

int main(int argc,char* argv[])
{
    if (argc != 3)
    {
        _help();
        return -1;
    }
    closeioandsignal(true);
    signal(2,EXIT);signal(15,EXIT);

   
    if (logfile.open(argv[1])  == false)
    {
        printf("logfile.open(%s) failed\n",argv[1]);return -1;
    }

    if (xmltoarg(argv[2]) == false) return -1;

    pactive.addpinfo(starg.timeout,starg.password);

    if (ftp.login(starg.host,starg.username,starg.password,starg.mode) == false)
    {
        logfile.write("ftp login(%s,%s,%s,%d) failed\n%s\n",starg.host,starg.username,starg.password,starg.mode,ftp.response());
        return -1;    
    }

    pactive.uptatime();

    if (ftp.chdir(starg.remotepath) == false)
    {
        logfile.write("ftp chdir(%s) failed\n%s\n",starg.remotepath,ftp.response());return -1;
    }


    if (ftp.nlist(".", sformat("/tmp/nlist/ftpgetfiles_%d.nlist",getpid())) == false)
    {
        logfile.write("ftp nlist(%s) failed\n%s\n",starg.remotepath,ftp.response());return -1;
    }
    // logfile.write("ftp nlist(%s) sucess\n%s\n",starg.remotepath,ftp.response());

    pactive.uptatime();

    if (loadlistfile() == false)
    {
        logfile.write("loadlistfile() failed\n%s\n",ftp.response());return -1;
    }

    if (starg.ptype == 1)
    {
        loadokfile();

        compmap();

        writetookfile();
    }
    else
        vfromnlist.swap(vdownload);
    
    pactive.uptatime();

    string strremotefilename,strlocalfilename;

    for (auto& aa:vdownload)
    {
        sformat(strremotefilename,"%s/%s",starg.remotepath,aa.filename.c_str());
        sformat(strlocalfilename,"%s/%s",starg.localpath,aa.filename.c_str());

        logfile.write("get %s...",strremotefilename.c_str());
    
        if (ftp.get(strremotefilename,strlocalfilename,starg.checkmtime) == false)
        {
            logfile << "failed \n" << ftp.response() << "\n"; return -1;
        }
        logfile << "ok \n";
        
        pactive.uptatime();

        if (starg.ptype == 1) appendtookfile(aa);

        if (starg.ptype == 2)   //删除已下载的内容
        {
            if (ftp.ftpdelete(strremotefilename) == false) 
            {
                logfile.write("ftp ftpdelte.delete(%s) failed\n%s\n",strremotefilename,ftp.response());return false;
            }
        }

        if (starg.ptype == 3)   //移动已下载的文件到其他地方
        {
            string strremotefilenamebak = sformat("%s/%s",starg.remotepathbak,aa.filename.c_str());
            if (ftp.ftprename(strremotefilename,strremotefilenamebak) == false)
            {
                logfile.write("ftp ftprename(%s,%s) failed\n%s\n",strremotefilename.c_str(),strremotefilenamebak.c_str(),ftp.response());
                return false;
            }
        }
        
    }

    return 0;
}

///////////////////

void _help()
{
    printf("Example: \n");
    printf("/project/tools/bin/ftpgetfiles /log/idc/ftpgetfiles_surfdata.log "\
            "\"<host>192.168.240.131:21</host><mode>1</mode>"\
            "<username>test1</username><password>1</password>"\
            "<remotepath>/tmp/idc/surfdata</remotepath><localpath>/idcdata/surfdata</localpath>"\
            "<matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname>"\
            "<ptype>1</ptype><remotepathbak>/tmp/idc/surfdatabak</remotepathbak>"\
            "<okfilename>/idcdata/fplist/ftpgetfiles_test.xml</okfilename>"\
            "<checkmtime>true</checkmtime><timeout>30</timeout>"\
            "<pname>ftpgetfiles_test</pname>\"\n\n"
            );
    // printf("/project/tools/bin/ftpgetfiles /log/idc/ftpgetfiles_surfdata.log "\
    //         "\"<host>192.168.240.131:21</host><mode>1</mode>"\
    //         "<username>test1</username><password>1</password>"\
    //         "<remotepath>/tmp/idc/surfdata/server</remotepath><localpath>/idcdata/surfdata</localpath>"\
    //         "<matchname>*.TXT</matchname>"\
    //         "<ptype>1</ptype><remotepathbak>/tmp/idc/surfdatabak</remotepathbak>"\
    //         "<okfilename>/idcdata/fplist/ftpgetfiles_test.xml</okfilename>"\
    //         "<checkmtime>true</checkmtime>\"\n\n"
    //         );
}
void EXIT(int sig)
{
    printf("sig=%d\n",sig);

    exit(0);
}

bool xmltoarg(const char* xmlbuffer)
{
    memset(&starg,0,sizeof(st_arg));

    getxmlbuffer(xmlbuffer,"host",starg.host,30);
    if (strlen(starg.host) == 0) 
    {
        logfile.write("host is empty\n");return false;
    }

    getxmlbuffer(xmlbuffer,"mode",starg.mode);
    if (starg.mode != 2) starg.mode = 1;

    getxmlbuffer(xmlbuffer,"username",starg.username,30);
    if (strlen(starg.username) == 0) 
    {
        logfile.write("username is empty\n");return false;
    }

    getxmlbuffer(xmlbuffer,"password",starg.password,30);
    if (strlen(starg.password) == 0) 
    {
        logfile.write("password is empty\n");return false;
    }

    getxmlbuffer(xmlbuffer,"remotepath",starg.remotepath,256);
    if (strlen(starg.remotepath) == 0) 
    {
        logfile.write("remotepath is empty\n");return false;
    }

    getxmlbuffer(xmlbuffer,"localpath",starg.localpath,256);
    if (strlen(starg.localpath) == 0) 
    {
        logfile.write("localpath is empty\n");return false;
    }

    getxmlbuffer(xmlbuffer,"matchname",starg.matchname,256);
    if (strlen(starg.matchname) == 0) 
    {
        logfile.write("matchname is empty\n");return false;
    }

    getxmlbuffer(xmlbuffer,"ptype",starg.ptype);
    if ((starg.ptype != 1) && (starg.ptype != 2) && (starg.ptype != 3))
    {
        logfile.write("ptype is empty\n");return false;
    }

    if (starg.ptype == 3)
    {
        getxmlbuffer(xmlbuffer,"remotepathbak",starg.remotepathbak,256);
        if (strlen(starg.remotepathbak) == 0) 
        {
            logfile.write("remotepathbak is empty\n");return false;
        }
    }

    if (starg.ptype == 1)
    {
        getxmlbuffer(xmlbuffer,"okfilename",starg.okfilename,256);
        if (strlen(starg.okfilename) == 0) 
        {
            logfile.write("okfilename is empty\n");return false;
        }

        getxmlbuffer(xmlbuffer,"checkmtime",starg.checkmtime);
        if (starg.checkmtime != true) starg.checkmtime = false;
    }

    getxmlbuffer(xmlbuffer,"timeout",starg.timeout);
    if (starg.timeout == 0 )
    {
        logfile.write("timeout is empty\n");return false;
    }

    getxmlbuffer(xmlbuffer,"pname",starg.pname,50);


    return true;

}

bool loadlistfile()
{
    vfromnlist.clear();

    cifile ifile;
    if (ifile.open(sformat("/tmp/nlist/ftpgetfiles_%d.nlist",getpid())) == false)
    {
        logfile.write("ifile.open(%s) failed\n",sformat("/tmp/nlist/ftpgetfiles_%d.nlist",getpid()));
        return false;
    }

    string filename;

    while(true)
    {
        if (ifile.readline(filename) == false) break;

        if (matchstr(filename,starg.matchname) == false) continue;

        if ( (starg.ptype == 1) && (starg.checkmtime == true))
        {
            if ( ftp.mtime(filename) == false )
            {
                logfile.write("ftp.mtime(%s)\n%s\n",filename.c_str(),ftp.response());return -1;
            }
        }

        vfromnlist.emplace_back(filename,ftp.m_mtime);
    }

    ifile.closeandremove();

    // for (auto& aa:vfromnlist)
    //     logfile.write("filename=%s,mtime=%s\n",aa.filename.c_str(),aa.mtime.c_str());
    
    return true;
}

bool loadokfile()
{
    if (starg.ptype != 1) return false;

    mfromok.clear();

    cifile ifile;
    if ( ifile.open(starg.okfilename) == false) return true;

    string strbuffer;

    struct st_fileinfo stfileinfo;

    while(true)
    {
        stfileinfo.clear();

        if (ifile.readline(strbuffer) == false) break;

        getxmlbuffer(strbuffer,"filename",stfileinfo.filename);
        getxmlbuffer(strbuffer,"mtime",stfileinfo.mtime);

        mfromok[stfileinfo.filename] = stfileinfo.mtime;
    }

    return true;
}

bool compmap()
{
    vtook.clear();
    vdownload.clear();

    for (auto&aa:vfromnlist)
    {
        auto it = mfromok.find(aa.filename);

        if (it != mfromok.end())    
        {
            if (starg.checkmtime == true)
            {
                if (it->second == aa.mtime) vtook.push_back(aa);
                else {vdownload.push_back(aa);}
            }
            else
                vtook.push_back(aa);
        }
        else
        {
            vdownload.push_back(aa);
        }
    }

    return true;
}
bool writetookfile()
{
    cofile ofile;

    if (ofile.open(starg.okfilename) == false) 
    {
        logfile.write("ofile.open(%s)\n",starg.okfilename);return false;
    }

    for(auto&aa:vtook)
    {
        ofile.writeline("<filename>%s</filename><mtime>%s</mtime>\n",aa.filename.c_str(),aa.mtime.c_str());
    }

    ofile.closeandrename();

    return true;
}

bool appendtookfile(const st_fileinfo& stfileinfo)
{
    cofile ofile;

    if (ofile.open(starg.okfilename,false,ios::app) == false)
    {
        logfile.write("ofile.wirte(%s,false,ios::app) failed\n",starg.okfilename);return false;
    }

    ofile.writeline("<filename>%s</filename><mtime>%s</mtime>\n",stfileinfo.filename.c_str(),stfileinfo.mtime.c_str());

    return true;
}