# linux下限制一个文件夹大小的方法  

>　　linux上有时由于一些特殊的用途，我们需要限制一个文件夹的大小。核心方法都是把需要限制的文件夹挂载在一个大小固定的分区上，下面有两种方法可以实现:  

1. 使用ramdisk  
linux可以把一部分内存mount为分区使用，通常为称为ramdisk，分为ramdisk, ramfs, tmpfs。可以一条命令实现我们的需求：  
`mount none testdir -t tmpfs -o size=1m`  
这样testdir目录最大可使用大小就是1MB了，但注意这是内存mount为分区，所以系统关闭后，里面的文件都会失去。  

2. 使用镜像文件挂载的方式  
```shell
# 生成一个10M的文件
dd if=/dev/zero of=disk.img bs=1M count=10
# 把生成的文件虚拟为块设备
losetup /dev/loop0 disk.img
# 格式化设备
mkfs.ext4 /dev/loop0
# 挂载
mount disk.img testdir
# 卸载
umount testdit
# 卸载loop设备与文件的关联
losetup -d /dev/loop0
```
　　这种方式系统重启之后只需要再mount一下就可以，存在文件夹的文件都还在。因为挂载在本地磁盘上，可限制的文件夹大小不受内存大小限制，可以更大些。　　