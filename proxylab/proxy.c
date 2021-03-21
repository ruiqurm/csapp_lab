#include <stdio.h>
#include "csapp.h"
#include <string.h>
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

// #define LISTENFD 4500

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *proxy_connection_hdr = "Proxy-Connection: close\r\n";
static const char *connection_hdr = "Connection: close\r\n";
/*
 * function
 */
void doit(int fd);
void clienterror(int fd,char *cause,char *errnum,char*shortmsg,char*longmsg);
void build_requesthdrs(rio_t* from,int to,char* host,char*path);
// void read_requesthdrs(rio_t *rp);
void parse_uri(const char* uri,char* host,char*path);

int main(int argc,char **argv)
{
    int listenfd,connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char clienthost_name[MAXLINE],client_port[MAXLINE];
    if (argc != 2){
        fprintf(stderr,"usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    listenfd = Open_listenfd(argv[1]);
    printf("Server listen on http://localhost:%s\n",argv[1]);
    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd,(SA*)&clientaddr,&clientlen);
        Getnameinfo((SA*)&clientaddr,clientlen,clienthost_name,MAXLINE,
                    client_port,MAXLINE,0);
        printf("Accept connection from (%s, %s)\n",clienthost_name,client_port);
        fflush(stdout);
        doit(connfd);
        Close(connfd);
        printf("Close connection from (%s, %s)\n\n",clienthost_name,client_port);
        fflush(stdout);
    }
}


/***********************
 * Other helper routines
 ***********************/

/******************************
 * end job list helper routines
 ******************************/

void doit(int fd){
    rio_t rio,rio_endserver;
    int clientfd,readed;
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char host[MAXLINE],path[MAXLINE];

    Rio_readinitb(&rio,fd);
    Rio_readlineb(&rio,buf,MAXLINE);
    sscanf(buf,"%s %s %s",method,uri,version); // GET /cmu/a.txt HTTP/1.1  
    
    if (strcasecmp(method,"get")){
        clienterror(fd,method,"501","Not implemented","Tiny does not implement this method");
        return;
    }
    parse_uri(uri,host,path);
    printf("method: %s,\nuri: %s,\nhost: %s,\npath: %s,\nversion: %s\n",method,uri,host,path,version);
    clientfd = Open_clientfd(host,"80");
    Rio_readinitb(&rio_endserver,clientfd);
    build_requesthdrs(&rio,clientfd,host,path);

    while ((readed = Rio_readlineb(&rio_endserver, buf, MAXLINE))) {//real server response to buf
        Rio_writen(fd, buf, readed);  //real server response to real client
    }
    Close(clientfd);
}
void parse_uri(const char* uri,char* host,char*path){
    char buf[MAXLINE];
    int remain_index = 0;
    char* token;
    
    strcpy(buf,uri);
    token = strtok(buf, "/");
    remain_index += strlen(token)+1;
    strcpy(host,token);
    strcpy(path,&uri[remain_index]);
    return;
}

void build_requesthdrs(rio_t* from,int to,char* host,char*path){
    char buf[MAXLINE],body[MAXBUF];
    int has_body = 0;
    printf("--------------------\nheaders:\n");
    sprintf(buf,"GET %s HTTP/1.0\r\n",path);
    Rio_writen(to,buf,strlen(buf));

    Rio_readlineb(from,buf,MAXLINE);
    while(strcmp(buf,"\r\n")){
        Fputs(buf,stdout);
        if (!strstr(buf,"User-Agent:") || !strstr(buf,"Host:") || !strstr(buf,"Connection:") || !strstr(buf,"Proxy-Connection:")){
            Rio_readlineb(from,buf,MAXLINE);
            continue;
        }
        // if (!strncasecmp(buf,"Content-Length:",10)){
        //     has_body =1;
        // }
        Fputs(buf,stdout);
        Rio_writen(to,buf,strlen(buf));
        Rio_readlineb(from,buf,MAXLINE);
    }
    strcpy(buf,user_agent_hdr);
    Fputs(buf,stdout);
    Rio_writen(to,buf,strlen(buf));
    
    strcpy(buf,proxy_connection_hdr);
    Fputs(buf,stdout);
    Rio_writen(to,buf,strlen(buf));
    
    strcpy(buf,connection_hdr);
    Fputs(buf,stdout);
    Rio_writen(to,buf,strlen(buf));

    sprintf(buf,"Host: %s\r\n",host);
    Fputs(buf,stdout);
    Rio_writen(to,buf,strlen(buf));

    printf("----------------------\n");
    fflush(stdout);
    Rio_writen(to,"\r\n",2);
    if (has_body){
        Rio_readlineb(from,body,MAXBUF);
        Rio_writen(to,body,MAXBUF);
    }
    return;
}

void clienterror(int fd,char *cause,char *errnum,char*shortmsg,char*longmsg){
    // modified from tiny-clienterror
    char buf[MAXLINE],body[MAXBUF];
    sprintf(body,"<html><title>Porxy Error</title><body>%s:%s\r\n<p>%s: %s\r\n</body></html>",errnum,shortmsg,longmsg,cause);
    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Connection: close\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Proxy-Connection: close\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "User-Agent: %s\r\n",user_agent_hdr);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n",(int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}