# phase0
## 目标
* eval: Main routine that parses and interprets the command line. [70 lines]
执行、处理命令行指令的函数
* builtin cmd: Recognizes and interprets the built-in commands: quit, fg, bg, and jobs. [25
lines]  
处理内置命令(`fg`,`bg`,`jobs`,`quit`)的函数
* do bgfg: Implements the bg and fg built-in commands. [50 lines]  
处理`bg`,`fg`命令
* waitfg: Waits for a foreground job to complete. [20 lines]  
等待一个前台进程执行完毕
* sigchld handler: Catches SIGCHILD signals. 80 lines]    
捕获`SIGHCHILD`
* sigint handler: Catches SIGINT (ctrl-c) signals. [15 lines]  
捕获`SIGINT`
* sigtstp handler: Catches SIGTSTP (ctrl-z) signals. [15 lines]  
捕获`SIGTSTP`
## 详细说明
### `quit`
退出
### `bg` `fg`
打印出前台或者后台执行的进程

### `jobs`
所有正在执行的进程

### `&`
在一个命令后面加上这个符号可以使其后台执行。

### `PID`,`JID`
`pid`是进程编号，用`<PID:int>`表示，`JID`是`job`编号（每新建一个`job`）递增1，用`%<JID:int>`表示

### `ctrl+c`,`ctrl+z`
输入这两个分别发送`SIGINT`和`SIGTSTP`  

### 其他
不需要支持`vi`,`more`等命令   
不需要支持管道和I/O重定向

## 编译和测试
* 使用`make`编译文件
* `./tsh` 可以打开编译后的文件。`ctrl+D` 可以退出
### 测试
首先`make`编译，然后`make test01`可以测试trace01.txt。以此类推。  
`make rtest01` 可以查看标准输出

## 提示
翻译自文档
* 根据测试文件编写代码。
* 使用`-pid` 停止整个进程组
* 关于`waitfg` 和`sigchld_handler`的处理，下面是一种比较简单的解决方案：  
    * 在`waitfg`中,用循环检测是否存在`job`，然后+`sleep`。(用suspend也可)
    * 在`sigchld_handler`中，只调用一次`waitpid`。
* 在`eval`中，主进程在`fork`之前要屏蔽`SIGCHLD`信号，在`addjob`后取消屏蔽。子进程进入时要取消继承来的`SIGCHLD`屏蔽。
* 在`eval`的子进程执行之前，需要使用`setpgid(0,0)`新建一个进程组（与主进程的进程组分离）。这样如果你使用`ctrl+c`发送`SIGINT`就不会同时发给主进程和子进程了（默认情况下，它们在一个进程组）。


## 源文件的可用函数

| 函数     | 解释                 |
| -------- | -------------------- |
| `parseline` | 处理命令行字符串，把参数装在数组里，返回bool值是否是`bg` |
| `init`,`clearjob`  |     初始化、情况jobs |
|`maxjid`   |            当前最大jid          |
|`addjob`|添加新job|
|`deletejob`|删除job|
|`fgpid`|返回前台运行job的pid|
|`getjobpid`|通过pid获取job|
|`getjobjid`|通过jid获取job|
|`pid2jid`|pid转换为jid|
|`listjobs`|打印出所有jobs|
|`unix_error`|输出异常|
|||


# phase1
## test01
这个测试的是`ctrl+D`，已经写好了  
## test02 - Process builtin quit command

这里需要大概写一下`eval`。这里抄了一半书上写的`eval`。

需要处理的是`builtin_cmd`函数。这个函数返回0时会开子进程执行，否则当作内置命令执行。这里需要当成内置命令执行。

```c
void eval(char *cmdline) 
{
    char buf[MAXLINE];
    char *argv[MAXARGS];
    int bg;

    strcpy(buf,cmdline);//dest在前
    bg = parseline(buf,argv);
    if (argv[0] == NULL){
        return;
    }
    if (!builtin_cmd(argv)){
        //....
    }
    return;
}
int builtin_cmd(char **argv) 
{
    //strcmp==0时为真
    if(!strcmp(argv[0],"quit"))
        exit(0);
    if (!strcmp(argv[0],"&"))//去除空行
        return 1;
    return 0;     /* not a builtin command */
}

```

## test03 - Run a foreground job

在test2的基础上，这次需要处理非内置命令，等待其执行，并在执行完后回收job。

```c
void eval(char *cmdline) 
{
    char buf[MAXLINE];
    char *argv[MAXARGS];
    pid_t pid;
    int jid,state;
    int bg;
    sigset_t mask,prev_mask;

    strcpy(buf,cmdline);//dest在前
    bg = parseline(buf,argv);
    if (argv[0] == NULL){
        return;
    }
    if (!builtin_cmd(argv)){
        sigemptyset(&mask);
        sigaddset(&mask,SIGCHLD);//阻塞子进程终止信号
        sigprocmask(SIG_BLOCK, &mask, &prev_mask);
            
        if ((pid=fork())==0){
            setpgid(0,0);//分离
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            // 子进程解除阻塞
            if (execve(argv[0],argv,environ)<0){
                printf("%s: Command not found.\n",argv[0]);
                exit(0);
            }
        }else{
            state = bg ? BG : FG;//bg和BG对应的数字是不一样的
            addjob(jobs,pid,state,cmdline);
            sigprocmask(SIG_SETMASK,&prev_mask,NULL);
            if(!bg){
                waitfg(pid);
            }
        }
    }
    return;
}
```

对于`waitfg`，只要简单的等待即可。

可以用`fgpid()`或者`getjobpid()` 来判断fg是否还在运行。  

也可以用`sigsuspend`(P545)。这个函数阻塞直到接收到一个信号，并恢复原来的set

```c
void waitfg(pid_t pid)
{
    while(fgpid(jobs)>0){
        sleep(0.1);
    }
    return;
}
//或者

void waitfg(pid_t pid)
{
    sigset_t mask,prev;
    sigemptyset(&mask);
    sigprocmask(SIG_BLOCK,&mask,&prev);//无论接到什么信号，都会停止
    
    while(fgpid(jobs)>0){
        sigsuspend(&prev);//效率更高
    }
    return;
}
```

对于处理`sigchld_handler`，使用`waitpid`获取pid，然后删除job即可。

```c
void sigchld_handler(int sig) 
{
    pid_t pid;
    sigset_t mask_all,prev_mask;
    sigfillset(&mask_all);
    
    while ((pid = waitpid(-1,NULL,0))>0){
        sigprocmask(SIG_BLOCK,&mask_all,&prev_mask);
        deletejob(jobs,pid); 
        sigprocmask(SIG_SETMASK,&prev_mask,NULL);
    }
    return;
}
```

## test04 -  Run a background job

后台程序和前台程序差不多，只是需要多输出一句`[1] (957) ./myspin 1 &`

修改上面的`eval`函数

```c
//...
if(!bg){
	waitfg(pid);
}else{
	jid = pid2jid(pid);
	printf("[%d] (%d) %s",jid, pid ,cmdline);
}
//...
```

## test05 - Process jobs builtin command.

处理`jobs`命令。

可以用内置的`listjobs`。

修改一下`builtin_cmd`

```c
int builtin_cmd(char **argv) {
    if(!strcmp(argv[0],"quit"))
        exit(0);
    if(!strcmp(argv[0],"jobs")){// 添加
        listjobs(jobs);
        return 1;
    }
    if (!strcmp(argv[0],"&"))
        return 1;
    return 0;     /* not a builtin command */
}
```

编译一下运行，发现`jobs`输出为空。检查了一圈，发现是`sigchld_handler`有问题。

`waitpid(-1,NULL,0)`：检查父进程所有的子进程，直到有一个子进程终止。

而改成`waitpid(-1,NULL,WNOHANG | WUNTRACED)`就可以了。这个参数使得`waitpid`不会被阻塞。

```c
void sigchld_handler(int sig) 
{
    int olderrno = errno;
    
    pid_t pid;
    sigset_t mask_all,prev_mask;
    sigfillset(&mask_all);
    
    while ((pid = waitpid(-1,NULL,WNOHANG | WUNTRACED))>0){//修改
        sigprocmask(SIG_BLOCK,&mask_all,&prev_mask);
        deletejob(jobs,pid); 
        sigprocmask(SIG_SETMASK,&prev_mask,NULL);
    }
    
    errno = olderrno;
    return;
}
```

此外，这里加上了errno的恢复。

## test06 - Forward SIGINT to foreground job.

处理`SIGINT  `  

这里使用`kill`发送信号即可。注意要发给整个进程组

```c
pid_t pid;
if ((pid=fgpid(jobs))>0){
	kill(-pid,SIGINT);// 要发给子进程的进程组
	printf("Job [%d] (%d) terminated by signal 2\n",pid2jid(pid),pid);
}
```



## test07 - Forward SIGINT only to foreground job.

这个因为之前就是前台`job`有效，因此不需要再修改了。

输出：

```shell
./sdriver.pl -t trace07.txt -s ./tsh -a "-p"
#
# trace07.txt - Forward SIGINT only to foreground job.
#
tsh> ./myspin 4 &
[1] (1575) ./myspin 4 &
tsh> ./myspin 5
Job [2] (1577) terminated by signal 2
tsh> jobs
[1] (1575) Running ./myspin 4 &
```

## test08 - Forward SIGTSTP only to foreground job.

首先像上面一样修改试试：

```c
void sigtstp_handler(int sig) 
{
    pid_t pid;
    if ((pid=fgpid(jobs))>0){
        getjobpid(jobs,pid)->state=ST;
        kill(-pid,SIGTSTP);
        printf("Job [%d] (%d) stopped by signal 20\n",pid2jid(pid),pid);
    }
    return;
}
```

预期的结果是：

```shell
./sdriver.pl -t trace08.txt -s ./tshref -a "-p"
#
# trace08.txt - Forward SIGTSTP only to foreground job.
#
tsh> ./myspin 4 &
[1] (2480) ./myspin 4 &
tsh> ./myspin 5
Job [2] (2482) stopped by signal 20
tsh> jobs
[1] (2480) Running ./myspin 4 &
[2] (2482) Stopped ./myspin 5 
```

同时，上面的进程是阻塞的，因为`job[2]`被暂停了。



但是执行一下会发现，虽然也成功阻塞了，但是joblists里面并没有`job[2]`。这可能是信号接收的时候出了问题，我尝试改了发送的信号为`SIGSTOP`之类的，并没有效果。

那么子进程的信号处理在哪里呢？在`sigchld_handler`中。

在一个子进程停止或终止时，会发送`SIGCHLD`给父进程。然而我们之前的代码只考虑了终止的情况：

```c
while ((pid = waitpid(-1,&status,WNOHANG | WUNTRACED))>0){
        sigprocmask(SIG_BLOCK,&mask_all,&prev_mask);
        deletejob(jobs,pid); 
        sigprocmask(SIG_SETMASK,&prev_mask,NULL);
}
```

这样子，即使是暂停的话也会被从`joblists`中删除。

因此我们需要区分子进程的状态。标准库提供了三个宏分别来判断是否正常终止(`WIFEXITED`)，信号终止(`WIFSIGNALED`)以及停止(`WIFSTOPPED`)。

```c
void sigchld_handler(int sig) 
{
    int olderrno = errno;
    pid_t pid;
    int status;
    sigset_t mask_all,prev_mask;
    // sigemptyset(&mask);
    sigfillset(&mask_all);
    while ((pid = waitpid(-1,&status,WNOHANG | WUNTRACED))>0){
        if WIFSTOPPED(status){
            //如果是停止的
            getjobpid(jobs,pid)->state=ST;
        }else{
            sigprocmask(SIG_BLOCK,&mask_all,&prev_mask);
            deletejob(jobs,pid); 
            sigprocmask(SIG_SETMASK,&prev_mask,NULL);
        }
    }
    // sigprocmask(SIG_SETMASK,&prev_mask,NULL);
    errno = olderrno;
    return;
}
```

之前的printf的输出也可以搬到这边来：

```c
void sigchld_handler(int sig) 
{
    int olderrno = errno;
    pid_t pid;
    int status;
    sigset_t mask_all,prev_mask;
    // sigemptyset(&mask);
    sigfillset(&mask_all);
    while ((pid = waitpid(-1,&status,WNOHANG | WUNTRACED))>0){
        if WIFSTOPPED(status){
            //如果是停止的
            printf("Job [%d] (%d) stopped by signal 20\n",pid2jid(pid),pid);
            getjobpid(jobs,pid)->state=ST;
        }else{
            if WIFSIGNALED(status){
                printf("Job [%d] (%d) terminated by signal 2\n",pid2jid(pid),pid);
            }
            sigprocmask(SIG_BLOCK,&mask_all,&prev_mask);
            deletejob(jobs,pid); 
            sigprocmask(SIG_SETMASK,&prev_mask,NULL);
        }
    }
    // sigprocmask(SIG_SETMASK,&prev_mask,NULL);
    errno = olderrno;
    return;
}
```



## test09,test10 - Process bg,fg builtin command

处理`bg`,`fg`指令

首先需要在`builtin_cmd`里加上`bg`。

```c
int builtin_cmd(char **argv) 
{
    if(!strcmp(argv[0],"quit"))
        exit(0);
    if(!strcmp(argv[0],"jobs")){
        listjobs(jobs);
        return 1;
    }
    if(!strcmp(argv[0],"bg") || !strcmp(argv[0],"fg")){//处理bg
        do_bgfg(argv);
        return  1;
    }
    if (!strcmp(argv[0],"&"))
        return 1;
    return 0;     /* not a builtin command */
}
```

然后写`do_bgfg`函数。

这个函数需要能够判断参数是JID还是PID。判断方法只需要判断参数第一位是否是`%`即可。然后转化成id，查询相应的job即可。

查询到job后，发送一个`SIGCONT`信号可以使它继续运行。

最后，判断当前工作是`fg` 还是`pg`的，修改状态，阻塞或者后台运行：

```c
void do_bgfg(char **argv) 
{
    int jid,pid;
    struct job_t* job;
    if (argv[1][0] == '%'){//JID
        jid = atoi(argv[1]+1);
        job = getjobjid(jobs,jid);
        pid = job->pid;
    }else{//PID
        pid = atoi(argv[1]);
        job = getjobpid(jobs,pid);
    }
    kill(-pid,SIGCONT);
    if(!strcmp(argv[0],"fg")){//ST->FG
        job->state = FG;
        waitfg(pid);
    }else{//ST->BG
        job->state = BG;
        printf("[%d] (%d) %s",pid2jid(pid),pid ,job->cmdline);
    }
    return;
}
```

 子进程恢复运行状态在`sigchld_handler`也会被捕获，因此要判断一下：

```c
void sigchld_handler(int sig) 
{
    int olderrno = errno;
    pid_t pid;
    int status;
    sigset_t mask_all,prev_mask;
    sigfillset(&mask_all);
    while ((pid = waitpid(-1,&status,WNOHANG | WUNTRACED))>0){
        if WIFCONTINUED(status){//判断是否是恢复运行
            // do nothing
        }else if WIFSTOPPED(status){
            printf("Job [%d] (%d) stopped by signal 20\n",pid2jid(pid),pid);
            getjobpid(jobs,pid)->state=ST;
        }else{
            if WIFSIGNALED(status){
                printf("Job [%d] (%d) terminated by signal 2\n",pid2jid(pid),pid);
            }
            sigprocmask(SIG_BLOCK,&mask_all,&prev_mask);
            deletejob(jobs,pid); 
            sigprocmask(SIG_SETMASK,&prev_mask,NULL);
        }
    }
    errno = olderrno;
    return;
}
```



## test11,12,13 - Forward SIGINT/SIGTSTP/SIGCONT to every process in foreground process group

这部分在之前做过了，即子进程自己分离出一个进程组；同时向进程发信号的时候也是向进程组发送信号。

## test14 - Simple error handling

异常处理

* Command not found

* fg/bg command requires PID or %jobid argument

  判断第一个参数是否存在即可

* argument must be a PID or %jobid

  这个之前用atoi，不太好判断；改用sscanf再判断就行了。

* No such process

* No such job

```c
void do_bgfg(char **argv) 
{
    int jid,pid;
    struct job_t* job;
    if (argv[1] == NULL) {
        printf("%s command requires PID or %%jobid argument\n",argv[0]);
        return;
    }
    if (sscanf(argv[1],"%%%d",&jid)>0){//JID
        job = getjobjid(jobs,jid);
        if (!job){
            printf("No such job\n");
            return;
        }
        pid = job->pid;
    }else if (sscanf(argv[1],"%d",&pid)>0){//PID
        job = getjobpid(jobs,pid);
        if (!job){
            printf("No such process\n");
            return;
        }
    }else{
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        return;
    }
    
    kill(-pid,SIGCONT);
    if(!strcmp(argv[0],"fg")){//ST->FG
        job->state = FG;
        waitfg(pid);
    }else{//ST->BG
        job->state = BG;
        printf("[%d] (%d) %s",pid2jid(pid),pid ,job->cmdline);
    }
    return;
}
```

## test15 - Putting it all together

最后的结果：

```c
void eval(char *cmdline) 
{
    char buf[MAXLINE];
    char *argv[MAXARGS];
    pid_t pid;
    int jid,state;
    int bg;
    sigset_t mask,prev_mask,empty_mask;

    strcpy(buf,cmdline);//dest在前
    bg = parseline(buf,argv);
    if (argv[0] == NULL){
        return;
    }
    if (!builtin_cmd(argv)){
        //....
        sigemptyset(&mask);
        sigaddset(&mask,SIGCHLD);//阻塞子进程终止信号
        sigprocmask(SIG_BLOCK, &mask, &prev_mask);

        if ((pid=fork())==0){
            setpgid(0,0);
            sigemptyset(&empty_mask);
            sigprocmask(SIG_SETMASK, &empty_mask, NULL);
            // 子进程解除阻塞
            if (execve(argv[0],argv,environ)<0){
                printf("%s: Command not found.\n",argv[0]);
                exit(0);
            }
        }else{
            state = bg ? BG : FG;
            addjob(jobs,pid,state,cmdline);
            sigprocmask(SIG_SETMASK,&prev_mask,NULL);
            if(!bg){
                waitfg(pid);
            }else{
                jid = pid2jid(pid);
                printf("[%d] (%d) %s",jid, pid ,cmdline);
            }
        }
    }
    return;
}



int builtin_cmd(char **argv) 
{
    //strcmp==0时为真
    if(!strcmp(argv[0],"quit"))
        exit(0);
    if(!strcmp(argv[0],"jobs")){
        listjobs(jobs);
        return 1;
    }
    if(!strcmp(argv[0],"bg") || !strcmp(argv[0],"fg")){
        do_bgfg(argv);
        return  1;
    }
    if (!strcmp(argv[0],"&"))//去除空行
        return 1;
    return 0;     /* not a builtin command */
}

void do_bgfg(char **argv) 
{
    int jid,pid;
    struct job_t* job;
    if (argv[1] == NULL) {
        printf("%s command requires PID or %%jobid argument\n",argv[0]);
        return;
    }
    if (sscanf(argv[1],"%%%d",&jid)>0){//JID
        job = getjobjid(jobs,jid);
        if (!job){
            printf("No such job\n");
            return;
        }
        pid = job->pid;
    }else if (sscanf(argv[1],"%d",&pid)>0){//PID
        job = getjobpid(jobs,pid);
        if (!job){
            printf("No such process\n");
            return;
        }
    }else{
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        return;
    }
    
    kill(-pid,SIGCONT);
    if(!strcmp(argv[0],"fg")){//ST->FG
        job->state = FG;
        waitfg(pid);
    }else{//ST->BG
        job->state = BG;
        printf("[%d] (%d) %s",pid2jid(pid),pid ,job->cmdline);
    }
    return;
}

void waitfg(pid_t pid)
{
    sigset_t mask,prev;
    sigemptyset(&mask);
    sigprocmask(SIG_BLOCK,&mask,&prev);
    
    while(fgpid(jobs)>0){
        sigsuspend(&prev);
        // sleep(0.1);
    }
    return;
}

/*****************
 * Signal handlers
 *****************/

void sigchld_handler(int sig) 
{
    int olderrno = errno;
    pid_t pid;
    int status;
    sigset_t mask_all,prev_mask;
    // sigemptyset(&mask);
    sigfillset(&mask_all);
    while ((pid = waitpid(-1,&status,WNOHANG | WUNTRACED))>0){
        if WIFCONTINUED(status){
            // do nothing
        }else if WIFSTOPPED(status){
            //如果是停止的
            printf("Job [%d] (%d) stopped by signal 20\n",pid2jid(pid),pid);
            getjobpid(jobs,pid)->state=ST;
        }else{
            if WIFSIGNALED(status){
                printf("Job [%d] (%d) terminated by signal 2\n",pid2jid(pid),pid);
            }
            sigprocmask(SIG_BLOCK,&mask_all,&prev_mask);
            deletejob(jobs,pid); 
            sigprocmask(SIG_SETMASK,&prev_mask,NULL);
        }
    }
    // sigprocmask(SIG_SETMASK,&prev_mask,NULL);
    errno = olderrno;
    return;
}

void sigint_handler(int sig) 
{
    pid_t pid;
    if ((pid=fgpid(jobs))>0){
        kill(-pid,SIGINT);// 要发给子进程的进程组
    }
    return;
}

void sigtstp_handler(int sig) 
{
    pid_t pid;
    if ((pid=fgpid(jobs))>0){
        // 要发给子进程的进程组
        kill(-pid,SIGTSTP);
        // printf("%d",j->state);
    }
    return;
}





```



