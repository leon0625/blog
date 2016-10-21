# web页面文件的缓存处理  

## 一、需要解决什么问题  
　　解决客户升级软件后每次都需要清除浏览器缓存的麻烦问题  

## 二、分析问题  
* 浏览器缓存的详细内容见http://www.cnblogs.com/lyzg/p/5125934.html  
* 具体问题  
　　用chrome测试了下AC6的页面，发现主控页面main.html没有缓存，因为页面里面添加了无缓存的标签`<meta http-equiv="pragma" content="no-cache">`。里面浮动层页面例如iptv.html在请求时会以后缀随机数的形式访问也没有缓存。缓存的都是资源文件（js,css,图片文件）。  
　　要让类似logo.png的资源文件及时刷新，解决方法都是在请求时添加后缀，问题是添加什么后缀。添加随机数版本号这些都可以，最好的办法是添加该资源文件的内容摘要，这样资源文件既能及时更新又能保证访问速度。  
* 需要处理哪些内嵌的资源文件呢  
    1. html里嵌的资源文件
    `<link href="css/reasy-ui.css" rel="stylesheet">`  
    `<script src="lang/b28n_async.js"></script>`    

    2. css里的资源文件  
    `.index-body .mastbody { background: url(../img/shadow.jpg) no-repeat center 100%; }`  

    3. js里面嵌的资源文件  
    `$(".loadding-ok img").attr("src", "./img/ok_connected.png")`  



思路：
    观察来看，从之前提出的只分析html里的href和src明显不够。会漏掉css和js里的资源文件。
    缓存的文件为js/css/图片，因此我觉得可以直接使用文件名替换的暴力解决办法。
    遍历页面文件，如果为资源文件，则对这个文件名进行字符替换。替换的文件包括html/css/js文件。

shell脚本处理速度偏慢，考虑写个C程序提高效率


http://www.cnblogs.com/lyzg/p/5125934.html

```shell
#!/bin/bash

# 压缩页面代码脚本，leon
# 2016年10月14日14:20:55



path=$1 #输入的页面代码路径

if [ "$1" = "" ]; then
    echo "args err"
    echo "help: ./no_cache [web_path]"
    echo "example: ./no_cache ./web"
    exit -1
fi

path=${path/%\//} #去掉末尾的'/'
web_floder=${path##*/}  
save_floder=${web_floder}_nocache
save_path=${path/$web_floder/$save_floder}
mkdir -p $save_path

cp -rf $path/* $save_path
savepath_list=`find $save_path -type f ! -path "*.svn*"`

cnt1=0
cnt2=0

function replace()
{
    src=$1
    dest=$2

    for filepath in $savepath_list
    do
        case $filepath in 
            *.html|*.js|*.css)
                sed -i "s/$src/$dest/g" $filepath
                ((cnt2++))
                ;;
        esac
    done
}

for filepath in $savepath_list
do
    case $filepath in 
        *.js|*.css|*.png|*.jpg|*.gif)
            # 遍历文件html,css,js文件替换文件名
            name=${filepath##*/}
            md5=`md5sum $filepath | awk '{print $1}'`
            newname=$name?$md5
            replace $name $newname
            ((cnt1++))
            ;;
    esac
done

echo "#####################################"
echo $cnt1  $cnt2
```

