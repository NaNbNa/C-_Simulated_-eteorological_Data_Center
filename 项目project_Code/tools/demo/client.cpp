#include "_public.h"

using namespace idc;

int main(int argc,char *argv[])
{
    if (argc != 3)
    {
        cout << "   ./client www.weather.com.cn 80\n\n"; return -1;
    }

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd==-1)
    {
      perror("socket"); return -1;
    }
 
    struct hostent* h;   
    if ( (h = gethostbyname(argv[1])) == 0 )  
    { 
        cout << "gethostbyname failed.\n" << endl; close(sockfd); return -1;
    }
    struct sockaddr_in servaddr;           
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy(&servaddr.sin_addr,h->h_addr,h->h_length); 
    servaddr.sin_port = htons(atoi(argv[2]));         
  
    if (connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr))!=0)  
    { 
        perror("connect"); close(sockfd); return -1; 
    }

    char buffer[1024];
    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,\
            "GET / HTTP/1.1\r\n"\
            "Host: %s:%s\r\n"\
            "\r\n",argv[1],argv[2]);
    

    if (send(sockfd,buffer,sizeof(buffer),0) <= 0)
    {
        perror("send()"); close(sockfd);return -1;
    }

    while(true)
    {
        memset(buffer,0,sizeof(buffer));
        if (recv(sockfd,buffer,sizeof(buffer),0) <= 0)
        {
            perror("recv()");close(sockfd);return -1;
        }

        cout << buffer <<endl;
    }
    
    close(sockfd);
    return 0;
}