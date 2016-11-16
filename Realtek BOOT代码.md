### Realtek BOOT代码

#### 启动进入CFE

head.S里`jal	init_arch` , `init_arch`再调用`start_kernel`

`start_kernel()`里初始化时钟、串口、堆、中断、flash、gpio、打印板子信息、tenda加的亮灯

​	`check_image()`如果返回0，待会儿进入doBooting里，会直接进入到cfe模式

​	`doBooting()` 通过判断进入正常模式或者cfe模式

​		`user_interrupt()` 检查是否按了ESC,按了按键等

​		`goToDownMode()` 进入cfe模式

```c
void goToDownMode()
{
#ifndef CONFIG_SW_NONE
    eth_startup(0);		//初始化网卡
    dprintf("\n---Ethernet init Okay!\n");
    sti();
    tftpd_entry(0);		//初始化arp表
#ifdef DHCP_SERVER
    dhcps_entry();		//初始化地址池，server ip, arp
#endif
#ifdef HTTP_SERVER
    httpd_entry();		//初始化arp table
#endif
#endif
    monitor();		//进入命令行
    return ;
}
```

***

#### 网络初始化

eth_startup()注册中断处理函数

```c
static struct irqaction irq_eth15 = {eth_interrupt, 0, 15, "eth0", NULL, NULL};
request_IRQ(ETH0_IRQ, &irq_eth15, &(ETH[0]));
```

在`eth_interrupt`里面收包，并通过`kick_tftpd()`处理收到的包,只处理了 ARP,DHCP,TFTP,HTTP。如果是DHCP的包进入`dhcps_input`，如果是TCP的包进入`tcpinput`，否则根据不同的包类型，设置event变量，执行回调。

判断不同包头，做不同处理

```c
if (kick_event != NUM_OF_BOOT_EVENTS)
{
  	jump = (void *)(*BootStateEvent[bootState][kick_event]);
  	jump();
}
```

这是注册的函数

```c
static const Func_t BootStateEvent[NUM_OF_BOOT_STATES][NUM_OF_BOOT_EVENTS]
=
{
    /*BOOT_STATE0_INIT_ARP*/
    {
        /*BOOT_EVENT0_ARP_REQ*/     doARPReply,
        /*BOOT_EVENT1_ARP_REPLY*/   updateARPTable,
        /*BOOT_EVENT2_TFTP_RRQ*/    setTFTP_RRQ,
        /*BOOT_EVENT3_TFTP_WRQ*/    setTFTP_WRQ,
        /*BOOT_EVENT4_TFTP_DATA*/   errorDrop,/*ERROR in state transition*/
        /*BOOT_EVENT5_TFTP_ACK*/    errorDrop,/*ERROR in state transition*/
        /*BOOT_EVENT6_TFTP_ERROR*/  errorDrop,/*ERROR in state transition*/
        /*BOOT_EVENT7_TFTP_OACK*/   errorDrop,/*ERROR in state transition*/
    },
    /*BOOT_STATE1_TFTP_CLIENT_RRQ*/
    {
        /*BOOT_EVENT0_ARP_REQ*/     doARPReply,
        /*BOOT_EVENT1_ARP_REPLY*/   updateARPTable,
        /*BOOT_EVENT2_TFTP_RRQ*/    setTFTP_RRQ,
        /*BOOT_EVENT3_TFTP_WRQ*/    errorTFTP,/*ERROR in TFTP protocol*/
        /*BOOT_EVENT4_TFTP_DATA*/   prepareACK,
        /*BOOT_EVENT5_TFTP_ACK*/    prepareDATA,
        /*BOOT_EVENT6_TFTP_ERROR*/  errorTFTP,/*ERROR in TFTP protocol*/
        /*BOOT_EVENT7_TFTP_OACK*/   errorTFTP,/*ERROR in TFTP protocol*/
    },
    /*BOOT_STATE2_TFTP_CLIENT_WRQ*/
    {
        /*BOOT_EVENT0_ARP_REQ*/     doARPReply,
        /*BOOT_EVENT1_ARP_REPLY*/   updateARPTable,
        /*BOOT_EVENT2_TFTP_RRQ*/    errorTFTP,/*ERROR in TFTP protocol*/
        /*BOOT_EVENT3_TFTP_WRQ*/    setTFTP_WRQ,
        /*BOOT_EVENT4_TFTP_DATA*/   prepareACK,
        /*BOOT_EVENT5_TFTP_ACK*/    prepareDATA,
        /*BOOT_EVENT6_TFTP_ERROR*/  errorTFTP,/*ERROR in TFTP protocol*/
        /*BOOT_EVENT7_TFTP_OACK*/   errorTFTP,/*ERROR in TFTP protocol*/
    },
#ifdef SUPPORT_TFTP_CLIENT
    /*BOOT_STATE3_TFTP_SERVER_DATA*/
    {
        /*BOOT_EVENT0_ARP_REQ*/ 	doARPReply,
        /*BOOT_EVENT1_ARP_REPLY*/	updateARPTable,
        /*BOOT_EVENT2_TFTP_RRQ*/	errorTFTP,/*ERROR in TFTP protocol*/
        /*BOOT_EVENT3_TFTP_WRQ*/	errorTFTP,
        /*BOOT_EVENT4_TFTP_DATA*/	prepareACK,
        /*BOOT_EVENT5_TFTP_ACK*/	errorTFTP,
        /*BOOT_EVENT6_TFTP_ERROR*/	errorTFTP,/*ERROR in TFTP protocol*/
        /*BOOT_EVENT7_TFTP_OACK*/	errorTFTP,/*ERROR in TFTP protocol*/
    },
    /*BOOT_STATE4_TFTP_SERVER_DATA*/
    {
        /*BOOT_EVENT0_ARP_REQ*/ 	doARPReply,
        /*BOOT_EVENT1_ARP_REPLY*/	updateARPTable,
        /*BOOT_EVENT2_TFTP_RRQ*/	errorTFTP,/*ERROR in TFTP protocol*/
        /*BOOT_EVENT3_TFTP_WRQ*/	errorTFTP,
        /*BOOT_EVENT4_TFTP_DATA*/	prepareACK,
        /*BOOT_EVENT5_TFTP_ACK*/	errorTFTP,
        /*BOOT_EVENT6_TFTP_ERROR*/	errorTFTP,/*ERROR in TFTP protocol*/
        /*BOOT_EVENT7_TFTP_OACK*/	errorTFTP,/*ERROR in TFTP protocol*/
    },
#endif
};
```

**HTTP处理**

　　在`tcpInputData`里对http数据包做处理，直接判断GET, POST，如果是GET，直接回页面内容，即全局变量indexdata[]里存的页面内容。如果是POST, 执行`httpuploadfile`收数据包，并拼合在一起。

*这里看着好厉害的样子，直接memcpy到某个地址*

```c
/*write upload image to 0x80400000*/
static unsigned long httpd_mem = (IMAGE_MEM_ADDR);

if (findimagehead(payload, length, &headlen))
 {
     /*in order to copy image aligned. we need find the image header. */
     memcpy((void *)(httpd_mem + httpd_mem_len), payload + headlen, length - headlen);
     httpd_mem_len += (length - headlen);
 }
```

收到数据的长度大于等于升级文件长度后，`imageFileValid` 检查升级文件，合法则设置`readyToUpgrade = 1;`

后面在收到FIN报文时，判断这个标志位然后写flash `writeImagetoflash`，重启`autoreboot`。

　　在`writeImagetoflash`里会判断升级文件头的位置，而且jack在里面已经加了如果有"RTK0"头则跳过的代码。所以在cfe模式下，通过tftp的方式可以直接升级RTK0开头的升级文件。那么如果要在cfe模式里通过http升级RTK0的升级文件，只需要改`imageFileValid`里的检查就可以了。

---

####正常启动

　　在`doBooting`里没检测到`ESC`按下，进入正常模式执行`goToLocalStartMode`。从flash里读出升级文件的头，再跳到这个头的startAddr执行。

```c
/* Firmware image header */
typedef struct _header_
{
    unsigned char signature[SIG_LEN];	//"cs6c"
    unsigned long startAddr;		//启动地址
    unsigned long burnAddr;
    unsigned long len;
} IMG_HEADER_T, *IMG_HEADER_Tp;

jump = (void *)(pheader->startAddr);
jump();				 // jump to start
```

看了一下这个地址为0x80500000，这个地址是在编译的时候写进去的。在`ecos-work/AP/rtkload/Makefile`可以看到这个地址的定义：

```Makefile
LOAD_START_ADDR=0x80500000

$(OBJCOPY) --add-section .vmlinux=vmlinux_img.gz vmlinux_img.o
@sed "$(SEDFLAGS)" < ld.script.in > $(LDSCRIPT)
$(LD) $(LDFLAGS) -G 0 -T $(LDSCRIPT) -o memload-partial $(START_FILE) $(LOADER_FILES) 				  $(SUPPORT_FILES) vmlinux_img.o
$(NM) memload-partial | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\				(LASH[RL]DI\)' | sort > system.map
```

vmlinux_img.gz为AP目录编译的users, kernel后得到的`appimg`经过一系列压缩而成。`$(OBJCOPY) --add-section .vmlinux=vmlinux_img.gz vmlinux_img.o`这里把vmlinux_img.gz的内容复制到vmlinux_img.o的.vmlinux段里。然后通过ld链接

上面ld的链接脚本为`rtkload/ld.script`, 脚本表明入口地址为`__start`，起始地址赋为`0x80500000`，`__vmlinux_start`和`__vmlinux_end`分别标记了.vmlinux段地址的起始。

```asm
// ld.script
OUTPUT_ARCH(mips)
ENTRY(__start)
SECTIONS
{
        /* The loader itself */
        . = 0x80500000;
        .text : { *(.text) } 
        .rodata : { *(.rodata) } 
        . = ALIGN(4);
        /* Compressed kernel ELF image */
		 .data    :
  		{
    		_fdata = . ;
    		*(.data)
   			. = ALIGN(1024);
   			__vmlinux_start = .;
   			*(.vmlinux)
   			__vmlinux_end = .;
   			. = ALIGN(1024);
    		CONSTRUCTORS
  		}
        .bss : { _bstart = . ; *(.bss) ; *(.sbss) ; *(COMMON) ; _bend = . ; }
        /* /DISCARD/ : { *(.reginfo) ; *(.mdebug) ; *(.note) ; *(.comment) *(__ex_table) ; } */
        /DISCARD/ : { *(.reginfo) ;  *(.note) ; *(.comment) *(__ex_table) ; }
        /* .filler : */
}
```

`__start`在start.S里定义，会通过`j main`语句跳转到main执行，main里面解压kernel，然后执行`start_kernel(kernelStartAddr);`。这个函数也在start.S里定义，代码知直接跳转到入参kernelStartAddr执行

```asm
start_kernel:
        move t0, a0
        li a0, 0
        li a1, 0
        li a2, 0
		jr t0	//直接跳到参数地址执行
```

那么kernelStartAddr怎么确定的呢？在上面执行main解压kernel时赋的值，__vmlinux_start加4 ，即后续会跳转到vmlinux_img里执行。

```c
	unsigned long pending_len = *((unsigned long *)__vmlinux_start);
	kernelStartAddr = *((unsigned long *)(startBuf+4));
```

我们知道目标文件vmlinux_img是appimg生成的。查看下appimg怎么生成的，./ecos-work/AP/Makefile里：

```Makefile
NAME = appimg
LDFLAGS += -L$(ECOS_INSTALL_DIR)/lib -L$(ECOS_AP_DIR) -Ttarget.ld
$(CC) -o $(NAME) $(LDFLAGS) $(LIB_OBJS) $(APP_OBJS) $(EXTRA_OBJS)   
```

那么如何执行appimg里的内容，就全看链接脚本target.ld了。

查看内核的链接脚本target.ld里`ENTRY(reset_vector)`可知，入口函数为reset_vector。定义在

`ecos-3.0/packages/hal/mips/arch/v3_0/src/vectors.S`里, `reset_vector`->`_start`->`cyg_start`。在汇编写的_start函数里执行了很多硬件初始化函数，然后跳转到了c代码`cyg_start`执行。

cyg_start同名的比较多，正确的为`ecos-3.0/packages/infra/v3_0/src/startup.cxx:91:cyg_start( void )`，然后进入cyg_user_start。`./ecos-work/AP/shell/init.c:91:void cyg_user_start(void)`。后面跟起来就很随手了

`cyg_user_start`->`shell_init_thread`->`create_sys_init_thread`->`tapf_sys_init`到了`tapf_sys_init`就很亲切了,这已经到了我们自己的函数了，就很熟悉了。



> *刘良明 	2016年7月28日16:38:59*