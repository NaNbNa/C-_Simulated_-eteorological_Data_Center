#include "_public.h"

using namespace idc;

int main(int argc,char *argv[])
{
    if (argc!=2)
    {
        cout << "   ./demo2 80\n\n"; return -1;
    }


    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd==-1) 
    { 
        perror("socket"); return -1; 
    }

    int opt = 1; unsigned int len = sizeof(opt);
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,len);
  
    struct sockaddr_in servaddr;        
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;        
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));     

    if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) != 0 )
    { 
        perror("bind"); close(listenfd); return -1; 
    }

    if (listen(listenfd,5) != 0 ) 
    { 
        perror("listen"); close(listenfd); return -1; 
    }
 

    int clientfd=accept(listenfd,0,0);
    if (clientfd==-1)
    {
        perror("accept"); close(listenfd); return -1; 
    }

    cout << "客户端已连接。\n";


    char buffer[1024];
    memset(buffer,0,sizeof(buffer));

    if (recv(clientfd,buffer,sizeof(buffer),0) <= 0)
    {
        return -1;
    }

    cout << "接受：" << buffer << endl;

    close(listenfd);close(clientfd);

}