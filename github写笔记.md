## 使用github写笔记  
remote llm 修改
* 在github建立一个blog仓库  
* 在这个仓库的设置里面添加自己电脑的ssh公钥。  
`ssh-keygen -t rsa -C "youremail@example.com"`生产，打开~/.ssh/id_rsa.pub复制里面的内容设置进github  
* 把github上的blog仓库clone下来  
`git clone ssh@github.com:leon0625/blog`  
* 修改后使用如下命令提交到github上，方便其他电脑查看  
```
git add test.md
git commit -m "我的修改日志"
git push origin master
```

修改日志
