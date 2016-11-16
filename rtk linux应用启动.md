##### 启动

###### 1、rcS启动脚本

```shell
#!/bin/sh

ifconfig lo 127.0.0.1

CINIT=1

hostname rlx-linux

mount -t proc proc /proc
mount -t ramfs ramfs /var

mkdir /var/tmp
mkdir /var/web
mkdir /var/log
mkdir /var/run
mkdir /var/lock
mkdir /var/system
mkdir /var/dnrd
mkdir /var/lib
mkdir /var/lib/misc

###for tr069
mkdir /var/cwmp_default
mkdir /var/cwmp_config

if [ ! -f /var/cwmp_default/DefaultCwmpNotify.txt ]; then
	cp -p /etc/DefaultCwmpNotify.txt /var/cwmp_default/DefaultCwmpNotify.txt 2>/dev/null
fi

##For miniigd
mkdir /var/linuxigd
cp /etc/tmp/pics* /var/linuxigd 2>/dev/null

##For pptp
mkdir /var/ppp
mkdir /var/ppp/peers

#smbd
mkdir /var/config
mkdir /var/private
mkdir /var/tmp/usb

#snmpd
mkdir /var/net-snmp

cp /bin/pppoe.sh /var/ppp/true
echo "#!/bin/sh" > /var/ppp/true
#echo "PASS"     >> /var/ppp/true

#for console login
cp /etc/shadow.sample /var/shadow

#extact web pages
cd /web
#flash extr /web
cd /
 
mkdir -p /var/udhcpc
mkdir -p /var/udhcpd
cp /bin/init.sh /var/udhcpc/eth0.deconfig
echo " " > /var/udhcpc/eth0.deconfig
cp /bin/init.sh /var/udhcpc/eth1.deconfig
echo " " > /var/udhcpc/eth1.deconfig
cp /bin/init.sh /var/udhcpc/br0.deconfig
echo " " > /var/udhcpc/br0.deconfig
cp /bin/init.sh /var/udhcpc/wlan0.deconfig
echo " " > /var/udhcpc/wlan0.deconfig

if [ "$CINIT" = 1 ]; then
startup.sh
fi

# for wapi certs related
mkdir /var/myca
# wapi cert(must done before init.sh)
cp -rf /usr/local/ssl/* /var/myca/ 2>/dev/null
# loadWapiFiles >/dev/null 2>&1
 
# for wireless client mode 802.1x
mkdir /var/1x
cp -rf /usr/1x/* /var/1x/ 2>/dev/null
 
# Start system script
init.sh gw all
 
# modify dst-cache setting
echo "8192" > /proc/sys/net/ipv4/route/max_size
echo "180" > /proc/sys/net/ipv4/route/gc_thresh
echo 20 > /proc/sys/net/ipv4/route/gc_elasticity
# echo 35 > /proc/sys/net/ipv4/route/gc_interval
echo 60 > /proc/sys/net/ipv4/route/secret_interval
# echo 10 > /proc/sys/net/ipv4/route/gc_timeout
 
# echo "4096" > /proc/sys/net/nf_conntrack_max
echo "4096" > /proc/sys/net/netfilter/nf_conntrack_max
echo "600" > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established
echo "20" > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_time_wait
echo "20" > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_close
echo "90" > /proc/sys/net/ipv4/netfilter/ip_conntrack_udp_timeout
echo "120" > /proc/sys/net/ipv4/netfilter/ip_conntrack_udp_timeout_stream
echo "90" > /proc/sys/net/ipv4/netfilter/ip_conntrack_generic_timeout
echo "1048576" > /proc/sys/net/ipv4/rt_cache_rebuild_count
echo "32" > /proc/sys/net/netfilter/nf_conntrack_expect_max

#echo 1 > /proc/sys/net/ipv4/ip_forward #don't enable ip_forward before set MASQUERADE
#echo 2048 > /proc/sys/net/core/hot_list_length

# start web server
boa

```

　　其中另外执行的`startup.sh`脚本，主要作用是检查一些flash参数是否正确，不正确则进行恢复处理。

　　语句`init.sh gw all`其实是执行这个`sysconf init gw all`，而sysconf是什么命令呢，在代码里find一下，可以很快知道这是在`./users/boa/system/`下编译出来的。

###### 2、sysconf

　　其中`setinit()`函数配置网络接口，系统变量，起进程等，tr069也是这里起的`start_tr069()`。

据我观察，sysconf命令主要做的事情就是配置系统拉其他进程起来。串口打印可见一斑：

```shell
sysconf init gw all 	/* 这条命令相当于重启整个应用进程了 */
sysconf wlanapp kill wlan0 
sysconf disc option 
sysconf conn ppp ppp0
sysconf firewall 
```

###### 3、打开cwmpClient进程（TR069）的调试开关

```shell
flash get CWMP_FLAG	# 查看这个值是多少，CWMP_FLAG_DEBUG_MSG = 1
flash set CWMP_FLAG 33
```

###### 4、ppp的配置文件生成

　　在代码里面看到有两个pppoe.sh，这个./users/script/pppoe.sh里有生成options，但编译出来的pppoe.sh是用的./users/script/cinit/pppoe.sh，代码里面users/boa/system/set_wan.c set_pppoe()里也有生成的地方

###### 5、users/rc/reload.c

　　身处rc目录下的单独文件reload.c，原以为和ecos里的reload目录一样，起着编译启动的关键动作，但这个reload.c作用却不那么重要。它作为单独进程在sysconf进程的`start_wlan_by_schedule()`函数拉起，作用就如同拉起它的函数名一般。

###### 6、IAPP进程

　　Inter-Access Point Protocol。IAPP协议也即802.11f是在数据链路层解决无线局域网用户在接入点AP间切换的问题。拉起位置sysconf进程的`setWlan_Applications()`函数，在`wan_connect()`也会重新拉起该进程。

###### 7、fwd进程

　　也在sysconf里拉起，升级时有用。页面升级的时候，boa把升级文件内存共享给这个进程，然后由这个进程来执行写flash动作。这是为什么，我也不知道，是怕boa进程写太久卡住引起什么异常所以换个进程写吗？

###### 8、wscd进程

　　wps相关进程，代码没开源，直接给的可执行文件。

###### 9、iwcontrol进程

　　wps相关进程。

###### 10、lld2d进程

　　作用不明，代码未开源。

###### 11、timelycheck进程

​	桥模式下定时发送arp冲突检查的作用。但目前里面的宏并没开，此进程没什么作用。

###### 12、boa进程

　　http服务器，应该是里面最重要的进程了。连sysconf都糅杂在这boa代码文件夹里面。