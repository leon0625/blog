#include<cstdio>
#include<iostream>
using namespace std;

int p[]={1, 1, 2, 6, 24, 120, 720, 5040, 40320};
int encode(int* s){//升序做为正序
  int x=0;
  int i,j;
  for(i=0;i<9;i++){
    int k =s[i];
    for(j=0;j<i;j++)
      if(s[j]<s[i])k--;
    x+=k*p[8-i];
  }
  return x;
}

void decode(int x,int* s){
  int i,j;
  for(i=0; i<9;i++){
    s[i]=x/p[8-i];
    x%=p[8-i];
  }
  for(i=8;i>=0;i--)
    for(j=8;j>i;j--)
      if(s[j]>=s[i])s[j]++;
}
int main()
{
    int a[9] = {8,7,6,5,4,3,2,1,0};
    int t[9];
    printf("key=%d\n", encode(a));
    decode(encode(a),t);//编码后解码，若得原码则可认为是正确的
    for(int i=0; i<9; i++)cout<<t[i]<<' ';
    cout<<endl;
    return 0;
}