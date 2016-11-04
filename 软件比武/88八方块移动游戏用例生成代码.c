#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define LLM 1

void getKeJiePingTu(int *a[], int vol, int col)
{
    int i = 0, j = 0;
    int *data = NULL;
    int maxnumber = vol*col-1;
    data = (int *)malloc(vol*col*sizeof(int));
    if(NULL == data)
    {
        printf("malloc data failed\n");
        return;
    }
    memset(data, 0x0, vol*col*sizeof(int));
    srand((int)time(0));
    for(i=0;i<maxnumber;++i)
    {
        data[i] = i;
        int replacei = rand()%(i+1);
        int t = data[i];
        data[i] = data[replacei];
        data[replacei] = t;
    }
    data[maxnumber] = maxnumber;
    //计?算?逆?序??对?数?y
    int coverPairCount = 0;
    for(i=0;i<maxnumber;++i)
    {
        for(j=i+1;j<maxnumber;++j)
        {
            if(data[i]>data[j])
                coverPairCount++;
        }
    }
    if((coverPairCount&1) == 1)
    {
        int t = data[maxnumber-1];
        data[maxnumber-1] = data[maxnumber-2];
        data[maxnumber-2] = t;
    }
    int index = 0;
    for(i=0;i<vol;++i)
    {
        for(j=0;j<col;++j)
        {
            a[i][j] = data[index++];
        }
    }
    free(data);
    return;
}

void free_array(int *a[], int vol)
{
    int i;

    for(i=0; i<vol; i++)
    {
        if(a[i] != NULL)
            free(a[i]);
    }
}
#if LLM
void print_array(int *a[], int vol, int col)
{
    int i, j;

    for(i=0; i<vol; i++)
    {
        for(j=0; j<col; j++)
            printf("%d", a[i][j]);
    }
    printf("\n");
}
#else
void print_array(int *a[], int vol, int col)
{
    int i, j;

    for(i=0; i<vol; i++)
    {
        for(j=0; j<col; j++)
            printf("\t%d", a[i][j]);
        printf("\n");
    }
}
#endif

int main(int argc, char *argv[])
{
    int vol, col;
    int **a = NULL, *int_p = NULL;
    int i;

    vol = atoi(argv[1]);//行数
    col = atoi(argv[2]);//列数
    // printf("vol = %d, col = %d\n", vol, col);

    a = (int **)malloc(vol*sizeof(int *));
    if(NULL == a)
    {
        printf("malloc a failed\n");
        return -1;
    }

    memset(a, 0x0, vol*sizeof(int *));
    for(i=0; i<vol; i++)
    {
        int_p = malloc(col*sizeof(int));
        if(NULL == int_p)
        {
            printf("<%d> malloc int_p failed\n", i);
            break;
        }
        memset(int_p, 0x0, col*sizeof(int));
        a[i] = int_p;
        int_p = NULL;
    }

    if(i == vol)
    {
        getKeJiePingTu(a, vol, col);
#if LLM
        print_array(a, vol, col);
#else
        print_array(a, vol, col);
#endif
    }
    free_array(a, vol);
    free(a);
    //printf("done!!!\n");
    return 0;
}
