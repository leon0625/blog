### goahead代码-rom page原理  

> goahead作为一个开源webserver，肯定会涉及到对服务器上web文件的读取。如果服务器操作系统上没有文件系统怎么办呢？goahead依然可以搞定，开启`WEBS_PAGE_ROM`宏就可以把web文件一并编译进可执行文件，而无需文件系统支持就能正常浏览页面，下面来看看它是怎么做的吧  

**如何存储**  
　　文件在内存中的存储，使用了结构体来表示  
```c
typedef struct {
    char_t          *path;                  /* Web page URL path */
    unsigned char   *page;                  /* Web page data */
    int             size;                   /* Size of web page in bytes */
    int             pos;                    /* Current read position */
} websRomPageIndexType;
```  
　　page成员则是保存了文件的二进制内容。所有的页面文件以这个结构体的形式保存在一个.c文件（如webrom.c）中的全局数组变量websRomPageIndex中。这个webrom.c文件的内容是通过程序生成的，实现的源文件为webcomp.c中，主要过程为读取文件列表，把内容以二进制的形式写入全局变量，再生成websRomPageIndex的内容。所以webrom.c的内容就如这样一般。  
*webrom.c*  
```c
// 为了可读这里写为字符内容，实际为16进制内容
static unsigned char p0[] = {'h','e','l','l','o',0};    // hello.txt文件的内容
static unsigned char p1[] = {'w','o','r','l','d',0};    // world.txt文件的内容

websRomPageIndexType websRomPageIndex[] = {
    {"hello.txt", p0, 5},
    {"world.txt", p1, 5},
};
```  
　　通过上面的结构，就可以不需要文件系统支持而访问页面文件了。  

**编译**  
　　编译时，重点就是生成webrom.c，然后把webrom.c加入编译webserver的依赖，一起把webserver编出来。  
```Makefile
OBJS += webrom.o
webcomp:
    cc -o webcomp -DWEBS -DUEMF -DOS="Linux" -DLINUX -D_STRUCT_TIMEVAL $(WEB_COMP_EXTRA_CFLAGS) webcomp.c

webrom.c: webcomp
    # 生成页面文件列表
    find web/$(CONFIG_WEB_DIR) -type f ! -path "*.svn*" >web_files     
    # 文件名、文件内容和长度信息通过程序写入webrom.c的全局变量 
    ./webcomp web/$(CONFIG_WEB_DIR) web_files >webrom.c
```  

**优缺点分析**  
　　把页面文件直接编译进server的可执行文件，整个融为一体，不需要文件系统支持，访问页面从内存读取，速度更快。但可执行文件变大，消耗内存变多，修改页面文件必须要重新编译server。  

