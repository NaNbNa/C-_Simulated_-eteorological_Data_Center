#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <cerrno>

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

    while(true)
    {
        printf("accpet() ...\n");
        if (accept(listenfd,0,0) == -1)
        {
            if (errno != EAGAIN)
            {  
                perror("accept:");return -1;
            }
        }
        else
            break;

        sleep(1);
    }

    printf("客户端已连接\n");

    return 0;

    pollfd fds[1024];

    for ( int ii = 0;ii<1024; ii ++)
    {
        fds[ii].fd = -1;
    }

    fds[listenfd].fd = listenfd;
    fds[listenfd].events = POLLIN;

    int maxfd = listenfd;

    while(true)
    {
        int infds = poll(fds,maxfd + 1,10000);

        if ( infds < 0)
        {
            printf("poll 失败\n");break;
        }

        if (infds == 0)
        {
            printf("poll 超时\n");continue;
        }

        for (int fd =0;fd <= maxfd;fd++)
        {
            if (fds[fd].fd < 0) continue;

            if ((fds[fd].revents & POLLIN) == 0) continue;

            if (fd == listenfd)
            {
                struct sockaddr_in servaddr;
                socklen_t len = sizeof(servaddr);

                int clientfd = accept(listenfd,(struct sockaddr*)&servaddr,&len);
                if (clientfd < 0) { perror("accept() failed\n"); continue; }

                printf("accept client(socket = %d) ok \n",clientfd);

                fds[clientfd].fd = clientfd;
                fds[clientfd].events = POLLIN;

                if (maxfd < clientfd ) maxfd = clientfd;
            }
            else
            {
                char buffer[1024];
                memset(buffer,0,sizeof(buffer));

                if (recv (fd,buffer,sizeof(buffer),0) < 0) 
                {
                    printf("客户端 (clientfd = %d) 断开连接\n",fd);
                    close(fd);fds[fd].fd = -1;

                    if (fd == maxfd)
                    {
                        for (int ii=maxfd;ii>0;ii--)
                        {
                            if (fds[ii].fd != -1)
                            {
                                maxfd = ii;break;
                            }
                        }
                    }
                }
                else
                {
                    printf("recv(eventfd=%d):%s\n",fd,buffer);
                    // printf("aaaaa\n");
                    // if (recv (fd,buffer,sizeof(buffer),0) < 0)
                    //     printf("recv ok\n");
                    // printf("aaaaa\n");
                    //sleep(1);
                    send(fd,buffer,strlen(buffer),0);
                }

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