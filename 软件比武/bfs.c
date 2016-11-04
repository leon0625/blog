/************************************************************
Copyright (C), 2016, Leon, All Rights Reserved.
FileName: bfs.c
Description: 
Author: Leon
Version: 1.0
Date: 
Function:

History:
<author>    <time>  <version>   <description>
 Leon
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true    1
#define false   0
#define QUEUE_SIZE  256

typedef int bool;

typedef struct point
{
    int x;
    int y;
    struct point *parent;
}point_t;

typedef struct queue
{
    point_t data[QUEUE_SIZE];
    int head;   //队列头下标，从0开始
    int tail;   
    int size;   //队列的及时长度
    int (*push)(struct queue *, point_t *);
    point_t *(*pop)(struct queue *);
    int (*empty)(struct queue *);
}queue_t; 

int queue_push(struct queue *q, point_t *point)
{
    if(!q || !point)
        return -1;

    if(q->size >= QUEUE_SIZE - 1)
    {
        fprintf(stderr, "queue size limit\n");
        return -1;
    }
    
    memcpy(&(q->data[q->tail]), point, sizeof(point_t));
    // 计算新的队尾位置
    q->tail = (q->tail + 1) % QUEUE_SIZE;

    q->size++;
    return 0;
}

int queue_empty(struct queue *q)
{
    if(!q)
        return -1;

    return (q->head == q->tail);
}

point_t *queue_pop(struct queue *q)
{
    if(!q)
        return NULL;

    if(queue_empty(q))
        return NULL;

    point_t *result = &(q->data[q->head]);
    //计算新的队首位置
    q->head = (q->head + 1) % QUEUE_SIZE;
    q->size--;
    return result;  /*返回指针，取到后需立即复制内容*/
}

/* 取得队头的节点 */
point_t *queue_front(struct queue *q)
{
    if(queue_empty(q))
        return NULL;

    return &(q->data[q->head]);
}

void queue_print(queue_t *q)
{
    int i = 0;
    if(q->head < q->tail)
    {
        for(i = 0; i < q->tail - q->head; i++)
        {
            printf("(%d, %d); ", q->data[q->head + i].x, q->data[q->head + i].y);
        }
    }
    
    if(q->head > q->tail)
    {
        for(i = 0; i < QUEUE_SIZE - q->head; i++)
        {
            printf("(%d, %d); ", q->data[q->head + i].x, q->data[q->head + i].y);
        }
        for(i = 0; i < q->tail; i++)
        {
            printf("(%d, %d); ", q->data[i].x, q->data[i].y);
        }
    }

    printf("\n");
}

queue_t *queue_init()
{
    queue_t *q;

    q = calloc(1, sizeof(queue_t));
    if(!q)
        return NULL;

    q->push = &queue_push;
    q->pop = &queue_pop;
    q->empty = &queue_empty;
    return q;
}

void queue_fini(queue_t *q)
{
    if(q)
        free(q);
}

#define WIDTH   (5*2) 
#define HEIGHT  (5*2)
/*int map[HEIGHT][WIDTH] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 1, 0,
    0, 1, 0, 0, 0,
    0, 1, 1, 0, 1,
    0, 0, 0, 0, 0,
};*/

int map[HEIGHT][WIDTH] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 1, 0,
    0, 1, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 1, 1, 0, 1, 0, 1, 1, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 1, 0,
    0, 1, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 1, 1, 0, 1, 0, 1, 1, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int isValid(point_t *point)
{
    if(point && point->x >=0 && point->x <= HEIGHT-1 && point->y >= 0
        && point->y <= WIDTH-1 && map[point->x][point->y] == 0)
        return 1;
    return 0;
}

void print_path(point_t *end)
{
    while(end)
    {
        printf("(%d, %d) ->> ", end->x, end->y);
        end = end->parent;
    }
    printf("\n");
}

bool BFS(void){
    queue_t *Q = queue_init();
    int i;
    point_t start = {0, 0, NULL}, end = {HEIGHT - 1, WIDTH - 1, NULL};
    point_t *curent = NULL, next = {0, 0, NULL};

    //用于标记颜色当visit[i][j]==true时，说明节点访问过，也就是黑色
    bool visit[WIDTH][HEIGHT] = {0};

    //四个方向
    int dir[][2] = {
        {0, 1}, {1, 0},
        {0, -1}, {-1, 0}
    };

    queue_push(Q, &start);
    visit[start.x][start.y] = true;

    while(!queue_empty(Q))
    {
        curent = queue_front(Q);
        //遍历四个方向上的点
        for(i = 0; i < 4; i++)
        {
            next.x = curent->x + dir[i][0];
            next.y = curent->y + dir[i][1];
            next.parent = curent;
            //判断点是否合法，是否已经访问
            if(isValid(&next) && visit[next.x][next.y] == false)
            {
                if(next.x == end.x && next.y == end.y)
                {
                    printf("oko\n");
                    //打印路径
                    print_path(&next);
                    return true;
                }
                queue_push(Q, &next);
                visit[next.x][next.y] = true;
            }
        }
        queue_pop(Q);
    }

    return false;


#if 0
    //初始状态将起点放进队列Q
    Q.push(Vs);
    visit[Vs.x][Vs.y] = true;//设置节点已经访问过了！

    while (!Q.empty()){//队列不为空，继续搜索！
        //取出队列的头Vn
        Vn = Q.front();
        Q.pop();

        for(i = 0; i < 4; ++i){
            Vw = Node(Vn.x+dir[i][0], Vn.y+dir[i][1]);//计算相邻节点

            if (Vw == Vd){//找到终点了！
                //把路径记录，这里没给出解法
                return true;//返回
            }

            if (isValid(Vw) && !visit[Vw.x][Vw.y]){
                //Vw是一个合法的节点并且为白色节点
                Q.push(Vw);//加入队列Q
                visit[Vw.x][Vw.y] = true;//设置节点颜色
            }
        }
    }
    return false;//无解
#endif
}

int main(int argc, char *argv[])
{
    BFS();
}