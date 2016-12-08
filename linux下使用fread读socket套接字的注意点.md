### linux下使用fread读socket套接字的注意点  

>　　linux总所周知，一切皆文件。因此我们在读写文件时觉得fread,recv这些可以随便用。下面看看有哪些注意事项呢。  

　　因为C库的文件处理函数较多，处理起来很灵活。所以在处理套接字时可以用fdopen函数把linux下的文件描述符转为一个C库的FILE文件指针来进行读写操作。  

#### fread的返回值  

　　我们知道在recv出错时会返回-1，但fread的返回值则不是这样，它**不返回-1**。fread/fwrite返回读到的字节数，返回的字节数小于希望读的字节数，则表明发生了错误或者读到了文件尾。要使用ferror()和feof来判断究竟发生了什么。  
那么对于一个读的接口应该按下面这个模板来写，以处理文件读写发生的异常情况  
```c
int read_data(FILE *fp, int len)
{
    int total_len = len;
    int read_len = 0;
    int rtn_len = 0;
    char buffer[RECV_BUF] = {0};

    while(total_len)
    {
        read_len = MIN(total_len, RECV_BUF);
        rtn_len = fread(buffer, sizeof(char), read_len, fp);
        if(rtn_len < read_len)  /* 读到数据小于预期 */
        {
            if(ferror(fp))
            {
                if(errno == EINTR) /* 信号使读操作中断 */
                {
                    /* 不做处理继续往下走 */;
                }
                else if(errno == EAGAIN || errno == EWOULDBLOCK) /* 发生了超时 */
                {
                    lprintf(MSG_ERROR, "socket recvice timeout: %dms\n", RCV_SND_TIMEOUT);
                    total_len -= rtn_len;
                    lprintf(MSG_DEBUG, "read len: %d\n", rtn_len);
                    break;
                }
                else    /* 其他错误 */
                {
                    lprintf(MSG_ERROR, "fread error: %m\n");
                    break;
                }
            }
            else    /* 读到文件尾 */
            {
                lprintf(MSG_ERROR, "socket closed by peer\n");
                total_len -= rtn_len;
                lprintf(MSG_DEBUG, "read len: %d\n", rtn_len);
                break;
            }
        }
        
        // lprintf(MSG_DEBUG, " %s\n", buffer);
        total_len -= rtn_len;
        lprintf(MSG_DEBUG, "read len: %d\n", rtn_len);
    }
    if(total_len != 0)
    {
        lprintf(MSG_ERROR, "we need to read %d bytes, but read %d bytes now\n", 
            len, len - total_len);
        return -1;
    }
}
```  

#### fgets可以和recv混用吗  

　　从目前的测试来看不可以！例如我在处理http通信时，我想用fgets来解析http的头，而后面的数据部分则使用recv来读，测试发现recv收到的数据前面少了一截。这该如何解释呢？fgets在读数据时为提高性能，从socket的接收缓冲区一次性读了很多数据到自己的缓冲区，然后返回给用户一行数据，下一次读数据则优先从C库自己的缓冲区拿数据。但是下一次你用recv来读数据了，因此它从socket的接收缓冲区读到的数据就少了一截。  

#### fread读数据时超时的表现是什么  

　　fread本身不支持超时设置，只可以设置阻塞非阻塞。但fread的FILE指针是通过socket转过来，而socket是可以设置接收发送超时的，所以使用fread接收socket数据时也就具有超时的属性。但表现和recv超时不太一样。  
　　例如设置10s超时，我想recv 1000个字节，10s超时后recv会立马返回-1.而我想fread 1000个字节，你可能会看见在fread这儿不止卡10s，因为fread会尽量读满1000个字节再返回。在C库实现中，如果10s内，能收到数据，就读到自己缓冲区，然后接着收数据，直到读满1000个字节再返回。如果在10s内读不到数据那么就返回实际读到的数据，并把错误代码设置为EAGAIN。  
　　所以假如设置10s超时，用fread读8个字节，而对端每9s发送一个字节，那么fread将卡72s然后成功返回。  

