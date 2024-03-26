#include "_public.h"

namespace idc
{
    char* deletelchr(char* str,const int cc)
    {
        if (str == nullptr) return nullptr;

        char* p = str;
        while(*p==cc) p++;

        memmove(str,p,strlen(p)+1);

        return str;
    }

    string& deletelchr(string& str,const int cc)
    {
        int pos = str.find_first_not_of(cc);    //false--return -1

        str.replace(0,pos,"");    
        
        return str;
    }

    char* deleterchr(char* str,const int cc)
    {
        if (str == nullptr) return nullptr;

        char* p = str;
        char* ppos = 0;
        while(*p!=0) 
        {
            if ((*p==cc) && (ppos==0)) ppos = p;
            if (*p != cc) ppos = 0;

            p++;
        }

        if (ppos!=0)    *ppos = 0;

        return str;
    }

    string& deleterchr(string& str,const int cc)
    {
        int pos = str.find_last_not_of(cc);    //false--return -1

        str.erase(pos+1);   
        
        return str;
    }

    char* deletelrchr(char* str,const int cc)
    {
        deletelchr(str,cc);
        deleterchr(str,cc);
        return str;
    }
    string& deletelrchr(string& str,const int cc)
    {
        deletelchr(str,cc);
        deleterchr(str,cc);
        return str;
    }

    char*   toupper(char* str)
    {
        if (str == nullptr) return nullptr;

        char* p = str;
        while(*p!=0)
        {
            if ((*p>='a')&&(*p<='z')) *p = *p - 'a' + 'A';
            p++;
        }

        return str;
    }
    string& toupper(string& str)
    {
        for(auto& cc: str)
            if ( (cc>='a') && (cc<='z')) cc = cc - 'a' + 'A';
        
        return str;
    }
    char*   tolower(char* str)
    {
        if (str == nullptr) return nullptr;

        char* p = str;
        while(*p!=0)
        {
            if ((*p>='A')&&(*p<='Z')) *p = *p - 'A' + 'a';
            p++;
        }

        return str;
    }
    string& tolower(string& str)
    {
        for(auto& cc: str)
            if ( (cc>='A') && (cc<='Z')) cc = cc - 'A' + 'a';
        
        return str;
    }
    bool replacestr(char*   str,const string& str1,const string& str2,const bool bloop)
    {
        if (str == nullptr) return false;

        string strtemp(str);
        if (replacestr(strtemp,str1,str2,bloop) == false) return false;;

        int itemplen = strtemp.length();
        memset(str,0,itemplen);

        // char* p = str;
        // for(auto& cc:strtemp)
        // {
        //     *p = cc; p++;
        // }
        // *p =0;

        strtemp.copy(str,itemplen);
        str[itemplen] = 0;

        return true;
    }
    bool replacestr(string& str,const string& str1,const string& str2,const bool bloop)
    {
        if (str1.empty() == true) return false;
        if ((bloop==true)&&(str2.find(str1)!=string::npos)) return false;

        int start =0,pos =0;
        int str1len = str1.length(),str2len = str2.length();
        while(true)
        {
            if (bloop == true&&pos==0) start =0;

            if ( (pos = str.find(str1,start)) == string::npos)
            {
                if (bloop == true)
                    {
                        if (start == 0) break;
                        pos =0;start =0;continue;
                    }
                else
                    break;
            }

            str.replace(pos,str1len,str2);

            start = pos + str2len;
        }

        return true;
    }

    char*   picknumber(const string& src,char*   dest,const bool bsigned,const bool bdot)
    {
        if (dest == nullptr) return nullptr;

        char* p = dest;
        for(auto&cc: src)
        {
            if ( (cc>='0')&&(cc<='9') ) {*p = cc; p++; continue;}
            if ( (bsigned == true) && ((cc == '+') || (cc == '-')) ) {*p = cc; p++; continue;}
            if ( (bdot == true) && (cc == '.') ) {*p = cc; p++;}
        }
        *p = 0;

        return dest;
    }
    string& picknumber(const string& src,string& dest,const bool bsigned,const bool bdot)
    {
        string strtemp;
        for(auto& cc:src)
        {
            if ( isdigit(cc) ) {strtemp.append(1,cc); continue;}
            if ( (bsigned == true) && ((cc == '+') || (cc == '-')) ) {strtemp.append(1,cc); continue;}
            if ( (bdot == true) && (cc == '.') ) {strtemp.append(1,cc);}
        }
        
        dest = strtemp;

        return dest;
    }
    string  picknumber(const string& src,const bool bsigned,const bool bdot)
    {
        string str;
        picknumber(src,str,bsigned,bdot);

        return str;
    }

    void ccmdstr::splittocmd(const string& buffer,const string& sepstr,const bool bdelspace)
    {
        m_cmdstr.clear();
        if (sepstr.empty() == true) return;

        string substr;
        int start =0,pos=0;
        int seplen = sepstr.length();
        while(true)
        {
            if ( (pos = buffer.find(sepstr,start)) == string::npos ) break;

            substr = buffer.substr(start,pos-start);
            if (bdelspace == true) deletelrchr(substr,' ');
            m_cmdstr.push_back(std::move(substr));

            start = pos + seplen;
        }
        substr = buffer.substr(start);
        if (bdelspace == true) deletelrchr(substr,' ');
        m_cmdstr.push_back(std::move(substr));

        return;
    }

    bool ccmdstr::getvalue(const int ii,string&         value,const int ilen)   const
    {
        if (ii >= size()) return false;

        int itemplen = m_cmdstr[ii].length();
        if ( (ilen > 0) || (ilen < itemplen)) itemplen = ilen;

        value = m_cmdstr[ii].substr(0,itemplen);
        return true;
    }   
    bool ccmdstr::getvalue(const int ii,char*           value,const int ilen)    const
    {
        if (ii >= size()) return false;

        int itemplen = m_cmdstr[ii].length();
        if ( (ilen > 0) || (ilen < itemplen)) itemplen = ilen;

        m_cmdstr[ii].copy(value,itemplen);
        value[itemplen] = 0;

        return true;
    }
    bool ccmdstr::getvalue(const int ii,int&            value)   const
    {
        if (ii >= size()) return false;

        try
        {
            value = stoi(picknumber(m_cmdstr[ii],true,false));
        }
        catch(const std::exception& e)
        {
            return false;
        }
        
        return true;
    }
    bool ccmdstr::getvalue(const int ii,unsigned int&   value)   const
    {
        if (ii >= size()) return false;

        try
        {
            value = stoi(picknumber(m_cmdstr[ii],false,false));
        }
        catch(const std::exception& e)
        {
            return false;
        }
        
        return true;
    }
    bool ccmdstr::getvalue(const int ii,long&           value)   const
    {
        if (ii >= size()) return false;

        try
        {
            value = stol(picknumber(m_cmdstr[ii],true,false));
        }
        catch(const std::exception& e)
        {
            return false;
        }
        
        return true;
    }
    bool ccmdstr::getvalue(const int ii,unsigned long&  value)   const
    {
        if (ii >= size()) return false;

        try
        {
            value = stoul(picknumber(m_cmdstr[ii],false,false));
        }
        catch(const std::exception& e)
        {
            return false;
        }
        
        return true;
    }
    bool ccmdstr::getvalue(const int ii,double&         value)   const
    {
        if (ii >= size()) return false;

        try
        {
            value = stod(picknumber(m_cmdstr[ii],true,true));
        }
        catch(const std::exception& e)
        {
            return false;
        }
        
        return true;
    }
    bool ccmdstr::getvalue(const int ii,float&          value)   const
    {
        if (ii >= size()) return false;

        try
        {
            value = stof(picknumber(m_cmdstr[ii],true,true));
        }
        catch(const std::exception& e)
        {
            return false;
        }
        
        return true;
    }
    bool ccmdstr::getvalue(const int ii,bool&           value)   const
    {
        if (ii >= size()) return false;

        string str = m_cmdstr[ii];
        toupper(str);

        if (str == "TRUE") value = true;
        else value = false;

        return true;
    }
    ostream& operator<<(ostream& out,ccmdstr& cmdstr)
    {
        for (int ii=0;ii<cmdstr.size();ii++)
        {
            out << "[" << ii << "]=" << cmdstr[ii] << endl;
        }

        return out;
    }

    bool matchstr(const string str,const string& rules)
    {
        if (rules.empty() == true) return false;
        if (rules == "*") return true;

        string strtemp = str,rulestemp = rules;
        toupper(strtemp); toupper(rulestemp);

        ccmdstr cmdstr,subcmdstr;
        cmdstr.splittocmd(rulestemp,",");

        int cmdsize = cmdstr.size(),subsize;
        int start=0,pos =0;
        int slen = strtemp.length();
        int jj;
        for (int ii=0;ii<cmdsize;ii++)
        {
            if (cmdstr[ii].empty() == true) continue;   

            subcmdstr.splittocmd(cmdstr[ii],"*");
            subsize = subcmdstr.size();
            start =0;
            for(jj=0;jj<subsize;jj++)
            {
                if (jj == 0)
                    if ( (strtemp.substr(0,subcmdstr[jj].length())!=subcmdstr[0]) ) break;
                
                if (jj == subsize-1)
                    if ( (strtemp.find(subcmdstr[jj],slen - subcmdstr[jj].length()) == string::npos))
                        break;
                
                if ( (pos = strtemp.find(subcmdstr[jj],start)) == string::npos ) break;

                start = pos + subcmdstr[jj].length();
            }
            if (jj == subsize) return true;
        }

        return false;
    }

    bool getxmlbuffer(const string& xmlbuffer,const string& filename,string&        value,const int ilen)
    {
        string start = "<" +filename + ">";
        string end   = "</" +filename + ">";

        int spos,epos;
        spos = xmlbuffer.find(start,0);
        if (spos == string::npos) return false;

        epos = xmlbuffer.find(end,spos);
        if (epos == string::npos) return false; 

        int itemplen = epos - spos - start.length();
        if ((ilen > 0) && (ilen < itemplen)) itemplen = ilen;

        value = xmlbuffer.substr(spos + start.length(),itemplen);

        return true;
    }
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,char*          value,const int ilen)
    {
        string str;
        if ( getxmlbuffer(xmlbuffer,filename,str,ilen) == false) return false;

        int itemplen = str.length();
        if ((ilen > 0) && (ilen < itemplen)) itemplen = ilen;

        str.copy(value,itemplen);
        value[itemplen] = 0;

        return true;
    }
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,int&           value)
    {
        string str;
        if ( getxmlbuffer(xmlbuffer,filename,str) == false) return false;

        try
        {
            value = stoi(picknumber(str,true,false));
        }
        catch(const std::exception& e)
        {
            return false;
        }
        
        return true;
    }
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,unsigned int&  value)
    {
        string str;
        if ( getxmlbuffer(xmlbuffer,filename,str) == false) return false;

        try
        {
            value = stoi(picknumber(str,false,false));
        }
        catch(const std::exception& e)
        {
            return false;
        }
        
        return true;
    }
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,long&          value)
    {
        string str;
        if ( getxmlbuffer(xmlbuffer,filename,str) == false) return false;

        try
        {
            value = stol(picknumber(str,true,false));
        }
        catch(const std::exception& e)
        {
            return false;
        }
        
        return true;
    }
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,unsigned long& value)
    {
        string str;
        if ( getxmlbuffer(xmlbuffer,filename,str) == false) return false;

        try
        {
            value = stoul(picknumber(str,false,false));
        }
        catch(const std::exception& e)
        {
            return false;
        }
        
        return true;
    }
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,double&        value)
    {
        string str;
        if ( getxmlbuffer(xmlbuffer,filename,str) == false) return false;

        try
        {
            value = stod(picknumber(str,true,true));
        }
        catch(const std::exception& e)
        {
            return false;
        }
        
        return true;
    }
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,float&         value)
    {
        string str;
        if ( getxmlbuffer(xmlbuffer,filename,str) == false) return false;

        try
        {
            value = stof(picknumber(str,true,true));
        }
        catch(const std::exception& e)
        {
            return false;
        }
        
        return true;
    }
    bool getxmlbuffer(const string& xmlbuffer,const string& filename,bool&          value)
    {
        string str;
        if ( getxmlbuffer(xmlbuffer,filename,str) == false) return false;

        toupper(str);
        if (str == "TRUE") value = true;
        else value = false;

        return true;
    }
    string& timetostr(const time_t ttime,string& strtime,const string& fmt)
    {
        struct tm sttm;localtime_r(&ttime,&sttm);

        sttm.tm_year = sttm.tm_year + 1900;
        sttm.tm_mon++;

        if ( (fmt=="") || (fmt == "yyyy-mm-dd hh24:mi:ss"))
        {
            strtime = sformat("%04u-%02u-%02u %02u:%02u:%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday,\
                                                              sttm.tm_hour,sttm.tm_min,sttm.tm_sec);
            return strtime;
        }

        if (fmt=="yyyy-mm-dd hh24:mi")
        {
            strtime=sformat("%04u-%02u-%02u %02u:%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday,\
                    sttm.tm_hour,sttm.tm_min);
            return strtime;
        }
        //3
        if (fmt=="yyyy-mm-dd hh24")
        {
            strtime=sformat("%04u-%02u-%02u %02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday,sttm.tm_hour);
            return strtime;
        }
        //4
        if (fmt=="yyyy-mm-dd")
        {
            strtime=sformat("%04u-%02u-%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday); 
            return strtime;
        }

        if (fmt=="yyyy-mm") //5
        {
            strtime=sformat("%04u-%02u",sttm.tm_year,sttm.tm_mon); 
            return strtime;
        }

        if (fmt=="yyyymmddhh24miss") //6
        {
            strtime=sformat("%04u%02u%02u%02u%02u%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday,\
                    sttm.tm_hour,sttm.tm_min,sttm.tm_sec);
            return strtime;
        }

        if (fmt=="yyyymmddhh24mi") //7
        {
            strtime=sformat("%04u%02u%02u%02u%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday,\
                    sttm.tm_hour,sttm.tm_min);
            return strtime;
        }

        if (fmt=="yyyymmddhh24") //8
        {
            strtime=sformat("%04u%02u%02u%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday,sttm.tm_hour);
            return strtime;
        }

        if (fmt=="yyyymmdd") //9
        {
            strtime=sformat("%04u%02u%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday); 
            return strtime;
        }

        if (fmt=="hh24miss") //10
        {
            strtime=sformat("%02u%02u%02u",sttm.tm_hour,sttm.tm_min,sttm.tm_sec); 
            return strtime;
        }

        if (fmt=="hh24mi") //11
        {
            strtime=sformat("%02u%02u",sttm.tm_hour,sttm.tm_min); 
            return strtime;
        }

        if (fmt=="hh24") //12
        {
            strtime=sformat("%02u",sttm.tm_hour); 
            return strtime;
        }

        if (fmt=="mi") //13
        {
            strtime=sformat("%02u",sttm.tm_min); 
            return strtime;
        }

        return strtime;
    }
    char* timetostr(const time_t ttime,char* strtime,const string& fmt)
    {
        if (strtime == nullptr) return nullptr;

        string str;
        timetostr(ttime,str,fmt);

        int len = str.length();
        str.copy(strtime,len);
        strtime[len] = 0;

        return strtime;
    }
    string timetostr1(const time_t ttime,const string& fmt)
    {
        string strtime;
        timetostr(ttime,strtime,fmt);

        return strtime;
    }

    string& ltime(string& strtime, const string& fmt, const int timeval)
    {
        time_t timer;
        time(&timer);

        timer = timer + timeval;
        timetostr(timer,strtime,fmt);
        return strtime;
    }
    char* ltime(char* strtime, const string& fmt, const int timeval)
    {
        if (strtime == nullptr) return nullptr;
        
        time_t timer;
        time(&timer);

        timer = timer + timeval;
        timetostr(timer,strtime,fmt);
        return strtime;
    }
    string ltime1(const string& fmt,const int timeval)
    {
        string strtime;
        ltime(strtime,fmt,timeval);
        return strtime;
    }

    time_t strtotime(const string&strtime)
    {
        string strtemp;
        picknumber(strtime,strtemp,false,false);

        if (strtemp.length()!=14) return -1;

        int year,mon,day,hour,min,sec;
        try
        {
            year = stoi(strtemp.substr(0,4)) - 1900;
            mon  = stoi(strtemp.substr(4,2)) - 1;
            day  = stoi(strtemp.substr(6,2));
            hour = stoi(strtemp.substr(8,2));
            min  = stoi(strtemp.substr(10,2));
            sec  = stoi(strtemp.substr(12));
        }
        catch(const std::exception& e)
        {
            return -1;
        }
        
        struct tm sttm;
        sttm.tm_year = year;sttm.tm_mon = mon;sttm.tm_mday = day;
        sttm.tm_hour = hour;sttm.tm_min = min;sttm.tm_sec  = sec;

        return mktime(&sttm);
    }

    bool addtime(const string& in_stime,char*   out_time,const int timevl,const string& fmt)
    {
        if (out_time == nullptr) return false;

        time_t timer;
        if ( (timer = strtotime(in_stime)) == -1) {strcpy(out_time,""); return false;}

        timer = timer + timevl;
        timetostr(timer,out_time,fmt);

        return true;
    }
    bool addtime(const string& in_stime,string& out_time,const int timevl,const string& fmt)
    {
        time_t timer;

        if ( (timer = strtotime(in_stime)) == -1) {out_time = ""; return false;}

        timer = timer + timevl;
        timetostr(timer,out_time,fmt);

        return true;        
    }
    void ctimer::start()
    {
        memset(&m_start,0,sizeof(struct timeval));
        memset(&m_end,0,sizeof(struct timeval));

        gettimeofday(&m_start,0);
    }

    double ctimer::elapsed()
    {
        gettimeofday(&m_end,0);

        string sstart,send;
        double dstart,dend;
        sstart = sformat("%ld.%06ld",m_start.tv_sec,m_start.tv_usec);
        send   = sformat("%ld.%06ld",m_end.tv_sec,m_end.tv_usec);
        try
        {
            dstart = stod(sstart);
            dend   = stod(send);
        }
        catch(const std::exception& e)
        {
            return -1;
        }

        start();

        return dend - dstart;
    }

    bool newdir(const string& pathorfilename,const bool bisfilename)
    {
        int start =1,pos;
        string subdir;
        while(true)
        {
            if ( (pos = pathorfilename.find("/",start)) == string::npos) break;
            
            subdir = pathorfilename.substr(0,pos);

            if (access(subdir.c_str(), F_OK) != 0)
            {
                if (mkdir(subdir.c_str(),0755) != 0) return false;
            }

            start = pos + 1;
        }

        if (bisfilename == false)
        {
            if (access(pathorfilename.c_str(), F_OK) != 0)
            {
                if (mkdir(pathorfilename.c_str(),0755) != 0) return false;
            }
        }

        return true;
    }

    bool renamefile(const string& srcfilename,const string& dstfilename)
    {
        if (access(srcfilename.c_str(), R_OK) != 0 ) return false;

        if (newdir(dstfilename, true) == false) return false;

        if (rename(srcfilename.c_str(), dstfilename.c_str()) == 0) return true;

        return false;
    }

    bool cifile::open(const string& filename,const ios::openmode mode)
    {
        if (isopen() == true) {fin.close();}

        m_filename = filename;

        fin.open(filename,mode);

        return fin.is_open();
    }
    bool cifile::readline(string& buf,const string& endbz)
    {
        buf.clear();

        string strline;
        while(true)
        {
            getline(fin,strline);

            if (fin.eof()) break;

            buf = buf + strline;
            if (endbz == "")
                return true;
            else
            {
                if (buf.find(endbz,strline.length() - endbz.length()) != string::npos) return true;
            }

            buf = buf + "\n";
        }

        return false;
    }

    int cifile::read(void* buffer,const int bufsize)
    {
        if (buffer == nullptr) return -1;

        fin.read(static_cast<char*>(buffer),bufsize);

        return fin.gcount();
    }

    bool cifile::closeandremove()
    {
        if (isopen() == false) return false;
        fin.close();

        if (remove(m_filename.c_str()) != 0) return false;

        return true;
    }
    bool cofile::open(const string& filename,const bool btemp,const ios::openmode mode, const bool benbuffer)
    {
        if (isopen() == true) fout.close();

        m_filename = filename;

        if (newdir(m_filename) == false) return false;
        if (btemp == true)
        {
            m_filetemp = m_filename + ".tmp" ;
            fout.open(m_filetemp,mode);
        }
        else
        {   
            m_filetemp.clear();
            fout.open(m_filename,mode);
        }

        if (benbuffer == false) fout << unitbuf;

        return fout.is_open();
    }
    bool cofile::write(void* buffer,const int bufsize)
    {
        if (isopen() == false) return false;

        fout.write(static_cast<char*>(buffer),bufsize);

        return fout.good();
    }

    bool cofile::closeandrename()
    {
        if (isopen() == false) return false;
        fout.close();

        if (m_filetemp.empty() == false)
            if (rename(m_filetemp.c_str(),m_filename.c_str()) !=0) return false;
        
        return true;
    }

    void cofile::close()
    {
        if (isopen() == false) return;
        fout.close();

        if (m_filetemp.empty() == false)
            remove(m_filetemp.c_str());
        
        return;
    }
    bool cdir::opendir(const string& dirname,const string&rules,const int maxfiles,const bool bandchild,bool bsort)
    {
        m_filelist.clear();
        m_hread =0;

        if ( newdir(dirname,false) == false ) return false;

        bool ret = _opendir(dirname,rules,maxfiles,bandchild);

        if (bsort == true) sort(m_filelist.begin(),m_filelist.end());

        return ret;
    }
    bool cdir::_opendir(const string& dirname,const string& rules,const int maxfiles,const bool bandchild)
    {
        DIR* dir;
        if ((dir = ::opendir(dirname.c_str())) == nullptr) return false;

        string filepath;
        struct dirent *stdir;

        while ( (stdir = ::readdir(dir)) != 0)
        {
            if ( m_filelist.size() > maxfiles) break;

            if ( stdir->d_name[0] == '.') continue;

            filepath = dirname + '/' + stdir->d_name;

            if (stdir->d_type == 4)
            {
                if (bandchild == true)
                {
                    if ( _opendir(filepath,rules,maxfiles,bandchild) == false)
                        return false;
                }   
            }

            if (stdir->d_type == 8)
            {
                if (matchstr(filepath,rules) == false) continue;

                m_filelist.push_back(std::move(filepath));
            }
        }

        closedir(dir);
        return true;
    }
    bool cdir::readdir()
    {
        if (m_hread >= size())
        {
            m_hread = 0;m_filelist.clear();return false;
        }

        m_ffilename = m_filelist[m_hread];

        int pp = m_ffilename.find_last_of("/");
        m_dirname  = m_ffilename.substr(0,pp);
        m_filename = m_ffilename.substr(pp+1);

        struct stat st_file;
        stat(m_ffilename.c_str(),&st_file);
        m_filesize = st_file.st_size;
        m_mtime    = timetostr1(st_file.st_mtime,m_fmt);
        m_ctime    = timetostr1(st_file.st_ctime,m_fmt);
        m_atime    = timetostr1(st_file.st_atime,m_fmt);

        m_hread++;
        return true;
    }

    
    int filesize(const string& filename)
    {
        struct stat st_file;
        stat(filename.c_str(),&st_file);

        return st_file.st_size;
    }

    bool filemtime(const string& filename,char*  mtime,const string& fmt)
    {
        if (mtime == nullptr) return false;

        struct stat st_file;
        stat(filename.c_str(),&st_file);
        
        timetostr(st_file.st_mtime,mtime,fmt);

        return true;
    }

    bool filemtime(const string& filename,string& mtime,const string& fmt)
    {
        struct stat st_file;
        stat(filename.c_str(),&st_file);
        
        timetostr(st_file.st_mtime,mtime,fmt);

        return true;
    }

    bool setmtime(const string& filename,const string& mtime)
    {
        struct utimbuf st_timebuf;
        st_timebuf.actime = st_timebuf.modtime = strtotime(mtime);

        if ( utime(filename.c_str(),&st_timebuf) != 0) return false;

        return true;
    }

    bool copyfile(const string& srcfilename,const string& dstfilename)
    {
        if (newdir(dstfilename,true) == false) return false;

        cifile ifile;cofile ofile;

        if (ifile.open(srcfilename, ios::in|ios::binary) == false) return false;
        if (ofile.open(dstfilename,ios::out|ios::binary) == false) return false;

        int ifilesize = filesize(srcfilename);
        int total_bytes = 0;
        int onread;
        char buffer[5000];
        int bufsize = sizeof(buffer);

        while(true)
        {
            if (ifilesize - total_bytes >= bufsize) onread = bufsize;
            else onread = ifilesize - total_bytes;

            memset(buffer,0,bufsize);

            ifile.read(buffer,onread);
            ofile.write(buffer,onread);

            total_bytes = total_bytes + onread;
            if (total_bytes == ifilesize) break;
        }

        ifile.close();
        ofile.close();
        
        string strtime; 
        filemtime(srcfilename,strtime);
        setmtime(srcfilename,strtime);

        return true;
    }

    bool clogfile::open(const string& filename,const ios::openmode mode,const bool bbackup,const bool benbuffer)
    {
        if (fout.is_open() == true) fout.close();

        m_filename = filename;
        m_mode = mode;
        m_backup = bbackup;
        m_enbuffer = benbuffer;
        
        if (newdir(m_filename,true) == false) return false;
        fout.open(m_filename,mode);

        if(benbuffer == false) fout << unitbuf;

        return fout.is_open();
    }

    bool clogfile::backup()
    {
        if (m_backup == false) return true;

        if (fout.is_open() == false) return false;

        if (fout.tellp() >= m_maxsize*1024*1024)
        {
            m_splock.lock();

            string strtemp = m_filename + "." + ltime1("yyyymmddhh24miss");
            while (access(strtemp.c_str(),F_OK) == 0)  //如果重名了，就循环命名
            {
                strtemp = strtemp + ".bak" ;
            }
            rename(m_filename.c_str(),strtemp.c_str());

            fout.close();
            fout.open(m_filename,m_mode);

            if(m_enbuffer == false) fout << unitbuf;

            m_splock.unlock();

            return fout.is_open();
        }

        return true;
    }

    bool ctcpclient::connect(const string& ip,const int port)
    {
        if (m_connfd >= 0) { ::close(m_connfd); m_connfd = -1;}

        m_ip = ip; m_port = port;

        struct hostent* h;
        struct sockaddr_in servaddr;
        memset(&servaddr,0,sizeof(servaddr));

        if ( (m_connfd = socket(AF_INET,SOCK_STREAM,0)) < 0) return false;

        if ( !( h = gethostbyname(m_ip.c_str())) ) 
        {
            ::close(m_connfd);m_connfd = -1; return false;
        }

        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(m_port);
        memcpy(&servaddr.sin_addr.s_addr,h->h_addr,h->h_length);

        if (::connect (m_connfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) !=0)
        {
            ::close(m_connfd);m_connfd = -1; return false;
        }

        return true;
    }
    void ctcpclient::close()
    {
        if (m_connfd > 0)
        {
            ::close(m_connfd);m_connfd = -1;
        }
    }
    
    bool ctcpserver::initserver(const unsigned int port,const int backlog)
    {
        if (m_listen >= 0) {::close(m_listen); m_listen = -1;}
        
        if ( (m_listen = socket(AF_INET,SOCK_STREAM,0)) <= 0) return false;

        signal(SIGPIPE,SIG_IGN);

        int opt =1;
        setsockopt(m_listen,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

        memset(&m_servaddr,0,sizeof(m_servaddr));
        m_servaddr.sin_family = AF_INET;
        m_servaddr.sin_port = htons(port);
        m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(m_listen,(struct sockaddr*)&m_servaddr,sizeof(m_servaddr)) !=0)
        {
            closelisten(); return false;
        }

        if (listen(m_listen,backlog) != 0)
        {
            closelisten(); return false;
        }

        return true;
    }

    bool ctcpserver::accept()
    {
        if (m_listen == -1) return false;

        int socklen = sizeof(struct sockaddr_in);
        if ( (m_connfd = ::accept(m_listen,(struct sockaddr*)&m_clientaddr,(socklen_t*)&socklen)) < 0)
                return false;
        
        return true;
    }
    char* ctcpserver::getip()
    {
        return (inet_ntoa(m_clientaddr.sin_addr));
    }

    void ctcpserver::closelisten()
    {
        if (m_listen > 0)
        {
            ::close(m_listen);m_listen = -1;
        }
    }

    void ctcpserver::closeclient()
    {
        if (m_connfd > 0)
        {
            ::close(m_connfd);m_connfd = -1;
        }
    }
    bool readn(const int sockfd,char* buffer,const size_t n)
    {
        
        int nleft = n;
        int onread = 0;
        int hread = 0;

        while(nleft > 0)
        {
            if ( (onread = recv(sockfd,buffer + hread,nleft,0)) <= 0) return false;

            hread = hread + onread;
            nleft = nleft - hread;
        }

        return true;
    }
    bool writen(const int sockfd,const char* buffer,const size_t n)
    {
        int nleft =n;
        int onwrite =0;
        int hwrite = 0;

        while ( nleft > 0)
        {
            if ( (onwrite = send(sockfd,buffer + hwrite, nleft,0) <= 0)) return false;

            hwrite = hwrite + onwrite;
            nleft = nleft - hwrite;
        }

        return true;
    }
    bool tcpread(const int sockfd,string& buffer,const int itimeout)
    {
        if (sockfd == -1) return false;

        

        if( itimeout > 0)
        {
            struct pollfd fds;
            fds.fd = sockfd;
            fds.events = POLLIN;
            if ( (poll(&fds,1,itimeout*1000)) <= 0) return false;
        }

        if( itimeout == -1)
        {
            struct pollfd fds;
            fds.fd = sockfd;
            fds.events = POLLIN;
            if ( (poll(&fds,1,0)) <= 0) return false;
        }

        int ibuflen =0;
        if (readn(sockfd,(char*)&ibuflen,4) == false) return false;
        buffer.clear();
        buffer.resize(ibuflen);
        
        if (readn(sockfd,static_cast<char*>(&buffer[0]),ibuflen) == false) return false;

        return true;
    }
    bool tcpread(const int sockfd,void*  buffer,const int ibuflen, const int itimeout)
    {
        if (sockfd == -1) return false;

        if( itimeout > 0)
        {
            struct pollfd fds;
            fds.fd = sockfd;
            fds.events = POLLIN;
            if ( (poll(&fds,1,itimeout*1000)) <= 0) return false;
        }

        if( itimeout == -1)
        {
            struct pollfd fds;
            fds.fd = sockfd;
            fds.events = POLLIN;
            if ( (poll(&fds,1,0)) <= 0) return false;
        }

        memset(buffer,0,sizeof(ibuflen)+1);
        if (readn(sockfd,static_cast<char*>(buffer),ibuflen) == false) return false;

        return true;        
    }
    bool tcpwrite(const int sockfd,const void* buffer, const int ibuflen)
    {   
        if (sockfd == -1) return false;

        if (writen(sockfd,(char*)buffer,ibuflen) == false) return false;

        return true;
    }   
    bool tcpwrite(const int sockfd,const string& buffer)
    {
        if (sockfd == -1) return false;

        int ibuflen = buffer.size();

        if (writen(sockfd,(char*)&ibuflen,4) == false) return false;

        if (writen(sockfd,buffer.c_str(),ibuflen) == false) return false;

         return true;
    }
    bool ctcpclient::read(string& buffer,const int itimeout)
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd,buffer,itimeout));
    }
    bool ctcpserver::read(string& buffer,const int itimeout)
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd,buffer,itimeout));
    }
    bool ctcpclient::read(void* buffer,const int ibuflen,const int itimeout)
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd,buffer,ibuflen,itimeout));
    }
    bool ctcpserver::read(void* buffer,const int ibuflen, const int itimeout)
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd,buffer,ibuflen,itimeout));
    }

    bool ctcpclient::write(const string& buffer)
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd,buffer));
    }
    bool ctcpserver::write(const string& buffer)
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd,buffer));
    }
    bool ctcpclient::write(const void* buffer,const int ibuflen)
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd,buffer,ibuflen));
    }
    bool ctcpserver::write(const void* buffer,const int ibuflen)
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd,buffer,ibuflen));
    }

    bool cesemp::init(const key_t key,const unsigned short value,const short sem_flag)
    {
        if (m_semid != -1) return false;

        m_sem_flag = sem_flag;
        
        if ( (m_semid = semget(key,1,0666)) == -1)
        {
            if (errno == ENOENT)
            {
                if ( (m_semid = semget(key,1,0666|IPC_CREAT|IPC_EXCL)) == -1)
                {
                    if (errno == EEXIST)
                    {
                        if ( (m_semid = semget(key,1,0666)) == -1)
                        {
                            perror("init 1 semget()");return false;
                        }

                    }
                    else
                    {
                        perror("init 2 semget()");return false;
                    }
                }

                union semun sem_union;
                sem_union.val = value;
                if (semctl(m_semid,0,SETVAL,sem_union))
                {
                    perror("init semctl()");return false;
                }
            }
            else
            {
                perror("init 3 semget()");return false;
            }
        }

        return true;
    }   

    bool cesemp::wait(const short value)
    {       
        if (m_semid == -1) return false;

        struct sembuf sem_b;
        sem_b.sem_flg = m_sem_flag;
        sem_b.sem_num = 0;
        sem_b.sem_op = value;
        if (semop(m_semid,&sem_b,1) == -1)
        {
            perror("P semop()");return false;
        }

        return true;
    }

    bool cesemp::post(const short value)
    {       
        if (m_semid == -1) return false;

        struct sembuf sem_b;
        sem_b.sem_flg = m_sem_flag;
        sem_b.sem_num = 0;
        sem_b.sem_op = value;
        if (semop(m_semid,&sem_b,1) == -1)
        {
            perror("V semop()");return false;
        }

        return true;
    }
    
    int cesemp::getvalue()
    {
        return semctl(m_semid,0,GETVAL);
    }

    bool cesemp::destroy()
    {
        if (m_semid == -1) return false;

        if (semctl(m_semid,0,IPC_RMID) == -1) 
        {
            perror("destroy() se,ctl");return false;
        }

        return true;
    }
}