# linux进程的资源限制  

`　　最近软件比武，需要限制每个进程的内存使用大小和CPU使用时间，而以前使用的CPPUTEST测试框架里没有这些东西，需要自己写。发现linux下的setrlimit函数就能很好的解决问题了。`  

## setrlimit和getrlimit    
　　每个进程都有一组资源限制，可以通过getrlimit和setrlimit函数查询更改。  

**注意：**  
　　**资源限制影响到调用进程并由子进程继承。**  

*函数原型*  
```  
#include <sys/resource.h>

int getrlimit(int resource, struct rlimit *rlim);
int setrlimit(int resource, const struct rlimit *rlim);

struct rlimit {
    rlim_t rlim_cur;  /* Soft limit */
    rlim_t rlim_max;  /* Hard limit (ceiling for rlim_cur) */
};
```
更改资源限制时，有以下几点规则：  
1. 软限制值小于等于硬限制值；  
2. **超级用户才能更改硬限制值**；  

可通过reource参数设置需要更改的资源限制  
下面介绍一下关心的时间和内存限制：  

* RLIMIT_CPU  
cpu使用的最大值（秒），当超过软限制时，内核向改进程发送SIGXCPU信号。进程可以捕捉该信号，到内核会每个1s发一次，当超过硬限制时，会发送SIGKILL信号终止该进程。  

* RLIMIT_AS  
进程可用存储区的最大长度（字节）。这会影响sbrk很mmap函数。超过限制会收到SIGSEGV信号。    

* RLIMIT_DATA  
数据段的最大字节长度，这是初始化、非初始化以及堆的总大小。超过限制会收到SIGSEGV信号。  

* RLIMIT_STACK  
栈的最大大小  

***

### RLIMIT\_AS和RLIMIT\_DATA  
* 在实际使用中，我们往往会限制竞赛程序的malloc分配总大小和全局数组空间大小。那么这两个参数该怎么用才能限制住呢？  
　　全局数组肯定会占用数据段的大小，用RLIMIT\_DATA限制即可。关键是malloc的限制，这个glibc的实现有关了。glibc在分配小内存时在堆上分配，这通过RLIMIT_DATA可以限制；而超过限制的大内存是通过mmap分配，mmap分配的不在堆上，而它产生的地址空间受RLIMIT\_AS影响。  

问题：RLIMIT\_DATA和RLIMIT\_AS限制多大合适？  
　　

参考：  
https://github.com/lodevil/Lo-runner/issues/10  
http://www.cnblogs.com/niocai/archive/2012/04/01/2428128.html  
unix系统高级编程  

*一段限制的小程序*  
```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#define KB (1024)

void set_limit(void)
{
    struct rlimit memory_limit;

    //限制进程虚拟地址段大小
    memory_limit.rlim_cur = memory_limit.rlim_max = 10*1024*KB;
    if(setrlimit(RLIMIT_AS, &memory_limit))
        perror("setrlimit:");

    //限制进程数据段大小
    memory_limit.rlim_cur = memory_limit.rlim_max = 10*1024*KB;
    if(setrlimit(RLIMIT_DATA, &memory_limit))
        perror("setrlimit:");

    //限制进程栈大小
    memory_limit.rlim_cur = memory_limit.rlim_max = 1024*KB;
    if(setrlimit(RLIMIT_STACK, &memory_limit))
        perror("setrlimit:");

    //限制进程CPU时间
    memory_limit.rlim_cur = memory_limit.rlim_max = 1;
    if(setrlimit(RLIMIT_CPU, &memory_limit))
        perror("setrlimit:");
}

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;

    /* 子进程参数处理 */
    char *child_argv[10] = {NULL};
    int i = 0;

    for(i = 1; i < argc; i++)
    {
        child_argv[i-1] = argv[i];
    }

    set_limit();
    if((pid = fork()) < 0)
    {
        perror("");
        exit(-1);
    }
    else if(pid == 0)
    {
        execvp(child_argv[0], child_argv);  //执行竞赛程序
        printf("llm->%s(%d) execvp error\n", __FUNCTION__, __LINE__);
    }
    else
    {
        if(wait(&status) != pid)
            perror("wait:");

        if(WIFEXITED(status))
            printf("child process normal exit\n");
        else if(WIFSIGNALED(status))
            printf("abort,  signal number = %d\n", WTERMSIG(status));
    }
    return 0;
}
```