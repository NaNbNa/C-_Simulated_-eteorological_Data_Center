#ifndef __PUBLIC_HH
#define __PUBLIC_HH 1

#include "_cmpublic.h"

using namespace std;

namespace idc
{
    //1
    char* deletelchr(char* str,const int cc=' ');
    string& deletelchr(string& str,const int cc=' ');

    //2
    char* deleterchr(char* str,const int cc=' ');
    string& deleterchr(string& str,const int cc=' ');

    char* deletelrchr(char* str,const int cc=' ');
    string& deletelrchr(string& str,const int cc=' ');

    char*   toupper(char* str);
    string& toupper(string& str);

    char*   tolower(char* str);
    string& tolower(string& str);

    bool replacestr(char*   str,const string& str1,const string& str2,const bool bloop=false);
    bool replacestr(string& str,const string& str1,const string& str2,const bool bloop=false);

    char*   picknumber(const string& src,char*   dest,const bool bsigned=false,const bool bdot=false);
    string& picknumber(const string& src,string& dest,const bool bsigned=false,const bool bdot=false);
    string  picknumber(const string& src,const bool bsigned=false,const bool bdot=false);

    class ccmdstr
    {
    private:
        vector<string> m_cmdstr;

        ccmdstr(const ccmdstr&) = delete;
        ccmdstr& operator=(const ccmdstr&) = delete;
    public:
        ccmdstr(){};
        ccmdstr(const string& buffer,const string& sepstr,const bool bdelspace=false)
        {
            splittocmd(buffer,sepstr,bdelspace);
        }

        const string& operator[](int ii) const 
        {
            return m_cmdstr[ii];
        }
        string& operator[](int ii) 
        {
            return m_cmdstr[ii];
        }

        void splittocmd(const string& buffer,const string& sepstr,const bool bdelspace=false);

        int size() const {return m_cmdstr.size();}
        int cmdout() const {return m_cmdstr.size();}

        bool getvalue(const int ii,string&         value,const int ilen=0)  const;
        bool getvalue(const int ii,char*           value,const int ilen=0)  const;
        bool getvalue(const int ii,int&            value)   const;
        bool getvalue(const int ii,unsigned int&   value)   const;
        bool getvalue(const int ii,long&           value)   const;
        bool getvalue(const int ii,unsigned long&  value)   const;
        bool getvalue(const int ii,double&         value)   const;
        bool getvalue(const int ii,float&          value)   const;
        bool getvalue(const int ii,bool&           value)   const;

    }; 
    ostream& operator<<(ostream& out,ccmdstr& cc);

    bool matchstr(const string str,const string& rules);

    bool getxmlbuffer(const string& xmlbuffer,const string& filename,string&        value,const int ilen=0);
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,char*          value,const int ilen=0);
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,int&           value);
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,unsigned int&  value);
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,long&          value);
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,unsigned long& value);
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,double&        value);
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,float&         value);
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,bool&          value);

    template<typename ...Args>
    bool sformat(string& str,const char* fmt,Args... args)
    {
        int len = snprintf(nullptr,0,fmt,args...); 

        if (len < 0) return false;
        if (len == 0) { str.clear();return false;}

        str.resize(len);
        snprintf(&str[0],len+1,fmt,args...);

        return true;
    }

    template<typename ...Args>
    string sformat(const char* fmt,Args... args)
    {
        string str;
        int len = snprintf(nullptr,0,fmt,args...); 

        if (len < 0) return str;
        if (len == 0) return str;

        str.resize(len);
        snprintf(&str[0],len+1,fmt,args...);

        return str;
    }

    string& timetostr(const time_t ttime,string& strtime,const string& fmt="");
    char* timetostr(const time_t ttime,char* strtime,const string& fmt="");
    string timetostr1(const time_t ttime,const string& fmt="");

    string& ltime(string& strtime, const string& fmt="", const int timeval = 0);
    char* ltime(char* strtime, const string& fmt="", const int timeval = 0);
    string ltime1(const string& fmt="",const int timeval =0);

    time_t strtotime(const string& strtime);

    bool addtime(const string& in_stime,char*   out_time,const int timevl,const string& fmt="");
    bool addtime(const string& in_stime,string& out_time,const int timevl,const string& fmt="");

    class ctimer
    {
    private:
        struct timeval m_start;
        struct timeval m_end;

        ctimer(const ctimer& ) = delete;
        ctimer& operator=(const ctimer& ) = delete;
    public:
        ctimer(){start();}

        void start();
        double elapsed();
    };

    bool newdir(const string& pathorfilename,const bool bisfilename=true);

    bool renamefile(const string& srcfilename,const string& dstfilename);

    class cifile
    {
    private:
        ifstream fin;
        string m_filename;
    public:
        cifile(){}

        bool isopen() const {return fin.is_open();};

        bool open(const string& filename,const ios::openmode mode=ios::in);

        bool readline(string& buf,const string& endz="");

        int read(void* buf,const int bufsize);

        bool closeandremove();

        void close(){ if (isopen() == true) fin.close(); }

        ~cifile(){close();}
    };
    class cofile
    {
    private:
        ofstream fout;
        string m_filename;
        string m_filetemp;
    public:
        cofile(){};

        bool isopen() {return fout.is_open();}

        bool open(const string& filename,const bool btemp = true,const ios::openmode mode=ios::out, const bool benbuffer = true);

        template<typename ...Args>
        bool writeline(const char* fmt,Args... args)
        {
            if (isopen() == false) return false;

            fout << sformat(fmt,args...);

            return fout.good();
        }

        template<typename T>
        cofile& operator<<(const T& ee)
        {
            fout << ee ; return *this;
        }

        bool write(void* buffer,const int bufsize);

        bool closeandrename();

        void close();

        ~cofile(){ close();}

    };
    class cdir
    {
    private:
        vector<string> m_filelist;
        int m_hread;
        string m_fmt;

        cdir(const cdir& ) = delete;
        cdir& operator=(const cdir& ) = delete;
    public:
        string m_dirname;
        string m_ffilename;
        string m_filename;
        int m_filesize;
        string m_mtime;
        string m_ctime;
        string m_atime;

        cdir():m_hread(0),m_fmt("yyyymmddhh24miss"){}

        void setfmt(const string& fmt){ m_fmt = fmt;}

        bool opendir(const string& dirname,const string& rules,const int maxfiles=10000,const bool bandchild = false,bool bsort = false);
    private:
        bool _opendir(const string& dirname,const string& rules,const int maxfiles=10000,const bool bandchild=false);
    public:
        bool readdir();

        unsigned int size() { return m_filelist.size();}

        ~cdir(){m_filelist.clear();}
    };
    int filesize(const string& filename);

    bool filemtime(const string& filename,char*   mtime,const string& fmt="yyyymmddhh24miss");
    bool filemtime(const string& filename,string& mtime,const string& fmt="yyyymmddhh24miss");

    bool setmtime(const string& filename,const string& mtime);

    bool copyfile(const string& srcfilename,const string& dstfilename);

    class spinlock_mutex
    {
    private:
        atomic_flag flag;

        spinlock_mutex(const spinlock_mutex&) = delete;
        spinlock_mutex& operator=(const spinlock_mutex&) = delete;
    public:
        spinlock_mutex()
        {
            flag.clear();
        }
        void lock()
        { 
            while(flag.test_and_set());
        }
        void unlock()
        {
            flag.clear();
        }
    };

    class clogfile
    {
    private:
        ofstream fout;
        string m_filename;
        ios::openmode m_mode;
        bool m_backup;
        int m_maxsize;
        bool m_enbuffer;
        spinlock_mutex m_splock;
    public:
        clogfile(int maxsize=100):m_maxsize(maxsize){}

        bool open(const string& filename,const ios::openmode mode=ios::app,const bool bbackup=true,const bool benbuffer=false);
    
        template<typename ...Args>
        bool write(const char* fmt,Args... args)
        {
            if (fout.is_open() == false) return false;

            backup();

            m_splock.lock();
            fout <<  ltime1() << " " << sformat(fmt,args...);
            m_splock.unlock();

            return fout.good();
        }

        template<typename T>
        clogfile& operator<<(const T& value)
        {
            m_splock.lock();
            fout << value;
            m_splock.unlock();

            return *this;
        }

    private:
        bool backup(); 
    public:
        void close(){ if( fout.is_open() == true) fout.close(); }
        ~clogfile(){ close(); }
    };

    class ctcpclient
    {
    private:
        int m_connfd;
        string m_ip;
        int m_port;
    public:
        ctcpclient():m_connfd(-1),m_port(0){}

        bool connect(const string& ip,const int port);

        bool read(string& buffer,const int itimeout=0);
        bool read(void* buffer,const int ibuflen,const int itimeout=0);

        bool write(const string& buffer);
        bool write(const void* buffer,const int ibuflen);

        void close();
    };

    class ctcpserver
    {
    private:
        int m_listen;
        int m_connfd;
        struct sockaddr_in m_clientaddr;
        struct sockaddr_in m_servaddr;
    public:
        ctcpserver():m_listen(-1),m_connfd(-1){}

        bool initserver(const unsigned int port,const int backlog = 5);
        bool accept();
        char* getip();

        bool read(string& buffer,const int itimeout=0);
        bool read(void* buffer,const int ibuflen,const int itimeout=0);

        bool write(const string& buffer);
        bool write(const void* buffer,const int ibuflen);

        void closelisten();
        void closeclient();

        ~ctcpserver(){ closelisten(); closeclient(); }
    };

    bool tcpread(const int sockfd,string& buffer, const int itimeout=0);
    bool tcpread(const int sockfd,void*   buffer, const int ibuflen,const int itimeout=0);

    bool tcpwrite(const int sockfd,const string& buffer);
    bool tcpwrite(const int sockfd,const void*   buffer, const int ibuflen);

    bool readn(const int sockfd,char* buffer,const size_t ibuflen);
    bool writen(const int sockfd,const char* buffer,const size_t ibuflen);
    class cesemp
    {
    private:
        union semun
        {
            int val;
            struct semid_ds* buf;
            unsigned short* array;
        };

        int m_semid;
        short m_sem_flag;

        cesemp(const cesemp&) = delete;
        cesemp& operator=(const cesemp&) = delete;
    public:
        cesemp():m_semid(-1){}

        bool init(const key_t key,const unsigned short value=1,const short sem_flag=SEM_UNDO);
        bool wait(const short value=-1);
        bool post(const short value = 1);
        int getvalue();
        bool destroy();
        ~cesemp(){}
    };

    struct st_procinfo
    {
        int pid =0;
        char pname[51]= {0};
        int timeout =0;
        int atime =0;
    };

    class cpactive
    {
    private:
        int m_shmid;
        int m_pos;
        struct st_procinfo* m_shmptr;
    
    public:
        cpactive():m_shmid(-1),m_pos(-1),m_shmptr(nullptr){}

        bool addinfo(const int timeout,const string& pname="",clogfile* logfile=nullptr);

        bool uptatime()
        {
            if (m_pos == -1) return false;

            m_shmptr[m_pos].pid = time(0);
            return true;
        }

        ~cpactive()
        {
            if (m_pos != -1) memset(m_shmptr+ m_pos,0,sizeof(st_procinfo));

            if (m_shmid != -1) shmdt(m_shmptr);
        }

    };
}

#endif
