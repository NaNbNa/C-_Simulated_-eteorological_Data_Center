#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include "sys/poll.h"

int setnonblock(int fd)
{
    int flag;

    if ((flag = fcntl(fd,F_GETFL,0)) == -1)
        flag = 0;
    
    return fcntl(fd,F_SETFL,flag|O_NONBLOCK);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage:./client 192.168.240.131 5008\n"); return -1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    char buf[1024];
    memset(buf,0,sizeof(buf));

    if ( (sockfd = (socket(AF_INET,SOCK_STREAM,0))) <= 0)
    {
        perror("socket() failed \n");return -1;
    }

    printf("sockfd = %d\n",sockfd);

    //setnonblock(sockfd);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    if ( (connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) != 0)
    {
        // if (errno != EINPROGRESS)
        // {    
            printf("connect(%s,%s) failed\n",argv[1],argv[2]);close(sockfd);
            return -1;
        // }
    }

    printf("connect(%s,%s) ok\n",argv[1],argv[2]);

    // pollfd fds;
    // fds.fd =sockfd;
    // fds.events = POLLOUT;
    // poll(&fds,1,-1);
    // if (fds.revents == POLLOUT)
    //      printf("connect(%s,%s) ok\n",argv[1],argv[2]);
    // else
    //      printf("connect(%s,%s) failed\n",argv[1],argv[2]);

    //close(sockfd);

    //printf("开始时间：%lu\n",time(0));

    for (int ii=0;ii<200000;ii++)
    {
        memset(buf,0,sizeof(buf));

        //strcpy(buf,"aaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaaaaaaaaaaa");

        printf("input:");scanf("%s",buf);
        
        if (send(sockfd,buf,strlen(buf),0) <= 0)
        {
            printf("write() failed\n");close(sockfd);return -1;
        }

 
        memset(buf,0,sizeof(buf));
        if (recv(sockfd,buf,sizeof(buf),0) <= 0)
        {
            printf("read() failed\n");close(sockfd);return -1;
        }

        printf("recv: %s\n\n",buf);
        usleep(1000);
    }

    //printf("结束时间：%lu",time(0));
    
}
