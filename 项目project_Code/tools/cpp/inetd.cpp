#include "_public.h"
using namespace idc;


struct st_route
{
    int srcport;
    char dstip[31];
    int dstport;
    int listensock;
}stroute;

vector<struct st_route> vroute;

#define MAXSOCK 1024
int clientsocks[MAXSOCK];
int clientatime[MAXSOCK];
string clientbuffer[MAXSOCK];

clogfile logfile;

cpactive pactive;


int epollfd = 0;
int tfd =0;

int initserver(const int port);

int conntodst(const char* ip,const int port);
void EXIT(int sig); 

bool loadroute(const char* inifile);
int main(int argc,char *argv[])
{
    if (argc != 3)
    {
        printf("\n");
        printf("Sample:/project/tools/bin/inetd /tmp/inetd.log /etc/inetd.conf\n\n");
        return -1;
    }
    closeioandsignal(true);
    signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

    if (logfile.open(argv[1])==false)
    {
        printf("打开日志文件失败（%s）。\n",argv[1]); return -1;
    }   

    pactive.addpinfo(30,"inetd");

    if (loadroute(argv[2]) == false) return -1;

    logfile.write("加载代理路由参数成功(%d)\n",vroute.size());
    for (auto &aa:vroute)
    {
        if ((aa.listensock = initserver(aa.srcport)) < 0)
        {
            logfile.write("initserver(%d) failed\n",aa.srcport);continue;
        }

        fcntl(aa.listensock,F_SETFL,fcntl(aa.listensock,F_GETFD,0)|O_NONBLOCK);
    }

    epollfd = epoll_create(1);

    struct epoll_event ev;

    for(auto& aa: vroute)
    {
        if (aa.listensock < 0) continue;

        ev.events = EPOLLIN;
        ev.data.fd = aa.listensock;
        epoll_ctl(epollfd,EPOLL_CTL_ADD,aa.listensock,&ev);
    }

    pactive.uptatime();

    //////////////////
    // tfd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);
    // struct itimerspec timeout;
    // memset(&timeout,0,sizeof(struct itimerspec));
    // timeout.it_value.tv_sec = 10;
    // timeout.it_value.tv_nsec = 0;

    // timerfd_settime(tfd,0,&timeout,0);
    // ev.data.fd = tfd;
    // ev.events = EPOLLIN;
    // epoll_ctl(epollfd,EPOLL_CTL_ADD,tfd,&ev);
    // ///////////////////////////////
    // sigset_t sigset;
    // sigemptyset(&sigset);
    // sigaddset(&sigset,SIGINT);
    // sigaddset(&sigset,SIGTERM);
    // sigprocmask(SIG_BLOCK,&sigset,0);
    // int sigfd = signalfd(-1,&sigset,0);
    // ev.data.fd = sigfd;
    // ev.events = EPOLLIN;
    // epoll_ctl(epollfd,EPOLL_CTL_ADD,sigfd,&ev);
    //////////////////

    struct epoll_event evs[10];

    int maxfd = 0;
    for (int ii =0;ii<vroute.size();ii++)
        if ( maxfd < vroute[ii].listensock) 
            maxfd = vroute[ii].listensock;
            
    while(true)
    {
        int infds = epoll_wait(epollfd,evs,10,-1);

        pactive.uptatime();

        if (infds < 0) { logfile.write("epoll() failed\n"); EXIT(-1); }

        for (int ii =0;ii<infds;ii++)
        {
            logfile.write("已发生的事件的socket = %d，事件是：%d\n",evs[ii].data.fd,evs[ii].events);
            //////////////
            // if (evs[ii].data.fd == tfd)
            // {
            //     logfile.write("定时器事件已经到了\n");

            //     timerfd_settime(tfd,0,&timeout,0);

            //     pactive.uptatime();

            //     for (int jj =0;jj<= maxfd;jj++)
            //     {
            //         if ( (clientsocks[jj] > 0) && (time(0) - clientatime[jj] >80))
            //         {
            //             logfile.write("client(%d,%d) timeout \n",clientsocks[jj],clientsocks[clientsocks[jj]]);
            //             close(clientsocks[clientsocks[jj]]);
            //             close(clientsocks[jj]);

            //             clientsocks[clientsocks[jj]] = 0;
            //             clientsocks[jj] = 0;

            //             if (jj == maxfd)
            //                 for (int ss= maxfd;ss>0;ss--)
            //                 {
            //                     if (clientsocks[ss] > 0)
            //                     {
            //                         maxfd = ss ;break;
            //                     }
            //                 }
            //         }
            //     }

            //     continue;
            // }
            ///////////////

            //////////////
            // if (evs[ii].data.fd == sigfd)
            // {
            //     struct signalfd_siginfo siginfo;
            //     int s = read(sigfd,&siginfo,sizeof(struct signalfd_siginfo));

            //     logfile.write("收到了信号=%d\n",siginfo.ssi_signo);


            //     continue;
            // }

            /////////////////////////
            int jj;
            for (jj =0;jj<vroute.size();jj++)
            {
                if (evs[ii].data.fd == vroute[jj].listensock)
                {
                    struct sockaddr_in client;
                    socklen_t len = sizeof(client);

                    int srcsock = accept(vroute[jj].listensock,(struct sockaddr*)&client,&len);
                    if (srcsock < 0) break;
                    if (srcsock >= MAXSOCK) 
                    {
                        logfile.write("连接数已超过最大值%d\n",MAXSOCK);close(srcsock);break;
                    }

                    int dstsock = conntodst(vroute[jj].dstip,vroute[jj].dstport);
                    if (dstsock < 0) { close(srcsock); break; }
                    if (dstsock >= MAXSOCK)
                    {
                        logfile.write("连接数已超过最大值%d\n",MAXSOCK);close(srcsock);break;
                    }

                    logfile.write("accept on port %d,client(%d,%d) ok\n",vroute[jj].srcport,srcsock,dstsock);

                    ev.data.fd =  srcsock ;ev.events = EPOLLIN;
                    epoll_ctl(epollfd,EPOLL_CTL_ADD,srcsock,&ev);
                    ev.data.fd =  dstsock ;ev.events = EPOLLIN;
                    epoll_ctl(epollfd,EPOLL_CTL_ADD,dstsock,&ev);

                    clientsocks[srcsock] = dstsock;clientatime[srcsock] = time(0);
                    clientsocks[dstsock] = srcsock;clientatime[dstsock] = time(0);

                    if (srcsock > maxfd ) maxfd = srcsock;
                    if (dstsock > maxfd ) maxfd = dstsock;

                    pactive.uptatime();

                    break;
                }
            }

            if (jj < vroute.size()) continue;

            if (evs[ii].events & EPOLLIN)
            {
                char buffer[5000];
                memset(buffer,0,sizeof(buffer));
                int buflen = 0;

                if ( (buflen = recv(evs[ii].data.fd,buffer,sizeof(buffer),0)) <= 0)
                {
                    logfile.write("client(%d,%d) disconnected\n",evs[ii].data.fd,clientsocks[evs[ii].data.fd]);

                    close(evs[ii].data.fd);close(clientsocks[evs[ii].data.fd]);
                    clientsocks[clientsocks[evs[ii].data.fd]] = 0;
                    clientsocks[evs[ii].data.fd] = 0;

                    if (evs[ii].data.fd == maxfd)
                        for (int ss= maxfd;ss>0;ss--)
                        {
                            if (clientsocks[ss] > 0)
                            {
                                maxfd = ss ;break;
                            }
                        }

                    pactive.uptatime();
                    continue;
                }

                logfile.write("from %d to %d,%d bytes\n",evs[ii].data.fd,clientsocks[evs[ii].data.fd],buflen);

                //send(clientsocks[evs[ii].data.fd],buffer,buflen,0);

                clientbuffer[clientsocks[evs[ii].data.fd]].append(buffer,buflen);

                ev.data.fd = clientsocks[evs[ii].data.fd];ev.events = EPOLLOUT|EPOLLIN;
                epoll_ctl(epollfd,EPOLL_CTL_MOD,ev.data.fd,&ev);

                clientatime[evs[ii].data.fd] = time(0);
                clientatime[clientsocks[evs[ii].data.fd]] = time(0);
            }

            if (evs[ii].events & EPOLLOUT)
            {
                int writen = send(evs[ii].data.fd,clientbuffer[evs[ii].data.fd].data(),clientbuffer[evs[ii].data.fd].length(),0);

                // int ilen;
                // if (clientbuffer[evs[ii].data.fd].length() > 10) ilen = 10;
                // else ilen = clientbuffer[evs[ii].data.fd].length();
                // int writen = send(evs[ii].data.fd,clientbuffer[evs[ii].data.fd].data(),ilen,0);

                pactive.uptatime();

                logfile.write("to %d %d bytes\n",evs[ii].data.fd,writen);

                clientbuffer[evs[ii].data.fd].erase(0,writen);

                if (clientbuffer[evs[ii].data.fd].length() == 0)
                {
                    ev.data.fd = evs[ii].data.fd;
                    ev.events = EPOLLIN;
                    epoll_ctl(epollfd,EPOLL_CTL_MOD,ev.data.fd,&ev);
                }
            }

        }

    }

}

bool loadroute(const char* inifile)
{
    cifile ifile;
    if (ifile.open(inifile) == false)
    {
        logfile.write("ifile.open(%s) failed\n",inifile);
    }

    string strbuffer;
    ccmdstr cmdstr;

    while(true)
    {
        if (ifile.readline(strbuffer) == false) break;

        auto pos = strbuffer.find("#");
        if (pos == string::npos) break;
        strbuffer.resize(pos);

        replacestr(strbuffer,"  "," ",true);
        deletelrchr(strbuffer,' ');

        cmdstr.splittocmd(strbuffer," ");
        if (cmdstr.size() != 3) continue;

        memset(&stroute,0,sizeof(struct st_route));
        cmdstr.getvalue(0,stroute.srcport);
        cmdstr.getvalue(1,stroute.dstip);
        cmdstr.getvalue(2,stroute.dstport);

        vroute.push_back(stroute);
    }

    return true;
}

int initserver(const int port)
{
    int sock = socket(AF_INET,SOCK_STREAM,0);

    if (sock < 0)
    {
        logfile.write("socket(%d) failed\n",port);return -1;
    }

    int opt = 1;unsigned int len = sizeof(opt);
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,len);

    struct sockaddr_in seraddr;
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(port);

    if (bind(sock,(struct sockaddr*)&seraddr,sizeof(seraddr)) < 0)
    {
        logfile.write("bind(%d) failed\n",port);return -1;
    }

    if (listen(sock,5) != 0)
    {
        logfile.write("listen(%d) failed\n",port);return -1;
    }

    return sock;
}

int conntodst(const char* ip,const int port)
{
    int sockfd;
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) return -1;

    struct hostent* h;
    if ((h = gethostbyname(ip)) == 0)
    {
        close(sockfd);return -1;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    memcpy(&servaddr.sin_addr,h->h_addr,h->h_length);

    fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFD,0)|O_NONBLOCK);

    if (connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            logfile.write("connect(%s,%d) %d failed\n",ip,port);
            //perror("connect");
            return -1;
        }
    }

    return sockfd;
}

void EXIT(int sig)
{
    logfile.write("程序退出，sig=%d。\n\n",sig);

    // 关闭全部监听的socket。
    for (auto &aa:vroute)
        if (aa.listensock>0) close(aa.listensock);

    // 关闭全部客户端的socket。
    for (auto aa:clientsocks)
        if (aa>0) close(aa);

    close(epollfd);   // 关闭epoll。

    exit(0);
}