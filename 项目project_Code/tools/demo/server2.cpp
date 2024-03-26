#include "_public.h"
#include "_ooci.h"

using namespace idc;


bool senddata(const int sockfd,const char* strget);

bool getvalue(const string& strget,const string& name,string& value,const int len);

int main(int argc,char *argv[])
{
    if (argc!=2)
    {
        cout << "   ./server2 8080\n\n"; return -1;
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

    char recvbuffer[1024];
    memset(recvbuffer,0,sizeof(recvbuffer));

    if (recv( clientfd,recvbuffer,sizeof(recvbuffer),0) <= 0)
    {
        return -1;
    }

    cout << recvbuffer <<endl;

    char sendbuffer[1024];
    memset(sendbuffer,0,sizeof(sendbuffer));
    sprintf(sendbuffer,\
            "HTTP/1.1 200 ok\r\n"\
            "Server: server2\r\n"\
            "Content-Type:text/html\r\n"\
            "\r\n");

    send(clientfd,sendbuffer,sizeof(sendbuffer),0);
    writen(clientfd,"<data>\n",strlen("<data>\n"));
    senddata(clientfd,recvbuffer);

    close(listenfd);
    close(clientfd);

    return 0;
}

bool senddata(const int sockfd,const char* strget)
{
    string username,passwd,intername,obtid,begintime,endtime;

    getvalue(strget,"username",username,30);
    getvalue(strget,"passwd",passwd,30);
    getvalue(strget,"intername",intername,30);
    getvalue(strget,"obtid",obtid,10);
    getvalue(strget,"begintime",begintime,20);
    getvalue(strget,"endtime",endtime,20);

    cout << "username=" <<  username <<endl;cout  << "passwd=" << passwd <<endl;
    cout << "intername=" << intername <<endl; cout << "obtid=" << obtid <<endl;
    cout << "begintime=" << begintime <<endl; cout << "endtime=" << endtime <<endl;
    
    connection conn;
    if (conn.connecttodb("idc/idcpwd@snorcl11g_131","Simplified Chinese_China.AL32UTF8") < 0)
        printf("conn database failed\n%s%\n",conn.message());
    else
        printf("conn database ok\n");

    sqlstatement stmt(&conn);
    stmt.prepare(
      "select '<obtid>'||obtid||'</obtid>'||'<ddatetime>'||to_char(ddatetime,'yyyy-mm-dd hh24:mi:ss')||'</ddatetime>'||'<t>'||t||'</t>'||'<p>'||p||'</p>'||'<u>'||u||'</u>'||'<keyid>'||keyid||'</keyid>'||'<endl/>' "
            "from T_ZHOBTMIND1 where obtid=:1 and ddatetime>to_date(:2,'yyyymmddhh24miss') and ddatetime<to_date(:3,'yyyymmddhh24miss')"
        );
    
    char strxml[1001];memset(strxml,0,sizeof(strxml));
    stmt.bindout(1,strxml,1000);
    stmt.bindin(1,obtid,10);stmt.bindin(2,begintime,14);stmt.bindin(3,endtime,14);

    if (stmt.execute() != 0)
        printf("execute failed\n%s\n%s\n",stmt.sql(),stmt.message());
    

    

    while(true)
    {
        memset(strxml,0,sizeof(strxml));
        if (stmt.next() != 0) break;

        strcat(strxml,"\n");

        writen(sockfd,strxml,strlen(strxml));
    }

    writen(sockfd,"</data>\n",strlen("</data>\n"));

    return true;
}

bool getvalue(const string& strget,const string& name,string& value,const int len)
{
    int startp = strget.find(name);
    if (startp == string::npos) return false;

    int endp = strget.find("&",startp);
    if (endp == string::npos) endp = strget.find(" ",startp);

    if (endp == string::npos) return false;

    int itemplen = endp - startp - name.length() - 1;
    if ((len >0) && (len < itemplen))   itemplen = len;
    
    value = strget.substr(startp+name.length()+1,itemplen);

    return true;
}