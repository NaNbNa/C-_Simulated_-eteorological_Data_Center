#include "_public.h"
#include "_ooci.h"

using namespace idc;

clogfile logfile;  

struct st_client
{
    int clientatime = 0;
    string recvbuffer;
    string sendbuffer;
};

struct st_recvmesg
{
    int sock = 0;
    string message;

    st_recvmesg(int in_sock,string in_message):sock(in_sock),message(in_message){ logfile.write("构造了报文\n"); }
};

class AA
{
private:
    queue<shared_ptr<st_recvmesg>> m_rq;
    mutex m_mutex_rq;
    condition_variable m_cond_rq;

    queue<shared_ptr<st_recvmesg>> m_sq;
    mutex m_mutex_sq;

    int m_sendpipe[2] = {0};                            

    unordered_map<int,struct st_client> clientmap;

    atomic_bool m_exit;
public:
    int m_recvpipe[2] = {0};
    AA()
    {
        pipe(m_sendpipe);
        pipe(m_recvpipe);
        m_exit = false;
    }

    void recvfunc(int listenport);

    void sendfunc();

    void inrq(const int sock,const string& message);
    void insq(const int sock,const string& message);

    void workfunc(int id);

    void bizmain(connection& conn1,const string& recvbuf,string& sendbuf);

};

void EXIT(int sig); 

bool getvalue(const string &strget,const string &name,string &value);

int initserver(const int port);

AA aa;

int main(int argc,char *argv[])
{
    if (argc != 3)
    {
        printf("\n");
        printf("        /project/tools/bin/webserver /log/idc/webserver.log 5088\n\n");
        return -1;
    }

    closeioandsignal();  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

    if (logfile.open(argv[1])==false)
    {
        printf("打开日志文件失败（%s）。\n",argv[1]); return -1;
    }

    thread t1(&AA::recvfunc,&aa,atoi(argv[2]));
    thread t2(&AA::workfunc,&aa,1);
    thread t3(&AA::workfunc,&aa,2);
    thread t4(&AA::workfunc,&aa,3);
    thread t5(&AA::sendfunc,&aa);

    

    logfile.write("已启动所有线程\n");

    while (true)
    {
        sleep(30);

        
    }
    return 0;
}

int initserver(const int port)
{
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock < 0)
    {
        logfile.write("socket(%d) failed.\n",port); return -1;
    }

    int opt = 1; unsigned int len = sizeof(opt);
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,len);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sock,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
    {
        logfile.write("bind(%d) failed.\n",port); close(sock); return -1;
    }

    if (listen(sock,5) != 0 )
    {
        logfile.write("listen(%d) failed.\n",port); close(sock); return -1;
    }

    fcntl(sock,F_SETFL,fcntl(sock,F_GETFD,0)|O_NONBLOCK);  

    return sock;
}

void EXIT(int sig)
{
    signal(sig,SIG_IGN);

    logfile.write("程序退出，sig=%d。\n\n",sig);

    write(aa.m_recvpipe[1],(char*)"o",1);   

    usleep(500);    

    exit(0);
}

bool getvalue(const string &strget,const string &name,string &value)
{
    int startp=strget.find(name);                                         
    if (startp==string::npos) return false; 

    int endp=strget.find("&",startp);                                 
    if (endp==string::npos) endp=strget.find(" ",startp);    

    if (endp==string::npos) return false;

    value=strget.substr(startp+(name.length()+1),endp-startp-(name.length()+1));

    return true;
}


void AA::recvfunc(int listenport)
{
    int listensock = initserver(listenport);
    if (listensock < 0)
    {
        logfile.write("接收线程：initserver(%d) failed\n",listensock);return;
    }

    int epollfd = epoll_create1(0);

    struct epoll_event ev;
    ev.data.fd= listensock;
    ev.events = EPOLLIN;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev);
 
    ev.data.fd = m_recvpipe[0];
    ev.events = EPOLLIN;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev);

    struct epoll_event evs[10];

    while(true)
    {
        int infds = epoll_wait(epollfd,evs,10,-1);

        if (infds < 0) { logfile.write("接收线程：epoll_wait() failed\n"); return; }

        for (int ii =0;ii < infds;ii++)
        {
            logfile.write("接收线程：已发生事件的fd=%d(%d)\n",evs[ii].data.fd,evs[ii].events);

            if (evs[ii].data.fd == listensock)
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
                
                logfile.write("接收线程：accept client(%d) ok\n",clientsock);

                ev.data.fd = clientsock;
                ev.events = EPOLLIN;
                epoll_ctl(epollfd,EPOLL_CTL_ADD,clientsock,&ev);

                continue;
            }
            
            if (evs[ii].data.fd == m_recvpipe[0])
            {
                logfile.write("接收线程(%d)：即将退出\n",evs[ii].data.fd);

                m_exit = true;

                m_cond_rq.notify_all();

                write(m_sendpipe[1],(char*)"o",1);

                return;
            }

            if (evs[ii].events & EPOLLIN)
            {
                char buffer[5000];memset(buffer,0,sizeof(buffer));
                int buflen = 0;

                if ( (buflen = recv(evs[ii].data.fd,buffer,sizeof(buffer),0)) <= 0)
                {
                    logfile.write("接收线程：client(%d) 已断开\n",evs[ii].data.fd);

                    close(evs[ii].data.fd);
                    clientmap.erase(evs[ii].data.fd);

                    continue;
                }

                logfile.write("接收线程：recv %d , %d bytes\n",evs[ii].data.fd,buflen);

                clientmap[evs[ii].data.fd].recvbuffer.append(buffer,buflen);

                if (clientmap[evs[ii].data.fd].recvbuffer.compare(clientmap[evs[ii].data.fd].recvbuffer.length() -4,4,"\r\n\r\n") == 0)
                {
                    logfile.write("接收线程(%d)：接收到了一个完整的报文\n",evs[ii].data.fd);

                    inrq(evs[ii].data.fd,clientmap[evs[ii].data.fd].recvbuffer);

                    clientmap[evs[ii].data.fd].recvbuffer.clear();
                }
                else
                {
                    if (clientmap[evs[ii].data.fd].recvbuffer.length() > 1000) 
                    {
                        close(evs[ii].data.fd);
                        clientmap.erase(evs[ii].data.fd);
                    }
                }

                clientmap[evs[ii].data.fd].clientatime = time(0);
            }

        }
    }

}

void AA::sendfunc()
{
    int epollfd = epoll_create1(0);
    struct epoll_event ev;

    ev.data.fd = m_sendpipe[0];
    ev.events = EPOLLIN;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev);

    struct epoll_event evs[10];

    while(true)
    {
        int infds = epoll_wait(epollfd,evs,10,-1);

        if (infds < 0) { logfile.write("发送线程：epoll_wait() failed\n"); return;}

        for (int ii = 0;ii< infds;ii++)
        {

            logfile.write("发送线程：已发生事件的fd=%d(%d)\n",evs[ii].data.fd,evs[ii].events);

            if (evs[ii].data.fd == m_sendpipe[0])
            {
                if (m_exit == true)
                {
                    logfile.write("发送线程(%d)：即将退出\n",evs[ii].data.fd);

                    return;
                }

                char cc;
                read(m_sendpipe[0],&cc,1);

                shared_ptr<st_recvmesg> ptr;

                lock_guard<mutex> lock(m_mutex_sq);
                
                while(m_sq.empty() == false)
                {
                    ptr = m_sq.front();m_sq.pop();

                    clientmap[ptr->sock].sendbuffer.append(ptr->message);

                    ev.data.fd = ptr->sock;
                    ev.events = EPOLLOUT;
                    epoll_ctl(epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev);
                }

                continue;
            }

            if (evs[ii].events & EPOLLOUT)
            {
                int writen = send(evs[ii].data.fd,clientmap[evs[ii].data.fd].sendbuffer.data(),clientmap[evs[ii].data.fd].sendbuffer.length(),0);

                logfile.write("发送线程:向%d发送了%d的字节\n",evs[ii].data.fd,writen);

                clientmap[evs[ii].data.fd].sendbuffer.erase(0,writen);

                if (clientmap[evs[ii].data.fd].sendbuffer.length() == 0)
                {
                    ev.data.fd = evs[ii].data.fd;
                    epoll_ctl(epollfd,EPOLL_CTL_DEL,ev.data.fd,&ev);
                }

                continue;
            }
        }
    }
}
void AA::inrq(const int sock,const string& message)
{
    shared_ptr<st_recvmesg> ptr = make_shared<st_recvmesg>(sock,message);

    lock_guard<mutex> lock(m_mutex_rq);

    m_rq.push(ptr);

    m_cond_rq.notify_one();
}

void AA::insq(const int sock,const string& message)
{
    {
        shared_ptr<st_recvmesg> ptr = make_shared<st_recvmesg>(sock,message);

        lock_guard<mutex> lock(m_mutex_sq);

        m_sq.push(ptr);
    }

    write(m_sendpipe[1],(char*)"o",1);
}

void AA::workfunc(int id)
{
    connection conn;
    if (conn.connecttodb("idc/idcpwd@snorcl11g_131","Simplified Chinese_China.AL32UTF8") != 0)
    {
        logfile.write("connect database(idc/idcpwd@snorcl11g_131) failed\n%s\n",conn.message());return;
    }


    while(true)
    {
        shared_ptr<st_recvmesg> ptr;

        {
            unique_lock<mutex> lock(m_mutex_rq);

            while(m_rq.empty() == true)
            {
                m_cond_rq.wait(lock);

                if (m_exit == true)
                {
                    logfile.write("工作线程：(%d):即将退出\n",id);return;
                }
            }

            ptr = m_rq.front();m_rq.pop();

        }

        string sendbuf;
        bizmain(conn,ptr->message,sendbuf);
        string message = sformat(
            "HTTP/1.1 200 ok\r\n"
            "Server:webserver\r\n"
            "Content-Type:text/html;charset=utf-8\r\n") + sformat("Content-Length:%d\r\n\r\n",sendbuf.size()) + sendbuf;


        logfile.write("工作线程（%d）：sock=%d,mesg=%s\n\n",id,ptr->sock,ptr->message.c_str());  


        insq(ptr->sock,ptr->message);
    }
}

void AA::bizmain(connection& conn1,const string& recvbuf,string& sendbuf)
{
    logfile.write("----------开始处理html报文----------\n");

    string username,passwd,intername;

    getvalue(recvbuf,"username",username);
    getvalue(recvbuf,"passwd",passwd);
    getvalue(recvbuf,"intername",intername);

    sqlstatement stmt;stmt.connect(&conn1);

    stmt.prepare("select ip from T_USERINFO where username = :1 and passwd = :2 and rsts = 1");
    string ip;
    stmt.bindin(1,username);
    stmt.bindin(2,passwd);
    stmt.bindout(1,ip,50);
    stmt.execute();
    if ( stmt.next() != 0)
    {
        sendbuf = "<retcode>-1</retcode><message>用户名或密码不正确。</message>";return;
    }

    if (ip.empty() == false)
    {

    }

    stmt.prepare("select count(*) from T_USERANDINTER " \
                " where username=:1 and intername=:2 and intername in (select intername from T_INTERCFG where rsts =1) ");
    stmt.bindin(1,username);
    stmt.bindin(2,intername);
    int icount=0;
    stmt.bindout(1,icount);
    stmt.execute();
    stmt.next();
    if (icount==0)
    {
        sendbuf="<retcode>-1</retcode><message>用户无权限，或接口不存在。</message>"; return;
    }

    // SQL语句：   select obtid,to_char(ddatetime,'yyyymmddhh24miss'),t,p,u,wd,wf,r,vis from T_ZHOBTMIND 
    //                     where obtid=:1 and ddatetime>=to_date(:2,'yyyymmddhh24miss') and ddatetime<=to_date(:3,'yyyymmddhh24miss')
    
    string selectsql,colstr,bindin; 
    stmt.prepare("select selectsql,colstr,bindin from T_INTERCFG where intername=:1");
    stmt.bindin(1,intername);          
    stmt.bindout(1,selectsql,1000);  
    stmt.bindout(2,colstr,300);        
    stmt.bindout(3,bindin,300);      
    stmt.execute();                         
    if (stmt.next()!=0)
    {
        sendbuf="<retcode>-1</retcode><message>内部错误。</message>"; return;
    }

    stmt.prepare(selectsql);

    ccmdstr cmdstr;
    cmdstr.splittocmd(bindin,",");

    vector<string> invalue;
    invalue.resize(cmdstr.size());

    for (int ii=0;ii<cmdstr.size();ii++)
    {
        getvalue(recvbuf,cmdstr[ii].c_str(),invalue[ii]);
        stmt.bindin(ii+1,invalue[ii]);
    }

    cmdstr.splittocmd(colstr,",");

    vector<string> colvalue;
    colvalue.resize(cmdstr.size());

    for (int ii=0;ii<cmdstr.size();ii++)
        stmt.bindout(ii+1,colvalue[ii]);


    if (stmt.execute() != 0)
    {
        logfile.write("stmt.execute() failed.\n%s\n%s\n",stmt.sql(),stmt.message()); 
        sformat(sendbuf,"<retcode>%d</retcode><message>%s</message>\n",stmt.rc(),stmt.message());
        return;
    }

    sendbuf="<retcode>0</retcode><message>ok</message>\n";

    sendbuf=sendbuf+"<data>\n";           

    while (true)
    {
        if (stmt.next() != 0) break;          

        for (int ii=0;ii<cmdstr.size();ii++)
            sendbuf=sendbuf+sformat("<%s>%s</%s>",cmdstr[ii].c_str(),colvalue[ii].c_str(),cmdstr[ii].c_str());

        sendbuf=sendbuf+"<endl/>\n";   

    }       


    sendbuf=sendbuf+"</data>\n";        

    logfile.write("intername=%s,count=%d\n",intername.c_str(),stmt.rpc());

    // 写接口调用日志表T_USERLOG
    logfile.write("----------html报文处理成功，返回应答报文----------\n");
    return;
}
