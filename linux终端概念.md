　　这一下涉及很多概念：终端，伪终端，虚拟终端，控制终端，串行终端，控制台，……。历史原因这些概念有些模糊，我也理不太清楚。就我直观对dev下设备而言来稍微解释下：  

* /dev/tty  
控制终端，即当前用户正在使用的终端，是一个映射，指向当前所使用的终端（例如/dev/tty1,/dev/pts/0）。往/dev/tty下写数据总是写到当前终端。  

* /dev/ttyn  
虚拟终端，例如ubuntu不启动图形界面时，那么就会默认连接到/dev/tty1这个虚拟终端。  

* /dev/pts/n  
伪终端，例如网络登录的telnet就是使用伪终端。这是UNIX98的实现风格，slave为/dev/pts/n是，master一般为/dev/ptmx。  

* /dev/pty[p-za-e][0-9a-f]  
伪终端，这是BSD的实现风格，slave一般使用/dev/tty[p-za-e][0-9a-f]这种格式，而master一般使用/dev/pty[p-za-e][0-9a-f]这种格式。 

* /dev/ttySn  
串行终端，串口设备对应的终端。  

* /dev/console  
应用层的控制台，一些进程的打印信息会输出到控制台。在用户层和内核都有一个console，分别对应printf和printk的输出。kernel下的console是输入输出设备driver中实现的简单的输出console，只实现write函数，并且是直接输出到设备。user空间下的console，实际就是tty的一个特殊实现，大多数操作函数都继承tty，所以对于console的读写，都是由kernel的tty层来最终发送到设备。  

**往/dev下各个终端设备写数据测试：**  
往/dev/ttyn, /dev/pts/n, /dev/ptyn, /dev/ttySn会写到对应的终端上去。  
往/dev/tty上写则会写到当前终端。  
往/dev/console写情况则不太一样，在ubuntu上测试时（没启动图像界面，启动的/dev/tty1)会写到/dev/tty1。板子上则会写到/dev/ttyS0。    

**参考**:  
[辛星浅析tty、pty与pts](http://www.2cto.com/os/201502/378314.html)  
[Linux中tty、pty、pts的概念区别](http://blog.sina.com.cn/s/blog_638ac15c01012e0v.html)  
[linux下tty，控制台，虚拟终端，串口，console（控制台终端）详解](http://blog.csdn.net/liaoxinmeng/article/details/5004743) 
[终端，虚拟终端和伪终端](http://www.linuxdiyf.com/linux/18188.html)  
[Linux 串行终端，虚拟终端，伪终端，控制终端，控制台终端的理解](http://blog.csdn.net/wdjhzw/article/details/39433373)  