# 相关变量

| 变量     | 解释                 |
| -------- | -------------------- |
| `prompt` | 命令行前缀（`tsh>`） |
|          |                      |
|          |                      |



## main

* 重定向`stderr`到`stdout`
* `getopt` 获取参数

### getopt

```c
#include <unistd.h> 

int getopt(int argc, char * const argv[], const char *optstring);
```

### 安装信号

此处安装了4个信号。

其中