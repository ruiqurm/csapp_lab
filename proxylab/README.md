
# lab说明
这个lab要求我们实现一个代理服务器，只需要处理GET请求即可。我们的代理服务器需要同时能接受一个HTTP请求和发送一个HTTP请求。

这次的自由性比较高，`proxy.c`几乎只给了一个`main`函数。
## 端口

## 测试
这个Lab没有提供测试点，需要自己用`telnet`、`curl`等工具测试。
### curl
```
 curl -v --proxy http://localhost:15214 http://localhost:15213/home.html
```
### netcat
```
 nc catshark.ics.cs.cmu.edu 12345
 GET http://www.cmu.edu/hub/index.html HTTP/1.0
 ```
# proxy lab
## Part 1

GET localhost/home.html HTTP/1.0
http://

但是，如果http请求无头的话，这个

## Part 3
LRU的一种比较简单的实现是用双端链表+Hash Map.