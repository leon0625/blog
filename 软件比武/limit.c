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
        execvp(child_argv[0], child_argv);
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