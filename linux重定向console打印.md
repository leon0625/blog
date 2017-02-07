> 在嵌入式开发中，我们都是使用串口进行调试定位问题。然而在成品调试或者远程调试时，没有串口，只能telnet进去，少了很多应用进程的打印，这样就不利于我们发现问题。需要一种方法把串口所有输出重定向到telnet。  

　　这就涉及到一些终端概念，可以参考上篇博文[linux下tty, ttyn, pts, pty, ttySn, console理解](http://blog.csdn.net/u013401853/article/details/54915853)。主要是利用tty的ioctl重定向方法来实现重定向，下面直接贴代码：  

```c

/************************************************************
Copyright (C), 2017, Leon, All Rights Reserved.
FileName: console_redirect.c
Description: console输出重定向
Author: Leon
Version: 1.0
Date: 2017-2-6 15:33:12
Function:
History:
<author>    <time>  <version>   <description>
 Leon
************************************************************/

/*
    内核的打印不能重定向过来，应用层打印可以重定向打印过来
    查看内核的打印，cat /proc/kmsg，在输出完缓冲区内容后，会阻塞卡住，内核有新的输出时会继续输出。
    如果要把内核打印到telnet，那么需要修改printk.c。
    kernel和user空间下都有一个console，关系到kernel下printk的方向和user下printf的方向，实现差别很大。
    kernel下的console是输入输出设备driver中实现的简单的输出console，只实现write函数，并且是直接输出到设备。
    user空间下的console，实际就是tty的一个特殊实现，大多数操作函数都继承tty，所以对于console的读写，都是由kernel的tty层来最终发送到设备。
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int tty = -1;
    char *tty_name = NULL;

    if(argc < 2)
    {
        printf("miss argument\n");
        return 0;
    }

    /* 获取当前tty名称 */
    tty_name = ttyname(STDOUT_FILENO);
    printf("tty_name: %s\n", tty_name);

    if(!strcmp(argv[1], "on"))
    {
        /* 重定向console到当前tty */
        tty = open(tty_name, O_RDONLY | O_WRONLY);
        ioctl(tty, TIOCCONS);
        perror("ioctl TIOCCONS");
    }
    else if(!strcmp(argv[1], "off"))
    {
        /* 恢复console */
        tty = open("/dev/console", O_RDONLY | O_WRONLY);
        ioctl(tty, TIOCCONS);
        perror("ioctl TIOCCONS");
    }
    else
    {
        printf("error argument\n");
        return 0;
    }

    close(tty);
    return 0;
}
```

* 使用  
交叉编译tftp进板子，然后telnet进去，用on参数执行这个程序就好了  

* 内核打印到telnet的思考  
内核的打印没有走tty的中间层，直接在printk哪儿调用了注册设备的write方法。如果要把printk打印到telnet，那么就需要分析下printk实现代码。自己改了下在printk输出时把数据重新写到/dev/console的这种方法，因为对printk逻辑不清楚，锁处理会产生问题，造成死锁或者系统挂掉。  