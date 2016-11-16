####Realtek linux sdk编译总结
#####一、编译方法
```shell
    tar -zxvf rtl819x.tar.gz  #解压sdk
    cd rtl819x/
    make menuconfig     #选择芯片方案,配置内核和应用进程
    make linux
    make users
    make romfs  #将可执行文件拷贝到romfs
    make image
    #也可使用make users_menuconfig，make linux_menuconfig来执行配置
```
#####二、升级文件介绍
```
    boot/rtl819x-bootcode-SDK-v3.4.6.7-96d-96e/btcode/boot --- boot升级文件
    rtl819x/image/root.bin ----------------------------------- romfs升级文件
    rtl819x/image/linux.bin ---------------------------------- 内核升级文件
    rtl819x/image/webpages-gw.bin ---------------------------- 页面升级文件
    rtl819x/image/fw.bin ------------------------------------- romfs,linux,webpage组合在一起的升级文件
    rt819x/image/config-gw-96d-92e.dat ----------------------- 配置文件
```
#####三、修改路由器的默认配置
>　　Realtek linux sdk的配置分三部分，硬件配置、默认配置和当前配置,都直接保存在flash上。页面恢复出厂设置时，拷贝默认配置区到当前配置区（和 flash reset效果一样）。路由器的默认配置放在rt819x/users/boa/defconfig/下，但默认情况下升级fw.bin是不会改变路由器的配置的。如果需要修改默认配置，可以通过下面两个方法实现  

1. 修改配置文件后缀为.bin，然后升级这个配置文件
2. 修改Makefile
    * 将硬件参数也一并修改
    ```makefile
    # users/boa/defconfig/Makefile, 去掉-no_hw参数
    ../tools/cvcfg-ap -no_hw $(CONFIG_FILE) $(CONFIG_DAT)
    ```
    * 将默认配置拼接到升级文件fw.bin中
    ```makefile
    # boards/rtl8196e/Makefile, 将下面的语句
    egrep "^CONFIG_APP_BOA=y" $(DIR_USERS)/.config > BOA.test; \
    if [ -s BOA.test ] ; then \
        cp $(DIR_USERS)/boa/html/$(WEBIMAGE_BIN) $(DIR_IMAGE)/$(WEBIMAGE_BIN); \
        cd $(DIR_USERS)/boa/defconfig; \
        mv *.dat $(DIR_ROOT)/boards/rtl8196e/image; \
        cd -; \
        $(MGBIN) -c -o $(FW_BIN) image/*.dat $(ROOT_BIN) $(WEBPAGE_BIN) $(LINUX_BIN); \
    # 修改为
    egrep "^CONFIG_APP_BOA=y" $(DIR_USERS)/.config > BOA.test; \
    if [ -s BOA.test ] ; then \
        cp $(DIR_USERS)/boa/html/$(WEBIMAGE_BIN) $(DIR_IMAGE)/$(WEBIMAGE_BIN); \
        cd $(DIR_USERS)/boa/defconfig; \
        mv *.dat $(DIR_ROOT)/boards/rtl8198/image; \
        cd -; \
        $(MGBIN) -c -o $(FW_BIN) image/*.dat $(ROOT_BIN) $(WEBPAGE_BIN) $(LINUX_BIN); \   # 加入了image/*.dat
    ```
**升级之后配置就会立即生效，变为配置文件里的配置。因为升级配置时当前配置区和默认配置区都改变了。**

