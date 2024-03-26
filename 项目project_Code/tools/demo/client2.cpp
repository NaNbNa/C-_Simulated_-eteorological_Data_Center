#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("    ./client2 192.168.240.131 5088\n"); return -1;
    }

    int sockfd;
    
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0))<0) { printf("socket() failed.\n"); return -1; }

	struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr=inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) != 0)
    {
        printf("connect(%s:%s) failed.\n",argv[1],argv[2]); close(sockfd);  return -1;
    }

    printf("connect ok.\n");

    char buf[1024];
    memset(buf,0,sizeof(buf));
    strcpy(buf,"aaaaa");

    if (send(sockfd,buf,strlen(buf),0) <=0)
    { 
        printf("write() failed.\n");  close(sockfd);  return -1;
    }

    sleep(10);

    memset(buf,0,sizeof(buf));
    strcpy(buf,"bbbbbb\r\n\r\n");
    if (send(sockfd,buf,strlen(buf),0) <= 0)
    {
        printf("write() failed\n");close(sockfd);return -1;
    }

    sleep(5);

    memset(buf,0,sizeof(buf));
    strcpy(buf,"cccccccc\r\n\r\n");

    if (send(sockfd,buf,strlen(buf),0) <= 0)
    {
        printf("write() failed\n");close(sockfd);return -1;
    }

    memset(buf,0,sizeof(buf));

    if (recv(sockfd,buf,sizeof(buf),0) <= 0)
    {
        printf("read() failed\n");close(sockfd);return -1;
    }

    printf("recv:%s\n");

}