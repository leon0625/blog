#!/bin/bash

# 测试8数码程序的运行时间，最大步数

echo "编译"
gcc 88八方块移动游戏用例生成代码.c -o create_number
gcc 8_number_sort.c -o 8_number_sort

echo "测试"
for((i=0;i<1000;i++))
do
    ./create_number 3 3 | xargs ./8_number_sort >> results.txt
done
