#!/usr/bin/python
#coding=utf-8

# 问题:打印延迟，会一起打印出来

import sys,tinify

print("Now will compress image...")
sys.stdout.flush()
tinify.key = "7vXsNCs64jLDs2sPi-UO6Vx9_os1I__5"

for i in range(1, len(sys.argv)):
    print("compree image: " + sys.argv[i])
    sys.stdout.flush()
    source = tinify.from_file(sys.argv[i])
    source.to_file(sys.argv[i])
    
print("All done")

