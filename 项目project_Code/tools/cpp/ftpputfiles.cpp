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
    char matchname[101];
    int ptype;
    char localpathbak[256];
    char okfilename[256];
    int timeout;
    char pname[51];
}starg;

struct st_fileinfo
{
    string filename;
    string mtime;
    st_fileinfo() = default;
    st_fileinfo(const string& in_filename,const string& in_mtime):filename(in_filename),mtime(in_mtime){}
    void clear(){ filename.clear(); mtime.clear(); }
};

void EXIT(int sig);

void help();

bool xmltoarg(const string& xmlbuffer);

bool loadfilelist();

bool loadtomap();

bool comptoput();

bool writetook();

bool appendtook(const st_fileinfo& stfileinfo);


map<string,string> mfromok;
list<st_fileinfo> vfromdir;
list<st_fileinfo> vtook;
list<st_fileinfo> vputlist;


clogfile logfile;
cftpclient ftp;
cpactive pactive;


int main(int argc,char* argv[])
{
    if (argc != 3)
    {
        help();
        return -1;
    }

    //closeioandsignal(true);
    signal(SIGINT,EXIT);signal(SIGTERM,EXIT);

    if (logfile.open(argv[1]) == false)
    {
        logfile.write("logfile.open%s failed\n",argv[1]);return -1;
    } 
    //logfile.write("logfile open(%s) sucess\n",argv[1]);

    if (xmltoarg(argv[2]) == false)
    {
        logfile.write("xmltoarg(%s) failed\n",argv[2]);return -1;
    }
    
    pactive.addpinfo(starg.timeout,starg.pname);

    if ( ftp.login(starg.host,starg.username,starg.password,starg.mode) == false)
    {
        logfile.write("ftp.login(%s,%s,%s,%d) failed\n%s\n",starg.host,starg.username,starg.password,starg.mode,ftp.response());
        return -1;
    }
    //logfile.write("ftp login sucess\n",argv[1]);

    pactive.uptatime();

    if (loadfilelist() == false)
    {
        logfile.write("loadfilelist() failed\n");return -1;
    }
    
    pactive.uptatime();

    if (starg.ptype == 1)
    {
        loadtomap();

        comptoput();

        writetook();
    }
    else
        vfromdir.swap(vputlist);

    pactive.uptatime();
    
    string strlocalfilename,strremotefilename;
    for(auto& aa: vputlist)
    //for(auto& aa: vfromdir)
    {
        sformat(strlocalfilename,"%s/%s",starg.localpath,aa.filename.c_str());
        sformat(strremotefilename,"%s/%s",starg.remotepath,aa.filename.c_str());

        logfile.write("put(%s,%s) ...",strlocalfilename.c_str(),strremotefilename.c_str());
        if (ftp.put(strlocalfilename,strremotefilename,true) == false)
        {
            logfile << "failed\n" << ftp.response()  << "\n";
            return -1;
        }
        logfile << " ok \n";
         
        pactive.uptatime();

        if (starg.ptype == 1) appendtook(aa);
        
        if (starg.ptype == 2)
        {
            if (remove(strlocalfilename.c_str()) != 0)
            {
                logfile.write("remove(%s) failed\n",strlocalfilename.c_str());return -1;
            }
        }

        if (starg.ptype == 3)
        {   
            string strlocalfilenamebak = sformat("%s/%s",starg.localpathbak,aa.filename.c_str());
            if ( renamefile(strlocalfilename.c_str(),strlocalfilenamebak.c_str()) == false )
            {
                logfile.write("rename(%s,%s) failed\n",strlocalfilename.c_str(),strlocalfilenamebak.c_str());return -1;
            }
        }
    }
    
}

void help()
{
    printf("\n");
    printf("/project/tools/bin/ftpputfiles /log/idc/ftpputfiles_surfdata.log "\
            "\"<host>192.168.240.131:21</host><mode>1</mode>"\
            "<username>test1</username><password>1</password>"\
            "<remotepath>/tmp/idc/surfdata</remotepath><localpath>/idcdata/surfdata</localpath>"\
            "<matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname>"\
            "<ptype>1</ptype><localpathbak>/tmp/idc/surfdatabak</localpathbak>"\
            "<okfilename>/idcdata/fplist/ftpputfiles_test.xml</okfilename>"\
            "<checkmtime>true</checkmtime><timeout>30</timeout>"\
            "<pname>ftpputfiles_test</pname>\"\n\n"
            );  
    // printf("/project/tools/bin/ftpputfiles /log/idc/ftpputfiles_surfdata.log "\
    //         "\"<host>192.168.240.131:21</host><mode>1</mode>"\
    //         "<username>test1</username><password>1</password>"\
    //         "<remotepath>/tmp/idc/surfdata/server</remotepath><localpath>/idcdata/surfdata</localpath>"\
    //         "<matchname>*.TXT</matchname>"\
    //         "<ptype>1</ptype><localpathbak>/tmp/idc/surfdatabak</localpathbak>"\
    //         "<okfilename>/idcdata/fplist/ftpputfiles_test.xml</okfilename>"\
    //         "<checkmtime>true</checkmtime><timeout>30</timeout>"\
    //         "<pname>ftpputfiles_test</pname>\"\n\n"
    //         );  
    
}

void EXIT(int sig)
{
    printf("sig=%d",sig);

    exit(0);
}

bool xmltoarg(const string& xmlbuffer)
{

    getxmlbuffer(xmlbuffer,"host",starg.host,30);
    if (strlen(starg.host) == 0)
    {
        logfile.write("host is null\n");return false;
    }

    getxmlbuffer(xmlbuffer,"mode",starg.mode);
    if ((starg.mode != 1) && (starg.mode != 2))
    {
        logfile.write("mode is null\n");return false;
    }

    getxmlbuffer(xmlbuffer,"username",starg.username,30);
    if (strlen(starg.username) == 0)
    {
        logfile.write("username is null\n");return false;
    }

    getxmlbuffer(xmlbuffer,"password",starg.password,30);
    if (strlen(starg.password) == 0)
    {
        logfile.write("password is null\n");return false;
    }   
    
    getxmlbuffer(xmlbuffer,"remotepath",starg.remotepath,255);
    if (strlen(starg.remotepath) == 0)
    {
        logfile.write("remotepath is null\n");return false;
    }

    getxmlbuffer(xmlbuffer,"localpath",starg.localpath,255);
    if (strlen(starg.localpath) == 0)
    {
        logfile.write("localpath is null\n");return false;
    }

    getxmlbuffer(xmlbuffer,"matchname",starg.matchname,100);
    if (strlen(starg.matchname) == 0)
    {
        logfile.write("matchname is null\n");return false;
    }

    getxmlbuffer(xmlbuffer,"ptype",starg.ptype);
    //if (starg.ptype != 1 && starg.ptype != 2) 

    if (starg.ptype == 1)
    {
        getxmlbuffer(xmlbuffer,"okfilename",starg.okfilename,255);
        if (strlen(starg.okfilename) == 0)
        {
            logfile.write("okfilename is null\n");return false;
        }

    }

    if (starg.ptype == 3)
    {
        getxmlbuffer(xmlbuffer,"localpathbak",starg.localpathbak,255);
        if (strlen(starg.localpathbak) == 0)
        {
            logfile.write("localpathbak is null\n");return false;
        }
    }

    getxmlbuffer(xmlbuffer,"timeout",starg.timeout);
    if (starg.timeout == 0)
    {
        logfile.write("timeout is null\n");return false;
    }

    getxmlbuffer(xmlbuffer,"pname",starg.pname,50);

    return true;
}

bool loadfilelist()
{
    vfromdir.clear();

    cdir dir;
    if (dir.opendir(starg.localpath,starg.matchname,10000,false) == false)
    {
        logfile.write("dir.open(%s,%s,10000,false) failed\n",starg.localpath,starg.matchname);
        return false;
    }

    while( dir.readdir() == true)
    {
        vfromdir.emplace_back(dir.m_filename,dir.m_mtime);
    }

    // for(auto& aa:vfromdir)
    //     logfile.write("filename=%s,mtime=%s\n",aa.filename.c_str(),aa.mtime.c_str());
    
    
    return true;
}

bool loadtomap()
{
    mfromok.clear();

    cifile ifile;
    if (ifile.open(starg.okfilename) == false) return true;
    

    string strbuffer;
    string strfilename, strmtime;
  
    while(true)
    {   
        if (ifile.readline(strbuffer) == false) break;

        getxmlbuffer(strbuffer,"filename",strfilename);
        getxmlbuffer(strbuffer,"mtime",strmtime);
        
        mfromok[strfilename] = strmtime;
    }

    // for(auto& aa:mfromok)
    //     logfile.write("filename=%s,mtime=%s\n",aa.first.c_str(),aa.second.c_str());

    return true;    
} 
bool comptoput()
{
    vtook.clear();vputlist.clear();

    
    for(auto& aa: vfromdir)
    {
        auto it = mfromok.find(aa.filename);

        if (it != mfromok.end())
        {
            if (aa.mtime == it->second) vtook.push_back(aa);
            else vputlist.push_back(aa);
        }
        else
        {
            vputlist.push_back(aa);
        }

    }

    // for(auto& aa:vputlist)
    //     logfile.write("vputlist -- filename=%s,mtime=%s\n",aa.filename.c_str(),aa.mtime.c_str());
    
    // for(auto& aa:vtook)
    //     logfile.write("vtook -- filename=%s,mtime=%s\n",aa.filename.c_str(),aa.mtime.c_str());
    
    return true;
}

bool writetook()
{
    cofile ofile;
    if (ofile.open(starg.okfilename) == false)
    {
        logfile.write("ofile.open(%s) failed\n",starg.okfilename);
        return false;
    }

    for(auto& aa:vtook)
    {
        ofile.writeline("<filename>%s</filename><mtime>%s</mtime>\n",aa.filename.c_str(),aa.mtime.c_str());
    }

    ofile.closeandrename();
    
    return true;
}


bool appendtook(const st_fileinfo& stfileinfo)
{
    cofile ofile;
    if (ofile.open(starg.okfilename,false,ios::app) == false)
    {
        logfile.write("ofile.open(%s,false,ios::app) failed\n",starg.okfilename);
        return false;
    }

   ofile.writeline("<filename>%s</filename><mtime>%s</mtime>\n",stfileinfo.filename.c_str(),stfileinfo.mtime.c_str());

   return true;
}