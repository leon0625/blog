#!/bin/bash  
# echo "calculate n!"  
# echo -n "n=?"  
# read n  
  
# r=1  
# for ((i=2; i <= n;i++)) ; do  
#     r=`echo $r*$i | bc`  
#     r=`echo $r | tr -d ["\134\n\040"]`  
# done  
      
# echo "$n!=$r" 

  
echo To calculate n!  
echo -n n=  
read nn  
  
rad=10000  
len=1  
buffs[1]=1  
for (( i=1; i <= nn; i++ )); do  
    let cc=0  
    for ((j=1; j<=len; j++)) ; do  
        let tt=buffs[j]*i+cc  
        let buffs[j]=tt%rad  
        let cc=tt/rad  
    done         
         
    if  [ $cc -gt 0 ] ; then  
        let len=len+1  
        let buffs[len]=cc  
    fi  
done  
  
echo -n ${nn}!=${buffs[$len]}  
for ((i=len-1; i>=1; i--)) ; do  
    echo -n `printf "%04d" ${buffs[$i]}`  
done  
echo  