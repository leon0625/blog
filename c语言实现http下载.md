### 实现http的下载  

> 用C语言实现简单的http下载接口，方便移植到其他程序中使用  

**要满足的功能：**  

* 支持chunked方式传输的下载  
* 被重定向时能下载重定向页面  
* 要实现的接口为`int http_download(char *url, char *save_path)`    

**思路：**  

1. 解析输入的URL，分离出主机，端口号，文件路径的信息  
2. 解析主机的DNS  
3. 填充http请求的头部，给服务器发包  
4. 解析收到的http头，提取状态码，Content-length, Transfer-Encoding等字段信息  
    * 如果是普通的头则进行接下来的正常收包流程  
    * 如果状态码为302，则从头里提取出重定向地址，用新的地址重新开始下载动作  
    * 如果传送方式是chunked的，则进行分段读取数据并拼接  
    * 如果是404或其他状态码则打印错误信息  

**缺陷：**  

* 没有使用非阻塞的方式来接收发送数据  
* 没有设置接收发送超时  
* 读写错误后没有判断错误代码  

**其他：**  

* 如何移植到没有文件系统的系统中？  
修改sava_data接口里面的保存就好了  

* 如何提高下载速度？  
    * 增大读写buffer缓冲区  
    * 改为多线程，使用Range字段分段读取，最后再拼在一起  

*代码*  
```c  
/************************************************************
Copyright (C), 2016, Leon, All Rights Reserved.
FileName: download.c
coding: UTF-8
Description: 实现简单的http下载功能
Author: Leon
Version: 1.0
Date: 2016-12-2 10:49:32
Function:

History:
<author>    <time>  <version>   <description>
 Leon
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#define HOST_NAME_LEN   256
#define URI_MAX_LEN     2048
#define RECV_BUF        8192

typedef struct {
    int sock;                       //与服务器通信的socket
    FILE *in;                       //sock描述符转为文件指针，方便读写
    char host_name[HOST_NAME_LEN];  //主机名
    int port;                       //主机端口号
    char uri[URI_MAX_LEN];          //资源路径
    char buffer[RECV_BUF];          //读写缓冲
    int status_code;                //http状态码
    int chunked_flag;               //chunked传输的标志位
    int len;                        //Content-length里的长度
    char location[URI_MAX_LEN];     //重定向地址
    char *save_path;                //保存内容的路径指针
    FILE *save_file;                //保存内容的文件指针
    int recv_data_len;              //收到数据的总长度
} http_t;

/* 打印宏 */
#define MSG_DEBUG   0x01
#define MSG_INFO    0x02
#define MSG_ERROR   0x04

static int print_level = /*MSG_DEBUG | */MSG_INFO | MSG_ERROR;

#define lprintf(level, format, argv...) do{     \
    if(level & print_level)     \
        printf("[%s][%s(%d)]:"format, #level, __FUNCTION__, __LINE__, ##argv);  \
}while(0)

#define MIN(x, y) ((x) > (y) ? (y) : (x))

#define HTTP_OK         200
#define HTTP_REDIRECT   302
#define HTTP_NOT_FOUND  404

/* 不区分大小写的strstr */
char *strncasestr(char *str, char *sub)
{
    if(!str || !sub)
        return NULL;

    int len = strlen(sub);
    if (len == 0)
    {
        return NULL;
    }

    while (*str)
    {
        if (strncasecmp(str, sub, len) == 0)
        {
            return str;
        }
        ++str;
    }
    return NULL;
}

/* 解析URL, 成功返回0，失败返回-1 */
/* http://127.0.0.1:8080/testfile */
int parser_URL(char *url, http_t *info)
{
    char *tmp = url, *start = NULL, *end = NULL;
    int len = 0;

    /* 跳过http:// */
    if(strncasestr(tmp, "http://"))
    {   
        tmp += strlen("http://");
    }
    start = tmp;
    if(!(tmp = strchr(start, '/')))
    {
        lprintf(MSG_ERROR, "url invaild\n");
        return -1;      
    }
    end = tmp;

    /*解析端口号和主机*/
    info->port = 80;   //先附默认值80

    len = MIN(end - start, HOST_NAME_LEN - 1);
    strncpy(info->host_name, start, len);
    info->host_name[len] = '\0';

    if((tmp = strchr(start, ':')) && tmp < end)
    {
        info->port = atoi(tmp + 1);
        if(info->port <= 0 || info->port >= 65535)
        {
            lprintf(MSG_ERROR, "url port invaild\n");
            return -1;
        }
        /* 覆盖之前的赋值 */
        len = MIN(tmp - start, HOST_NAME_LEN - 1);
        strncpy(info->host_name, start, len);
        info->host_name[len] = '\0';
    }
    
    /* 复制uri */
    start = end;
    strncpy(info->uri, start, URI_MAX_LEN - 1);

    lprintf(MSG_INFO, "parse url ok\nhost:%s, port:%d, uri:%s\n", 
        info->host_name, info->port, info->uri);
    return 0;
}

/* dns解析,返回解析到的第一个地址，失败返回-1，成功则返回相应地址 */
unsigned long dns(char* host_name)
{

    struct hostent* host;
    struct in_addr addr;
    char **pp;

    host = gethostbyname(host_name);
    if (host == NULL)
    {
        lprintf(MSG_ERROR, "gethostbyname %s failed\n", host_name);
        return -1;
    }

    pp = host->h_addr_list;
    
    if (*pp!=NULL)
    {
        addr.s_addr = *((unsigned int *)*pp);
        lprintf(MSG_INFO, "%s address is %s\n", host_name, inet_ntoa(addr));
        pp++;
        return addr.s_addr;
    }

    return -1;
}

/* 连接到服务器 */
int connect_server(http_t *info)
{
    int sockfd;
    struct sockaddr_in server;
    unsigned long addr = 0;
    unsigned short port = info->port;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        lprintf(MSG_ERROR, "socket create failed\n");
        goto failed;
    }
    
    /* 这里最好设置一下发送接收超时时间 */

    if ((addr = dns(info->host_name)) == -1)
    {
        lprintf(MSG_ERROR, "Get Dns Failed\n");
        goto failed;
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET; 
    server.sin_port = htons(port); 
    server.sin_addr.s_addr = addr;

    if (-1 == connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)))
    {
        lprintf(MSG_ERROR, "connect failed\n");
        goto failed;
    }

    info->sock = sockfd;
    return 0;

failed:
    if(sockfd != -1)
        close(sockfd);
    return -1;
}

/* 发送http请求 */
int send_request(http_t *info)
{
    int len;

    memset(info->buffer, 0x0, RECV_BUF);
    snprintf(info->buffer, RECV_BUF - 1, "GET %s HTTP/1.1\r\n"
        "Accept: */*\r\n"
        "User-Agent: Mozilla/5.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n"
        "Host: %s\r\n"
        "Connection: Close\r\n\r\n", info->uri, info->host_name);

    lprintf(MSG_DEBUG, "request:\n%s\n", info->buffer);
    return send(info->sock, info->buffer, strlen(info->buffer), 0);
}

/* 解析http头 */
void parse_http_header(http_t *info)
{
    char *p = NULL;

    // 解析第一行
    fgets(info->buffer, RECV_BUF, info->in);
    p = strchr(info->buffer, ' ');
    info->status_code = atoi(p + 1);   
    lprintf(MSG_DEBUG, "http status code: %d\n", info->status_code);

    // 循环读
    while(fgets(info->buffer, RECV_BUF, info->in))
    {
        // 判断头部是否读完
        if(!strcmp(info->buffer, "\r\n"))
            break;
        lprintf(MSG_DEBUG, "%s", info->buffer);
        // 解析长度 Content-length: 554
        if(p = strncasestr(info->buffer, "Content-length"))
        {
            p = strchr(p, ':');
            p += 2;     // 跳过冒号和后面的空格
            info->len = atoi(p);
            lprintf(MSG_INFO, "Content-length: %d\n", info->len);
        }
        else if(p = strncasestr(info->buffer, "Transfer-Encoding"))
        {
            if(strncasestr(info->buffer, "chunked"))
                info->chunked_flag = 1;
            lprintf(MSG_INFO, "%s", info->buffer);
        }
        else if(p = strncasestr(info->buffer, "Location"))
        {
            p = strchr(p, ':');
            p += 2;     // 跳过冒号和后面的空格
            strncpy(info->location, p, URI_MAX_LEN - 1);
            lprintf(MSG_INFO, "Location: %s\n", info->location);
        }
    }
}

/* 保存服务器响应的内容 */
int save_data(http_t *info, const char *buf, int len)
{
    int total_len = len;
    int write_len = 0;

    // 文件没有打开则先打开
    if(!info->save_file)
    {
        info->save_file = fopen(info->save_path, "w");
        if(!info->save_file)
        {
            lprintf(MSG_ERROR, "fopen %s error: %m\n", info->save_path);
            return -1;
        }
    }

    while(total_len)
    {
        write_len = fwrite(buf, sizeof(char), len, info->save_file);
        if(write_len < 0)
        {
            lprintf(MSG_ERROR, "fwrite error: %m\n");
            return -1;
        }
        total_len -= write_len;
    }
}

/* 读数据 */
int read_data(http_t *info, int len)
{
    int total_len = len;
    int read_len = 0;

    while(total_len)
    {
        read_len = MIN(total_len, RECV_BUF);
        read_len = fread(info->buffer, sizeof(char), read_len, info->in);
        if(read_len < 0)
        {
            lprintf(MSG_ERROR, "fread error: %m\n");
            return -1;
        }
        total_len -= read_len;
        lprintf(MSG_DEBUG, "read len: %d\n", read_len);
        if(-1 == save_data(info, info->buffer, read_len))
        {
            return -1;
        }
        info->recv_data_len += read_len;
    }
    if(total_len != 0)
    {
        lprintf(MSG_ERROR, "we need to read %d bytes, but read %d bytes now\n", 
            info->len, info->len - total_len);
        return -1;
    }
}

/* 接收服务器发回的chunked数据 */
int recv_chunked_response(http_t *info)
{
    long part_len;

    //有chunked，content length就没有了
    do{
        // 获取这一个部分的长度
        fgets(info->buffer, RECV_BUF, info->in);
        part_len = strtol(info->buffer, NULL, 16);
        lprintf(MSG_DEBUG, "part len: %ld\n", part_len);
        if(-1 == read_data(info, part_len))
            return -1;

        //读走后面的\r\n两个字符
        if(2 != fread(info->buffer, sizeof(char), 2, info->in))
            return -1;
    }while(part_len);
    lprintf(MSG_INFO, "recv %d bytes\n", info->recv_data_len);
    return 0;
}

/* 接收服务器的响应数据 */
int recv_response(http_t *info)
{
    int len = 0, total_len = info->len;
    
    if(info->chunked_flag)
        return recv_chunked_response(info);

    if(-1 == read_data(info, total_len))
        return -1;

    lprintf(MSG_INFO, "recv %d bytes\n", info->recv_data_len);
    return 0;
}

/* 清理操作 */
void clean_up(http_t *info)
{
    if(info->in)
        fclose(info->in);
    if(-1 != info->sock)
        close(info->sock);
    if(info->save_file)
        fclose(info->save_file);
    if(info)
        free(info);
}

/* 下载主函数 */
int http_download(char *url, char *save_path)
{
    http_t *info = NULL;
    char tmp[URI_MAX_LEN] = {0};

    if(!url || !save_path)
        return -1;

    //初始化结构体
    info = malloc(sizeof(http_t));
    if(!info)
    {
        lprintf(MSG_ERROR, "malloc failed\n");
        return -1;
    }
    memset(info, 0x0, sizeof(http_t));
    info->sock = -1;
    info->save_path = save_path;

    // 解析url
    if(-1 == parser_URL(url, info))
        goto failed;

    // 连接到server
    if(-1 == connect_server(info))
        goto failed;

    // 发送http请求报文
    send_request(info);

    // 接收响应的头信息
    info->in = fdopen(info->sock, "r");
    if(!info->in)
    {
        lprintf(MSG_ERROR, "fdopen error\n");
        goto failed;
    }

    // 解析头部
    parse_http_header(info);

    switch(info->status_code)
    {
        case HTTP_OK:
            // 接收数据
            lprintf(MSG_DEBUG, "recv data now\n");
            if(-1 == recv_response(info))
                goto failed;
            break;
        case HTTP_REDIRECT:
            // 重启本函数
            lprintf(MSG_INFO, "redirect: %s\n", info->location);
            strncpy(tmp, info->location, URI_MAX_LEN - 1);
            clean_up(info);
            return http_download(tmp, save_path);

        case HTTP_NOT_FOUND:
            // 退出
            lprintf(MSG_ERROR, "Page not found\n");
            goto failed;
            break;

        default:
            lprintf(MSG_INFO, "Not supported http code %d\n", info->status_code);
            goto failed;
    }

    clean_up(info);
    return 0;
failed:
    clean_up(info);
    return -1;
}

/****************************************************************************
测试用例:
./a.out "http://www.httpwatch.com/httpgallery/chunked/chunkedimage.aspx" test.txt
./a.out "192.168.10.1/main.html" test.txt
./a.out "32131233" test.txt
./a.out "www.baidu.com/" test.txt
./a.out "192.168.0.200:8000/FS_AC6V1.0BR_V15.03.4.12_multi_TD01.bin" test.txt
****************************************************************************/

int main(int argc, char *argv[])
{
    if(argc < 3)
        return -1;

    http_download(argv[1], argv[2]);
    return 0;
}
```