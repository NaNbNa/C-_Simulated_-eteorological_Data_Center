#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>          
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>

int initserver(int port);

int setnonblock(int fd)
{
    int flag;

    if ((flag = fcntl(fd,F_GETFL,0)) == -1)
        flag = 0;
    
    return fcntl(fd,F_SETFL,flag|O_NONBLOCK);
}

int main(int argc,char *argv[])
{
    if (argc != 2) { printf("usage: ./tcpepoll port\n"); return -1; }

    int listenfd = initserver(atoi(argv[1]));
    printf("listefd = %d\n",listenfd);

    if (listenfd < 0) { printf("initserver failed\n"); return -1;}

    setnonblock(listenfd);

    int epollfd = epoll_create(1);

    epoll_event ev;
    ev.data.fd = listenfd;
    //ev.events = EPOLLIN;
    ev.events = EPOLLIN | EPOLLET;
    
    epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&ev);

    epoll_event evs[10];

    while(true)
    {
        int infds = epoll_wait(epollfd,evs,10,-1);

        if (infds < 0)
        {
            perror("epoll_wait() failed\n");break;
        }

        if (infds == 0)
        {
            printf("epoll timeout\n");continue;
        }

        for (int ii=0;ii < infds;ii++)
        {
            if (evs[ii].data.fd == listenfd)
            {
                // printf("监听的事件发生\n");
                // break;
                while(true)
                {
                    struct sockaddr_in client;
                    socklen_t len = sizeof(client);

                    int clientfd = accept(listenfd,(struct sockaddr*)&client,&len);
                    if ((clientfd < 0) && (errno == EAGAIN)) { printf("缓存队列为空\n"); break; }

                    printf("accept client(%d) \n",clientfd);
                    static int ii =0;
                    printf("这是第%d个连接\n",ii++);

                    ev.data.fd = clientfd;
                    // ev.events = EPOLLIN | EPOLLET;
                    ev.events = EPOLLIN;
                    //ev.events = EPOLLOUT | EPOLLET;
                    //ev.events = EPOLLOUT;
                    setnonblock(clientfd);

                    epoll_ctl(epollfd,EPOLL_CTL_ADD,clientfd,&ev);
                }
            }

            if (evs[ii].events & EPOLLOUT)
            {
                
                printf("触发写事件\n");
                for (int ii =0;ii<10000000;ii++)
                {
                    if (send(ev.data.fd,"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbb",49,0) <= 0 )
                    {
                        if (errno == EAGAIN)
                        {
                            printf("发送缓冲区已填满\n");break;
                        }
                    }
                }
            }

            if (evs[ii].events & EPOLLIN)
            {
                //printf("触发读事件\n");
                // char buffer[1024];
                // memset(buffer,0,sizeof(buffer));

                // int readn;char* ptr = buffer;

                // while(true)
                // {
                //     if ((readn = recv(evs[ii].data.fd,ptr,5,0)) <= 0)
                //     {
                //         if ((readn < 0) && (errno == EAGAIN))
                //         {
                //             send(evs[ii].data.fd,buffer,sizeof(buffer),0);
                //             printf("recv(client=%d):%s\n",evs[ii].data.fd,buffer);
                //         }
                //         else
                //         {
                //             printf("client(eventfd=%d) disconnect\n",evs[ii].data.fd);
                //             close(evs[ii].data.fd);
                //         }

                //         break;
                //         //epoll_ctl(epollfd,EPOLL_CTL_DEL,evs[ii].data.fd,0);
                //     }
                //     else
                //     {
                //         ptr = ptr + readn;
                //     }
                // }      
            }

        }
    }

}

int initserver(int port)
{
    int sock = socket(AF_INET,SOCK_STREAM,0);

    if (sock < 0)
    {
        perror("socket() failed\n");return -1;
    }

    int opt = 1;unsigned int len = sizeof(opt);
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,len);

    struct sockaddr_in seraddr;
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(port);

    if (bind(sock,(struct sockaddr*)&seraddr,sizeof(seraddr)) < 0)
    {
        perror("bind() failed");return -1;
    }

    if (listen(sock,5) != 0)
    {
        perror("listen() failed");return -1;
    }

    return sock;
}