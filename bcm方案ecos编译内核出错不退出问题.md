#### bcm方案ecos编译内核出错，编译不退出  

> 按理说Makefile里任何操作出错，都会退出  

这是编译内核的那段Makefile  


```Makefile
kernel:	.config
	@echo "Build eCos kernel library"
	if [ ! -d $(BSPDIR)/kernel ] ; then \
		mkdir -p $(BSPDIR)/kernel ; \
	fi
	if [ ! -e $(BSPDIR)/kernel/ecos.ecc ] ; then \
		cp -f $(BSPDIR)/arch/$(ARCH)/$(CONFIG_ECC) $(BSPDIR)/kernel/ecos.ecc ; \
	fi
	if [ ! -d $(BSPDIR)/kernel/ecos_install ] ; then \
		cp -f $(BSPDIR)/kernel/ecos.ecc $(BSPDIR)/kernel/ecos_work.ecc ; \
		mkdir -p $(BSPDIR)/kernel/ecos_build ; \
		cd $(BSPDIR)/kernel/ecos_build ; \
		$(ECOS_TOOLS)/ecosconfig --config=$(BSPDIR)/kernel/ecos_work.ecc --prefix=$(BSPDIR)/kernel/ecos_install tree ; \
		make -C $(BSPDIR)/kernel/ecos_build ; \
		rm -f $(BSPDIR)/kernel/ecos_work.ecc ; \
	fi 
	
```

　　if里面的语句通过`;`连接起来，这样`make -C $(BSPDIR)/kernel/ecos_build  `出错后，会继续执行`rm -f`语句并且执行成功，在shell判断里认为这整个语句执行结果是对的，所以不报错，继续编译。  

　　因此针对这种情况最好使用`&&`来连接。在这里可以把`make -C $(BSPDIR)/kernel/ecos_build`这句话拿出来单独成为一行就可以了。  



测试验证代码：这样就会发现ls命令得到了执行  

```makefile
all:
	error; date
	ls
```

**include target问题**  

```Makefile
inlcude ecos.mk
ecos.mk :
	XXXX
```

　　形如上面的语句，当include的文件没找到时会报错，但不会退出make过程，进而执行ecos.mk目标下面的动作，认为这会生成ecos.mk目标。如果改为`-include ecos.mk`，那么即便ecos.mk目标的动作执行失败，也不会使make过程退出。  

**如何直接编译内核：**  
在编译bcm的代码时有两个问题，一是内核编译错误不立即报错退出，二是修改了内核直接make并不会编译内核。第一个问题上面已经有解决方案，第二个问题可以有个临时的解决的方案：  
修改src/ecos/router/Makefile，添加一个编译内核的target，如下：
```makefile
kernel:
	$(MAKE) -C $(BSPDIR)/kernel/ecos_build
```
修改了内核直接`make kernel && make`即可，比之前`make clean && make`节省不少时间  

