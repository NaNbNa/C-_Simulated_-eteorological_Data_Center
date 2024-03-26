#include "_public.h"
using namespace idc;

#define MAXSOCK 1024
int clientsocks[MAXSOCK];
int clientatime[MAXSOCK];
string clientbuffer[MAXSOCK];

clogfile logfile;

cpactive pactive;

int epollfd = 0;
int tfd;

int cmdconnsock =0;

int conntodst(const char* ip,const int port,bool bio = false);
void EXIT(int sig); 

int main(int argc,char *argv[])
{
    if (argc != 4)
    {
        printf("\n");
        printf("    /project/tools/bin/rinetdin /tmp/rinetdin.log 192.168.240.131 5001\n\n");
        return -1;
    }

    closeioandsignal(true);
    signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

    if (logfile.open(argv[1])==false)
    {
        printf("打开日志文件失败(%s)。\n",argv[1]); return -1;
    }   

    if (( cmdconnsock = conntodst(argv[2],atoi(argv[3]),true) ) < 0)
    {
        logfile.write("tcpclient(%s,%s) connect failed\n",argv[2],argv[3]);return -1;
    }

    logfile.write("与外部的命令通道已经建立(%d)\n",cmdconnsock);

    fcntl(cmdconnsock,F_SETFL,fcntl(cmdconnsock,F_GETFD,0)|O_NONBLOCK);

    epollfd = epoll_create(1);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = cmdconnsock;

    epoll_ctl(epollfd,EPOLL_CTL_ADD,cmdconnsock,&ev);

    struct epoll_event evs[10];

    while(true)
    {
        int infds = epoll_wait(epollfd,evs,10,-1);

        if (infds < 0) { logfile.write("epoll() failed\n"); EXIT(-1); }

        for (int ii=0;ii<infds; ii ++)
        {
            if (evs[ii].data.fd == cmdconnsock)
            {
                char buffer[256];
                memset(buffer,0,sizeof(buffer));

                if (recv(cmdconnsock,buffer,sizeof(buffer),0) <= 0)
                {
                    logfile.write("与外网的命令通道已经断开(%d)\n",cmdconnsock);EXIT(-1);
                }

                if (strcmp(buffer,"<activetest") == 0) continue;

                int srcsock = conntodst(argv[2],atoi(argv[3]));
                if (srcsock < 0) continue;
                if (srcsock >= MAXSOCK) 
                {
                    logfile.write("连接数已超过最大值%d\n",MAXSOCK);close(srcsock);continue;
                }

                char dstip[11];
                int  dstport;
                getxmlbuffer(buffer,"dstip",dstip,30);
                getxmlbuffer(buffer,"dstport",dstport);

                int dstsock = conntodst(dstip,dstport);
                if (dstsock < 0) { close(srcsock);continue;}
                if (dstsock >= MAXSOCK) 
                {
                    logfile.write("连接数已超过最大值%d\n",MAXSOCK);close(dstsock);close(srcsock);continue;
                }

                logfile.write("外网--服务端内网连接通道(%d,%d) ok\n",srcsock,dstsock);

                ev.data.fd =  srcsock ;ev.events = EPOLLIN;
                epoll_ctl(epollfd,EPOLL_CTL_ADD,srcsock,&ev);
                ev.data.fd =  dstsock ;ev.events = EPOLLIN;
                epoll_ctl(epollfd,EPOLL_CTL_ADD,dstsock,&ev);

                clientsocks[srcsock] = dstsock;     clientatime[srcsock] = time(0);
                clientsocks[dstsock] = srcsock;     clientatime[dstsock] = time(0);

                continue;
            }

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

                logfile.write("读事件成功，下一步：from %d to %d,%d bytes\n",evs[ii].data.fd,clientsocks[evs[ii].data.fd],buflen);

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

int conntodst(const char* ip,const int port,bool bio)
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

    if (bio == false)
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
    // for (auto &aa:vroute)
    //     if (aa.listensock>0) close(aa.listensock);

    // 关闭全部客户端的socket。
    for (auto aa:clientsocks)
        if (aa>0) close(aa);

    close(epollfd);   // 关闭epoll。

    exit(0);
}