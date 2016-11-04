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

 9组用例通过
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

// typedef struct position
// {
//     point_t point;
//     struct position *parent;
// }position_t;

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

// 用例1， 3
// #define WIDTH   (5) 
// #define HEIGHT  (5)

// char map[HEIGHT][WIDTH] = {
//     {'.', '.', '.', '.', 'L'},
//     {'.', '#', '#', '#', '.'},
//     {'b', '#', 'b', '#', 'a'},
//     {'#', '#', '.', '#', '#'},
//     {'.', '.', '.', 'Q', 'a'},
// };

// 用例2， -1
// #define WIDTH   (5) 
// #define HEIGHT  (5)

// char map[HEIGHT][WIDTH] = {
//     {'.', '.', '.', '.', 'L'},
//     {'.', '#', '#', '#', '.'},
//     {'.', '#', '.', '#', '.'},
//     {'#', '#', '.', '#', '#'},
//     {'.', '.', '.', 'Q', '.'},
// };

// 用例3， 7
// #define WIDTH   (4) 
// #define HEIGHT  (4)

// char map[HEIGHT][WIDTH] = {
//         {'Q', 'a', '.', '.'},
//         {'b', '#', '.', '.'},
//         {'.', 'a', '#', '.'},
//         {'.', 'b', '#', 'L'},
// };

// 用例4， 8
// #define WIDTH   (4) 
// #define HEIGHT  (4)
// char map[HEIGHT][WIDTH] = {
//     {'Q', 'a', '.', '.'},
//     {'#', '#', '.', '.'},
//     {'.', 'a', '#', '.'},
//     {'.', '#', '#', 'L'},        
// };

// 用例5， 5
// #define WIDTH   (4) 
// #define HEIGHT  (5)
// char map[HEIGHT][WIDTH] = {
//     {'.', '.', '.', '.'},
//     {'.', '#', 'a', '.'},
//     {'#', 'Q', '#', '.'},
//     {'#', '.', '#', '.'},
//     {'.', 'a', 'L', '.'},       
// };

// 用例6， 2
// #define WIDTH   (5) 
// #define HEIGHT  (5)
// char map[HEIGHT][WIDTH] = {
//     {'.', '.', 'Q', 'a', '.'},
//     {'.', '.', '.', '.', '.'},
//     {'.', '.', '.', '.', '.'},
//     {'.', '.', 'L', 'a', '.'},
//     {'.', '.', '.', '.', '.'},       
// };

// 用例7  ，13
// #define WIDTH   (9) 
// #define HEIGHT  (8) 
// char map[HEIGHT][WIDTH] = {
//     {'.', '.', '.', 'e', '.', '#', '#', '.', '.'},
//     {'g', '.', '.', '#', '#', 'h', '#', '.', 'b'},
//     {'#', '#', '#', 'h', 'e', '.', '#', '.', 'd'},
//     {'L', 'a', '#', 'Q', '#', '#', '#', '.', 'c'},
//     {'.', 'b', '#', '#', 'f', '#', '#', '.', '.'},
//     {'.', '#', '#', 'd', '#', 'f', '#', '#', '#'},
//     {'#', '#', '.', '.', '.', '#', 'g', '.', '#'},
//     {'#', '#', '.', 'a', '#', 'c', '.', '.', '.'},
// };

// 用例8， 4
#define WIDTH   (5) 
#define HEIGHT  (5)
char map[HEIGHT][WIDTH] = {
    {'.', '.', '.', '#', 'L'},
    {'.', '#', '.', '#', 'a'},
    {'.', '#', 'a', '#', 'Q'},
    {'.', '#', '#', '#', '.'},
    {'.', '.', '.', '.', '.'},      
};

//用例9， -1
// #define WIDTH   (4) 
// #define HEIGHT  (4) 
// char map[HEIGHT][WIDTH] = {
//     {'L', '#', 'c', 'b'},
//     {'a', 'c', '#', '#'},
//     {'#', '#', 'b', 'a'},
//     {'#', '#', 'Q', '#'},
// };


static tp_cnt = 0;

int isValid(point_t *point)
{
    if(point && point->x >=0 && point->x <= HEIGHT-1 && point->y >= 0
        && point->y <= WIDTH-1 && map[point->x][point->y] != '#')
        return 1;
    return 0;
}

void print_path(point_t *end)
{
    int i = 0;
    while(end)
    {
        printf("(%d, %d) ->> ", end->x, end->y);
        end = end->parent;
        i++;
    }
    printf("\n");
    printf("%d!\n", i - 1);
}

int init_map(point_t *start, point_t *end)
{
    int i, j;
    for(i = 0; i < HEIGHT; i++)
        for(j = 0; j <WIDTH; j++)
        {
            if(map[i][j] == 'L')
            {
                start->x = i;
                start->y = j;
            }
            if(map[i][j] == 'Q')
            {
                end->x = i;
                end->y = j;
            }
        }

    printf("start:(%d,%d), end(%d,%d)\n", start->x, start->y, end->x, end->y);
    return 1;
}

point_t search_tp(point_t *tp)
{
    int i, j;
    point_t tp2;
    for(i = 0; i < HEIGHT; i++)
        for(j = 0; j <WIDTH; j++)
        {
            if(map[i][j] == map[tp->x][tp->y] && (i != tp->x || j != tp->y))
            {
                tp2.x = i;
                tp2.y = j;
            }
        }
    // printf("TP: [%c] (%d,%d)>>>(%d,%d)\n", map[tp->x][tp->y], tp->x, tp->y, tp2.x, tp2.y);
    tp_cnt++;
    return tp2;
}

bool BFS(void){
    queue_t *Q = queue_init();
    int i;
    point_t start = {0, 0, NULL}, end = {HEIGHT - 1, WIDTH - 1, NULL};
    point_t *curent = NULL, next = {0, 0, NULL};
    point_t tp;

    init_map(&start, &end); //初始化开始和结束位置
    //用于标记颜色当visit[i][j]==true时，说明节点访问过，也就是黑色
    bool visit[HEIGHT][WIDTH]= {0};

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
        // printf("(%d, %d), ", curent->x, curent->y);
        
        //遍历四个方向上的点
        for(i = 0; i < 4; i++)
        {
            next.x = curent->x + dir[i][0];
            next.y = curent->y + dir[i][1];
            //判断点是否合法，是否已经访问
            if(isValid(&next))
            {
                if(next.x == end.x && next.y == end.y)
                {
                    printf("oko\n");
                    //打印路径
                    next.parent = curent;
                    print_path(&next);
                    return true;
                }

                if(map[next.x][next.y] >= 'a' && map[next.x][next.y] <= 'z')
                {
                    // 标记传送门的终点为已访问
                    tp = search_tp(&next);
                    if(visit[tp.x][tp.y] == false)
                    {
                        tp.parent = curent;
                        visit[tp.x][tp.y] = true;
                        queue_push(Q, &tp);
                    }
                }
                else if(visit[next.x][next.y] == false)
                {
                    visit[next.x][next.y] = true;
                    next.parent = curent;
                    queue_push(Q, &next);
                }
            }
        }
        queue_pop(Q);
    }

    printf("-1!\n");
    return false;

}

int main(int argc, char *argv[])
{
    BFS();
}