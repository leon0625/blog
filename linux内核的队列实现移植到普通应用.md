# linux内核的队列实现移植  

[!TOC]  

`在c编程中有时会使用一些常用的数据结构，如队列。每次都手写一遍有些麻烦，写一个通用的比较好，而内核有实现队列，直接移植出来就好了。`  

## 内核的队列实现  
　　内核的队列实现在linux-2.6.32.68/kernel/kfifo.c和对应的kfifo.h中，主要接口罗列如下：  

```c
struct kfifo {
    unsigned char *buffer;  /* the buffer holding the data */
    unsigned int size;  /* the size of the allocated buffer */
    unsigned int in;    /* data is added at offset (in % size) */
    unsigned int out;   /* data is extracted from off. (out % size) */
    spinlock_t *lock;   /* protects concurrent modifications */
};

extern struct kfifo *kfifo_alloc(unsigned int size, gfp_t gfp_mask,
                 spinlock_t *lock);
extern void kfifo_free(struct kfifo *fifo);
static inline void kfifo_reset(struct kfifo *fifo);
static inline unsigned int kfifo_put(struct kfifo *fifo,
                const unsigned char *buffer, unsigned int len);
static inline unsigned int kfifo_get(struct kfifo *fifo,
                     unsigned char *buffer, unsigned int len);
static inline unsigned int kfifo_len(struct kfifo *fifo);                     
```

函数名字已非常清晰的表明了用途，无需多言。说一下，它的实现里面我觉得比较好的几个点。  

* **内核实现的亮点**  
较我自己以前实现的队列而言，在队列发生循环时，会通过取余的方式去更新队尾和队头下标。这样队尾下标有可能比队头下标小。计算队列长度时，相减又需要考虑此种情况。  
而内核in和out是一直递增的（因为是无符号，溢出也没关系），put和get时通过`fifo->in & (fifo->size - 1)`方式取余计算，分开拷贝。内核的put核心代码如下：    
```c
unsigned int __kfifo_put(struct kfifo *fifo,
            const unsigned char *buffer, unsigned int len)
{
    unsigned int l;

    len = min(len, fifo->size - fifo->in + fifo->out);

    /* first put the data starting from fifo->in to buffer end */
    l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
    memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);

    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(fifo->buffer, buffer + l, len - l);

    fifo->in += len;
    return len;
}
```

## 移植  
　　移植到普通应用，可以视情况去掉还是保留锁机制，在这儿就去掉了，再替换掉一些kmalloc这样的内核函数,实现is_power_of_2和roundup_pow_of_two两个函数就可以了。为了通用性，fifo.c里的代码都是对char型指针结合长度参数进行写入读取，而在自己使用时，可以在fifo.h里封装一下，方便自己的数据存放和读取。我在测试时封装成了存取int整数。  

***  

*fifo.c*  
```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fifo.h"

static inline int is_power_of_2(unsigned int n)
{
    return !((n - 1) & n);
}

unsigned int roundup_pow_of_two(unsigned int n)
{
    if(is_power_of_2(n))
        return n;

    int i = 32;
    while(i--)
    {
        if((n >> i) & 1)
            break;
    }

    n = 1 << (i + 1);
    return n;
} 

struct fifo *fifo_init(unsigned char *buffer, unsigned int size)
{
    struct fifo *fifo;

    fifo = malloc(sizeof(struct fifo));
    if (!fifo)
    {
        fprintf(stderr, "malloc %ubyte failed\n", sizeof(struct fifo));
        return NULL;
    }

    fifo->buffer = buffer;
    fifo->size = size;
    fifo->in = fifo->out = 0;

    return fifo;
}

struct fifo *fifo_alloc(unsigned int size)
{
    unsigned char *buffer;
    struct fifo *ret;

     /* round up to the next power of 2 */
    if(size > 0x80000000)
    {
        fprintf(stderr, "The size is too lager\n");
        return NULL;
    }
    size = roundup_pow_of_two(size);

    buffer = malloc(size);
    if (!buffer)
    {
        fprintf(stderr, "malloc %ubyte failed\n", size);
        return NULL;
    }

    ret = fifo_init(buffer, size);

    if (!ret)
        free(buffer);

    return ret;
}

void fifo_free(struct fifo *fifo)
{
    free(fifo->buffer);
    free(fifo);
}

unsigned int _fifo_put(struct fifo *fifo, const unsigned char *buffer, unsigned int len)
{
    unsigned int l;

    len = min(len, fifo->size - fifo->in + fifo->out);

    /* first put the data starting from fifo->in to buffer end */
    l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
    memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);

    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(fifo->buffer, buffer + l, len - l);

    fifo->in += len;

    return len;
}

unsigned int _fifo_get(struct fifo *fifo, unsigned char *buffer, unsigned int len)
{
    unsigned int l;

    len = min(len, fifo->in - fifo->out);

    /* first get the data from fifo->out until the end of the buffer */
    l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));
    memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + l, fifo->buffer, len - l);

    fifo->out += len;

    return len;
}


```

***

*fifo.h*  
```c
#ifndef _FIFO_H_
#define _FIFO_H_

struct fifo {
    unsigned char *buffer;  /* the buffer holding the data */
    unsigned int size;  /* the size of the allocated buffer */
    unsigned int in;    /* data is added at offset (in % size) */
    unsigned int out;   /* data is extracted from off. (out % size) */
};

#define min(a, b) ((a) > (b) ? (b) : (a))

extern struct fifo *fifo_alloc(unsigned int size);
extern void fifo_free(struct fifo *fifo);
extern unsigned int _fifo_put(struct fifo *fifo, 
                const unsigned char *buffer, unsigned int len);
extern unsigned int _fifo_get(struct fifo *fifo,
                unsigned char *buffer, unsigned int len);

static inline void _fifo_reset(struct fifo *fifo)
{
    fifo->in = fifo->out = 0;
}


static inline unsigned int _fifo_len(struct fifo *fifo)
{
    return fifo->in - fifo->out;
}

/****************************************************/
/* 根据需要进行封装，这儿我封装为保存int数
/****************************************************/

static inline void fifo_reset(struct fifo *fifo)
{
    _fifo_reset(fifo);
}

static inline unsigned int fifo_len(struct fifo *fifo)
{
    return _fifo_len(fifo)/4;
}

static inline unsigned int fifo_put(struct fifo *fifo, int n)
{
    return _fifo_put(fifo, (unsigned char *)&n, sizeof(int));
}

static inline unsigned int fifo_get(struct fifo *fifo, int *n)
{
    return _fifo_get(fifo, (unsigned char *)n, sizeof(int));
}

#endif

```