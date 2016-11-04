#!/bin/bash

# 计算一个程序的平均运行时间

cmd="echo"
count=1

# 执行程序
if [[ -n $1 ]]; then
    cmd=$1
fi

# 执行次数
if [[ -n $2 ]]; then
    count=$2
fi

sum_time=0
for((i=0; i<$((count)); i++))
do
    /usr/bin/time -o time.file -f '%U + %S' $cmd > /dev/null
    output=`cat time.file | bc`
    sum_time=`echo "$sum_time + $output" | bc`   
done

rm time.file
echo "scale=3; $sum_time / $count " | bc


