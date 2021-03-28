#include <stdio.h>
#include "csapp.h"
#include <string.h>
#include "link_list.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define DEBU

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
void *thread(void *vargp);
void parse_uri(char* uri,char* host,char*path,int*port);

int main(int argc,char **argv)
{
    int listenfd;
    int *connfd;// 每个线程独享
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char clienthost_name[MAXLINE],client_port[MAXLINE];
    pthread_t tid;//线程编号
    
    struct link_list LRU;
    init_link_list(&LRU);

    if (argc != 2){
        fprintf(stderr,"usage: %s <port>\n", argv[0]);
        #ifdef DEBUG
            listenfd = Open_listenfd("28936");
            printf("Server listen on http://localhost:%s\n","28936");
        #else
            exit(1);
        #endif
    }else{
        listenfd = Open_listenfd(argv[1]);
        printf("Server listen on http://localhost:%s\n",argv[1]);
    }
    

    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = (int*) Malloc(sizeof(int));
        *connfd = Accept(listenfd,(SA*)&clientaddr,&clientlen);
        Getnameinfo((SA*)&clientaddr,clientlen,clienthost_name,MAXLINE,
            client_port,MAXLINE,0);
        printf("Accept connection from (%s, %s)\n",clienthost_name,client_port);
        fflush(stdout);
        Pthread_create(&tid,NULL,thread,connfd);
    }
    free_list(&LRU);
}

void *thread(void *vargp)
{
    int connfd = *((int*)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    doit(connfd);
    Close(connfd);
    printf("Close connection.\n");
    fflush(stdout);
    return NULL;
}


void doit(int fd){
    rio_t rio,rio_endserver;
    int clientfd,readed;
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char host[MAXLINE],path[MAXLINE];
    char port[6];
    Rio_readinitb(&rio,fd);
    Rio_readlineb(&rio,buf,MAXLINE);
    sscanf(buf,"%s %s %s",method,uri,version); // GET /cmu/a.txt HTTP/1.1  
    
    if (strcasecmp(method,"get")){
        clienterror(fd,method,"501","Not implemented","Tiny does not implement this method");
        return;
    }
    int iport =80;
    printf("uri: %s\n",uri);;
    parse_uri(uri,host,path,&iport);
    printf("method: %s,\nhost: %s,\npath: %s,\nversion: %s\n",method,host,path,version);;
    sprintf(port,"%d",iport);
    clientfd = Open_clientfd(host,port);
    Rio_readinitb(&rio_endserver,clientfd);
    build_requesthdrs(&rio,clientfd,host,path);

    while ((readed = Rio_readlineb(&rio_endserver, buf, MAXLINE))) {//real server response to buf
        Rio_writen(fd, buf, readed);  //real server response to real client
    }
    Close(clientfd);
}
void parse_uri(char* uri,char* host,char*path,int *port){
    char* ptr = strstr(uri,"//");
    if (ptr!=NULL){
        // 形如uri:http://localhost:15213/home.html
            ptr += 2; 
            char* portpos = strstr(ptr, ":");
            if (portpos!=NULL){// localhost:15213/home.html
                *portpos = '\0' ;
                sscanf(ptr, "%s", host);
                sscanf(portpos+1, "%d%s", port, path);
            }else{// localhost/home.html
                char *split = strstr(ptr,"/");
                *split = '\0';
                strcpy(host,ptr);
                *split = '/';
                strcpy(path,split);
            }
        return ;
    }
    ptr = uri;
    if (ptr[0]=='/'){
        ptr+=1;
        char* portpos = strstr(ptr, ":");
        if (portpos!=NULL){// /localhost:15213/home.html
            *portpos = '\0' ;
            sscanf(ptr, "%s", host);
            sscanf(portpos+1, "%d%s", port, path);
            return; 
        }else{// localhost/home.html
            char *split = strstr(ptr,"/");
            *split = '\0';
            strcpy(host,ptr);
            printf("host: %s\n",host);
            *split = '/';
            strcpy(path,split);
            return;
        }
    }
    // {
    //     temp = strstr(ptr,"/");
    //     if(temp != NULL) {
    //         *temp = '\0';
    //         sscanf(ptr, "%s", host);
    //         *temp = '/';
    //         sscanf(temp, "%s", path);
    //     }
    //     else {
    //         sscanf(ptr, "%s", host);
    //     }
    // }
    // return;
}

void build_requesthdrs(rio_t* from,int to,char* host,char*path){
    char buf[MAXLINE];
    #ifdef DEBUG
        printf("--------------------\nheaders:\n");
    #endif
    sprintf(buf,"GET %s HTTP/1.0\r\n",path);
    Rio_writen(to,buf,strlen(buf));

    Rio_readlineb(from,buf,MAXLINE);
    while(strcmp(buf,"\r\n")){
        if (!strstr(buf,"User-Agent:") || !strstr(buf,"Host:") || !strstr(buf,"Connection:") || !strstr(buf,"Proxy-Connection:")){
            Rio_readlineb(from,buf,MAXLINE);
            continue;
        }
        Fputs(buf,stdout);
        Rio_writen(to,buf,strlen(buf));
        Rio_readlineb(from,buf,MAXLINE);
    }
    strcpy(buf,user_agent_hdr);
    #ifdef DEBUG
        Fputs(buf,stdout);
    #endif
    Rio_writen(to,buf,strlen(buf));
    
    strcpy(buf,proxy_connection_hdr);
    #ifdef DEBUG
        Fputs(buf,stdout);
    #endif
    Rio_writen(to,buf,strlen(buf));
    
    strcpy(buf,connection_hdr);
    #ifdef DEBUG
        Fputs(buf,stdout);
    #endif
    Rio_writen(to,buf,strlen(buf));

    sprintf(buf,"Host: %s\r\n",host);
    #ifdef DEBUG
        Fputs(buf,stdout);
    #endif
    Rio_writen(to,buf,strlen(buf));

    #ifdef DEBUG
        printf("----------------------\n");fflush(stdout);
    #endif
    
    Rio_writen(to,"\r\n",2);
    // if (has_body){
    //     Rio_readlineb(from,body,MAXBUF);
    //     Rio_writen(to,body,MAXBUF);
    // }
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