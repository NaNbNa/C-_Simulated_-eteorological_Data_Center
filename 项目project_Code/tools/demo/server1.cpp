#include "_public.h"

using namespace idc;

bool sendfile(const int clientfd,const string& filename);
int main(int argc,char *argv[])
{
    if (argc!=3)
    {
        cout << "   ./server1 80 index.html\n\n"; return -1;
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

    if (recv( clientfd,buffer,sizeof(buffer),0) <= 0)
    {
        return -1;
    }

    cout << buffer <<endl;

    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,\
            "HTTP/1.1 200 ok\r\n"\
            "Server: server1\r\n"\
            "Content-Length:%d\r\n"\
            "Content-Type:text/html\r\n"\
            "\r\n",filesize(argv[2]));

    send(clientfd,buffer,sizeof(buffer),0);

    sendfile(clientfd,argv[2]);


    close(listenfd);
    close(clientfd);

    return 0;
}

bool sendfile(const int clientfd,const string& filename)
{
    cifile ifile;
    if (ifile.open(filename,ios::binary|ios::in) == false)
    {
        printf("ifile.open(%s) failed\n ",filename.c_str());return false;
    }


    int filelen = filesize(filename);
    int onread=0;int totalbytes =0;
    char buffer[1024];int buflen = sizeof(buflen);
    while(true)
    {
        if ( filelen - totalbytes > buflen) onread = buflen;
        else onread = filelen - totalbytes;

        memset(buffer,0,sizeof(buffer));
        ifile.read(buffer,onread);

        send(clientfd,buffer,onread,0);

        totalbytes = totalbytes + onread;

        if (totalbytes == filelen) break;
    }

    return true;
}
