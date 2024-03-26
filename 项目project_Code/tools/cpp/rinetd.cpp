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
int tfd;

int cmdlistensock =0;
int cmdconnsock =0;

int initserver(const int port);

void EXIT(int sig); 
bool loadroute(const char* inifile);

int main(int argc,char *argv[])
{
    if (argc != 4)
    {
        printf("\n");
        printf("    /project/tools/bin/rinetd /tmp/rinetd.log /etc/rinetd.conf 5001\n\n");
        return -1;
    }

    closeioandsignal(true);
    signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

    if (logfile.open(argv[1]) == false)
    {
        printf("打开日志文件失败（%s）。\n",argv[1]); EXIT(-1);
    }   

    //pactive.addpinfo(30,"inetd");

    if (loadroute(argv[2]) == false) return -1;

    logfile.write("加载代理路由参数成功(%d)\n",vroute.size());

    if ((cmdlistensock = initserver(atoi(argv[3])))<  0)
    {
        logfile.write("server(%s) initserver failed\n",argv[3]);EXIT(-1);
    }

    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    cmdconnsock = accept(cmdlistensock,(struct sockaddr*)&client,&len);
    if (cmdconnsock < 0)
    {
        logfile.write("accept() failed\n");EXIT(-1);
    }
    logfile.write("与内部的命令通道已经建立(%d)\n",cmdconnsock);

    for (int ii = 0;ii< vroute.size();ii++)
    {
        if ((vroute[ii].listensock = initserver(vroute[ii].srcport)) < 0)
        {
            logfile.write("initserver(%d) failed\n",vroute[ii].srcport);continue;
        }

        fcntl(vroute[ii].listensock,F_SETFL,fcntl(vroute[ii].listensock,F_GETFD,0)|O_NONBLOCK);
    }

    epollfd = epoll_create(1);

    struct epoll_event ev;

    for (int ii =0;ii< vroute.size();ii++)
    {
        ev.data.fd = vroute[ii].listensock;
        ev.events = EPOLLIN;
        epoll_ctl(epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev);
    }

    struct epoll_event evs[10];

    while(true)
    {
        int infds = epoll_wait(epollfd,evs,10,-1);

        if (infds < 0) { logfile.write("epoll() failed\n"); EXIT(-1); }

        for (int ii=0;ii<infds; ii ++)
        {
            
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

                    char buffer[256];
                    memset(buffer,0,sizeof(buffer));
                    sprintf(buffer,"<dstip>%s</dstip><dstport>%d</dstport>",vroute[jj].dstip,vroute[jj].dstport);
                    if (send(cmdconnsock,buffer,strlen(buffer),0) <= 0)
                    {
                        logfile.write("与内网的命令通道已断开(%d)\n",cmdconnsock);EXIT(-1);
                    }

                    int dstsock = accept(cmdlistensock,(struct  sockaddr*)&client,&len);
                    if (dstsock < 0) { close(srcsock); break; }
                    if (dstsock >= MAXSOCK)
                    {
                        logfile.write("连接数已超过最大值%d\n",MAXSOCK);close(dstsock);close(srcsock);break;
                    }

                    ev.data.fd =  srcsock ;ev.events = EPOLLIN;
                    epoll_ctl(epollfd,EPOLL_CTL_ADD,srcsock,&ev);
                    ev.data.fd =  dstsock ;ev.events = EPOLLIN;
                    epoll_ctl(epollfd,EPOLL_CTL_ADD,dstsock,&ev);

                    clientsocks[srcsock] = dstsock;     clientatime[srcsock] = time(0);
                    clientsocks[dstsock] = srcsock;     clientatime[dstsock] = time(0);

                    //pactive.uptatime();

                    logfile.write("客户端--外网连接通道(%d,%d) ok\n",srcsock,dstsock);

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
    
                    //pactive.uptatime();
                    continue;
                }

                logfile.write("读事件成功：from %d to %d,%d bytes\n",evs[ii].data.fd,clientsocks[evs[ii].data.fd],buflen);

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

                //pactive.uptatime();

                logfile.write("写事件开始：to %d %d bytes\n",evs[ii].data.fd,writen);

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

void EXIT(int sig)
{
    logfile.write("程序退出，sig=%d。\n\n",sig);

    if (cmdlistensock > 0) close(cmdlistensock);
    if (cmdconnsock > 0) close(cmdconnsock);

    // 关闭全部监听的socket。
    for (auto &aa:vroute)
        if (aa.listensock>0) close(aa.listensock);

    // 关闭全部客户端的socket。
    for (auto aa:clientsocks)
        if (aa>0) close(aa);

    close(epollfd);   // 关闭epoll。

    exit(0);
}