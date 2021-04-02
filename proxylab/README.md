
# lab说明
这个lab要求我们实现一个代理服务器，只需要处理GET请求即可。我们的代理服务器需要同时能接受一个HTTP请求和发送一个HTTP请求。

## 目前进度

完成了lab，但是有一些小细节没有修改：（可能以后有空回来再加上吧）

* 无法处理uri上不带host的情况
* LRU 哈希表没写（直接顺序查询）

## 测试
lab提供了自动打分的`driver.sh`。但是在`wsl`环境下是用不了的，可以在ubuntu的虚拟机环境上使用。此外，如果是比较新版本的`ubuntu`，`/usr/bin/python`是没有的，需要把`/nop-server.py`里的第一行改为`#!/usr/bin/python3`

自己测试的时候，有几种可选的方式：

* 显式地把代理地址写在uri里。例如` http://localhost:15214/localhost:15213/home.html` 。其中15214是代理服务器。

* `curl -v --proxy`

  例如：` curl -v --proxy http://localhost:15214 http://localhost:15213/home.html`

* 使用`netcat`

  ```
   nc localhost 15213
   GET http://www.cmu.edu/hub/index.html HTTP/1.0
   
   Host: www.cmu.edu
   
  ```

使用`netcat`如果不手动断开连接，就会一直连着服务器。对于`lab2`，可以测试被`netcat`占用时能否响应。

## hint

* 可以随意修改文件、添加文件，修改`makefile`
* 

# proxy lab

## Part 1

大体的框架是：

* 等待请求(`Accept`)
* 处理`uri`
* 构建新头
* 连接目标服务器
* 写入目标服务器
* 关闭连接

具体的实现可以参考书上`tiny`服务器+客户端；

需要仔细处理的主要是处理`uri`和构建`header`。

### `parse_uri`

不同形式地访问服务器，uri也会相应不同。至少有以下几种形式：

* `http://localhost:15214/localhost:15213/home.html`

* `http://localhost/localhost:15213/home.html`(无端口)
* `/localhost:15213/home.html`
* `/localhost/home.html`(无端口)
* `/home.html`(没有host)

目前只处理了前4种情况，对于没有host的相对uri。例如`/home.html`，就会报错。

```c
void parse_uri(char* uri,char* host,char*path,int *port){
    //下面会修改uri，但会复原
    char* ptr = strstr(uri,"//");
    char c_temp;//用于复原uri
    
    if (ptr!=NULL){
        // uri形如:http://localhost:15213/home.html
            ptr += 2; 
            char* portpos = strstr(ptr, ":");
            if (portpos!=NULL){
                // localhost:15213/home.html
                c_temp = *portpos;
                *portpos = '\0' ;
                sscanf(ptr, "%s", host);
                sscanf(portpos+1, "%d%s", port, path);
                *portpos = c_temp;
            }else{
                // localhost/home.html
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
        if (portpos!=NULL){
            // /localhost:15213/home.html
            c_temp = *portpos;
            *portpos = '\0' ;
            sscanf(ptr, "%s", host);// 读取host
            sscanf(portpos+1, "%d%s", port, path);//读取端口和path
            *portpos = c_temp;
            return; 
        }else{
            // localhost/home.html
            char *split = strstr(ptr,"/");
            *split = '\0';
            strcpy(host,ptr); // 读取host
            printf("host: %s\n",host);
            *split = '/';
            strcpy(path,split);//分离出path
            return;
        }
    }
}
```



### 构建新头

这一部分需要将旧的头复制到新的头上。主要需要注意的是`\r\n`。每行都需要`\r\n`，同时，请求和请求头之间有一个空行。

题目中还要求要修改`User-Agent`,`Host`,`Connection`,`Proxy-Connection`。因为这边是直接写入的，因此不能后面再修改。这边将原头的这几个部分跳过，后面再加上。

```c
void build_requesthdrs(rio_t* from,int to,char* host,char*path){
    char buf[MAXLINE];
    printf("--------------------\nheaders:\n");
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
    return;
}
```

### 连接和写入

首先解析GET请求的uri，获得目标host，然后打开和目标端口的连接，复制新头，最后写入请求。

```c
void doit(int fd){
    rio_t rio,rio_endserver;
    int clientfd,readed;
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char host[MAXLINE],path[MAXLINE];
    char port[6];
    Rio_readinitb(&rio,fd);
    Rio_readlineb(&rio,buf,MAXLINE);
    sscanf(buf,"%s %s %s",method,uri,version); // GET /cmu/home.html HTTP/1.1  
    
    if (strcasecmp(method,"get")){
        clienterror(fd,method,"501","Not implemented","Tiny does not implement this method");
        return;
    }
    int iport =80;
    printf("uri: %s\n",uri);
    parse_uri(uri,host,path,&iport);
    printf("method: %s,\nhost: %s,\npath: %s,\nversion: %s\n",method,host,path,version);
    sprintf(port,"%d",iport);
    clientfd = Open_clientfd(host,port);
    Rio_readinitb(&rio_endserver,clientfd);
    build_requesthdrs(&rio,clientfd,host,path);

    while ((readed = Rio_readlineb(&rio_endserver, buf, MAXLINE))) {
        Rio_writen(fd, buf, readed);  
    }
    Close(clientfd);
}
```

### 完整的文件

[proxy_part1.c](./proxy_part1.c)

### 异常处理

lab并不测试异常处理，这一部分不写也没事。但是如果用浏览器连接这个服务器做代理，可能中途就会因为异常而被关闭。

对于`csapp.c`的，某些内置函数，例如`Open_clientfd`等，如果失败了，就会直接`exit(0)`。由于这边开的是单/多线程，这样直接整个主程序就退出了。

可以参考[七 PROXY LAB](https://www.jianshu.com/p/a501d0c2f131)  Part 2.1的解决方案：

![](https://upload-images.jianshu.io/upload_images/10803273-9283823830fd4755.png?imageMogr2/auto-orient/strip|imageView2/2/w/677/format/webp)

![](https://upload-images.jianshu.io/upload_images/10803273-1348de739e273043.png?imageMogr2/auto-orient/strip|imageView2/2/w/631/format/webp)



## Part 2

这一部分要求代理服务器能够实现并发。说明文件中要求用线程实现，但我一开始没看到，因此实现了一个进程版的。

## process

[process.c](./proxy_part2_process.c)

和`Part 1`相比，主要修改了几个地方：

* 子进程回收
* 子进程和主进程fd关闭

```c
....
Signal(SIGCHLD,sigchld_handler);// 回收同时产生的子进程回收信号
....
while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd,(SA*)&clientaddr,&clientlen);
        if(Fork()==0){
            Close(listenfd);// 子进程关闭监听的fd

            Getnameinfo((SA*)&clientaddr,clientlen,clienthost_name,MAXLINE,
                    client_port,MAXLINE,0);
            printf("[%d]Accept connection from (%s, %s)\n",getpid(),clienthost_name,client_port);fflush(stdout);
            
            doit(connfd);

            Close(connfd);
            printf("[%d]Close connection from (%s, %s)\n\n",getpid(),clienthost_name,client_port);fflush(stdout);

            exit(0);//子进程退出
        }
        printf("main process listening..\n");
        Close(connfd);//父进程关闭处理http的fd
    }
```
信号回收主要是处理同时死亡的子进程。
```c
void sigchld_handler(int sig)
{
    while(waitpid(-1,0,WNOHANG) > 0)
        ;
    return;
}
```



## thread

[thread.c](./proxy_part2_thread.c)

线程需要注意传参。

按照书上的代码构建框架：

`main`:

```c
...
while(1){
    clientlen = sizeof(struct sockaddr_storage);
    connfd = (int*) Malloc(sizeof(int)); //*
    *connfd = Accept(listenfd,(SA*)&clientaddr,&clientlen);
    Getnameinfo((SA*)&clientaddr,clientlen,clienthost_name,MAXLINE,
    client_port,MAXLINE,0);
    printf("Accept connection from (%s, %s)\n",clienthost_name,client_port);
    fflush(stdout);
    Pthread_create(&tid,NULL,thread,connfd);//创建一个子线程
}
...
```

`thread`

```c
void *thread(void *vargp)
{
    int connfd = *((int*)vargp);
    Pthread_detach(pthread_self()); //和主线程脱离
    Free(vargp);
    doit(connfd);
    Close(connfd);
    printf("Close connection.\n");
    fflush(stdout);
    return NULL;
}
```



## Part 3

LRU的一种比较简单的实现是用链表+Hash Map。此处我只实现了链表。

字符串哈希的我直接采用了这里的：[字符串哈希函数](https://blog.csdn.net/mylinchi/article/details/79508112)

```c
// BKDR Hash Function
unsigned int BKDRHash(char *str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
 
    while (*str)
    {
        hash = hash * seed + (*str++);
    }
 
    return (hash & 0x7FFFFFFF);
}
```

LRU的链表实现在每次更新的时候把元素放到最后面去，每次取元素时把元素提到最前面去。在没有哈希表的情况下，需要`O(n)`的时间去检索。

[link_list.c](./link_list.c)

因为这个链表是写在外部的，还要修改makefile:

```makefile
all: proxy

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

proxy.o: proxy.c csapp.h
	$(CC) $(CFLAGS) -c proxy.c

link_list.o: link_list.c link_list.h
	$(CC) $(CFLAGS) -c link_list.c

proxy: proxy.o csapp.o link_list.o
	$(CC) $(CFLAGS) proxy.o csapp.o link_list.o -o proxy $(LDFLAGS)

```

在每次`get_cache`或者`update_cahce`的时候，都要加锁。

`doit`

```c
...
struct link_node *node;
int size; 

P(&lock);
node = get_cache(&LRU,uri);
if (node){
    strcpy(buf,node->data);
    size = node->size;
}
V(&lock);

if ((node)!=NULL)
{
    Rio_writen(fd,buf,size);
}else{
    parse_uri(uri,host,path,&iport);
    printf("method: %s,\nhost: %s,\npath: %s,\nversion: %s\n",method,host,path,version);;
    sprintf(port,"%d",iport);
    clientfd = Open_clientfd(host,port);
    Rio_readinitb(&rio_endserver,clientfd);
    build_requesthdrs(&rio,clientfd,host,path);
    int cache_size = 0;
    while ((readed = Rio_readlineb(&rio_endserver, buf, MAXLINE))) {
        if(cache_size+readed<MAX_OBJECT_SIZE)
        {
            strcpy(cache+cache_size,buf);
            cache_size += readed;
        }
        Rio_writen(fd, buf, readed);  
    }
    P(&lock);
    update_cache(&LRU,uri,cache,cache_size);
    V(&lock);
    Close(clientfd);
}
...
```

[完整文件](./proxy_part3.c)

