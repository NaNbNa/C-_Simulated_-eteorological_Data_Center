#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>

int initserver(int port);

int main(int argc,char *argv[])
{
    if (argc != 2) { printf("usage: ./tcpselect 5008\n"); return -1; }

    int listenfd;
    if ( (listenfd = initserver(atoi(argv[1]))) < 0)
    {
        printf("initserver(%s) failed\n",argv[1]);return -1;
    }

    printf("listenfd = %d\n",listenfd);

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listenfd,&readfds);

    int maxfd = listenfd;
    
    int bb =0;
    while(true)
    {
        struct timeval timeout;
        timeout.tv_sec =10;
        timeout.tv_usec = 0;

        fd_set tmpfds = readfds;
        fd_set tmpfds1 = readfds;

        int infds = select(maxfd + 1,&tmpfds,&tmpfds1,NULL,0); 

        // if (bb ==0)
        // {
        //     sleep(5);bb = 1;continue;
        // }

        if (infds < 0)
        {
            perror("select failed");break;
        }

        // if (infds == 0)
        // {
        //     printf("select() timeout\n");continue;
        // }

        // for (int eventfd =0;eventfd <= maxfd;eventfd++)
        // {
        //     if (FD_ISSET(eventfd,&tmpfds1) == 0) continue;

        //     printf("eventfd =%d 可以写\n",eventfd);
        // }

        for (int eventfd=0;eventfd<=maxfd;eventfd++)
        {
            if (FD_ISSET(eventfd,&tmpfds) == 0) continue;

            if (eventfd == listenfd) 
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientsock = accept(listenfd,(struct sockaddr*)&client,&len);

                if (clientsock < 0) { perror("accept() failed"); continue; } 

                printf("accept client(socket=%d) ok\n",clientsock);

                FD_SET(clientsock,&readfds);

                if (maxfd < clientsock) maxfd = clientsock;
                
                continue;
            }
            
            char buffer[1024];
            memset(buffer,0,sizeof(buffer));
                
            if (recv(eventfd,buffer,sizeof(buffer),0) <= 0)
            //if (recv(eventfd,buffer,2,0) <= 0)
            {
                printf("client(eventfd = %d) disconnected\n",eventfd);
                close(eventfd);FD_CLR(eventfd,&readfds);

                if (eventfd == maxfd)
                {
                    for (int ii =maxfd;ii>0;ii--)
                    {
                        if (FD_ISSET(ii,&readfds))
                        {
                            maxfd = ii;break;
                        }
                    }
                }

                continue;
            }
            
            printf("recv(eventfd =%d):%s\n",eventfd,buffer);

            send(eventfd,buffer,strlen(buffer),0);
   
        }
    }

    return 0;
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