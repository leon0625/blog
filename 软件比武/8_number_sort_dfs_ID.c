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

/*****************************************************************
 3X3的有9!=362880种状态，广搜最坏要搜索保存这么多中间状态。
使用深度优先的迭代加深算法
*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true    1
#define false   0
#define QUEUE_SIZE  (362880 + 1)
#define STATES_MAX  (362880 + 1) // 9! + 1
#define HEIGHT 3
#define WIDTH  3

#define abs(a) ((a) > 0 ? (a) : -(a))

typedef int bool;

typedef struct point
{
    int x;
    int y;
} point_t;

typedef struct location
{
    int state;
    int visited;
    struct location *parent;
}location_t;

typedef struct queue
{
    int data[QUEUE_SIZE];
    int head;   //队列头下标，从0开始
    int tail;
    int size;   //队列的及时长度
    int (*push)(struct queue *, int);
    int (*pop)(struct queue *);
    int (*empty)(struct queue *);
} queue_t;

int queue_push(struct queue *q, int state)
{
    if (!q)
        return -1;

    if (q->size >= QUEUE_SIZE - 1)
    {
        fprintf(stderr, "queue size limit\n");
        return -1;
    }

    q->data[q->tail] = state;
    // 计算新的队尾位置
    q->tail = (q->tail + 1) % QUEUE_SIZE;

    q->size++;
    return 0;
}

int queue_empty(struct queue *q)
{
    if (!q)
        return -1;

    return (q->head == q->tail);
}

int queue_pop(struct queue *q)
{
    if (!q)
        return 0;

    if (queue_empty(q))
        return 0;

    int result = q->data[q->head];
    //计算新的队首位置
    q->head = (q->head + 1) % QUEUE_SIZE;
    q->size--;
    return result;
}

/* 取得队头的节点 */
int queue_front(struct queue *q)
{
    if (queue_empty(q))
        return 0;

    return (q->data[q->head]);
}

queue_t *queue_init()
{
    queue_t *q;

    q = calloc(1, sizeof(queue_t));
    if (!q)
        return NULL;

    q->push = &queue_push;
    q->pop = &queue_pop;
    q->empty = &queue_empty;
    return q;
}

void queue_fini(queue_t *q)
{
    if (q)
        free(q);
}

static tp_cnt = 0;

int is_valid(point_t *point)
{
    if (point && point->x >= 0 && point->x <= HEIGHT - 1 && point->y >= 0
            && point->y <= WIDTH - 1)
        return 1;
    return 0;
}

/* 返回*/
point_t find_blank_block(int state)
{
    int i = 8;
    point_t p = {0, 0};

    while (state % 10)
    {
        state /= 10;
        i--;
    }
    p.x = i / 3;
    p.y = i % 3;
    return p;
}

//返回静态数组
int *state_convert_array(int state)
{
    static int array[9] = {0};
    int i = 8;

    memset(&array, 0x0, sizeof(array));
    for (i = 8; i >= 0; i--)
    {
        array[i] = state % 10;
        state /= 10;
    }
    return &array[0];
}

int update_state(int state, point_t *old_blank, point_t *new_blank)
{
    int old = old_blank->x * 3 + old_blank->y;
    int new = new_blank->x * 3 + new_blank->y;
    int *array = state_convert_array(state);
    int i = 0;
    int new_state = 0;

    for (i = 0; i < 9; i++)
    {
        if (i == old)
            new_state = new_state * 10 + array[new];
        else if (i == new)
            new_state = new_state * 10 + array[old];
        else
            new_state = new_state * 10 + array[i];
    }

    return new_state;
}

// 全排列的完美hash函数
int p[] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320};
//生成key
int encode(int* s) 
{
    int x = 0;
    int i, j;
    for (i = 0; i < 9; i++) {
        int k = s[i];
        for (j = 0; j < i; j++)
            if (s[j] < s[i])k--;
        x += k * p[8 - i];
    }
    return x;
}

//从key生成对应的序列
void decode(int x, int* s) 
{
    int i, j;
    for (i = 0; i < 9; i++) {
        s[i] = x / p[8 - i];
        x %= p[8 - i];
    }
    for (i = 8; i >= 0; i--)
        for (j = 8; j > i; j--)
            if (s[j] >= s[i])s[j]++;
}

location_t map[STATES_MAX] = {0};

int is_visit(int state)
{
    int *array = state_convert_array(state);
    int key = encode(array);

    if(map[key].visited == 1)
        return 1;
    return 0;
}

int visited(int state)
{
    int *array = state_convert_array(state);
    int key = encode(array);

    if(key < 0 || key > STATES_MAX)
    {
        printf("hash error\n");
        return -1;
    }
    map[key].state = state;
    map[key].visited = 1;
    return 0;
}

void print_search_step_cnt()
{
    int i = 0;
    int cnt = 0;

    for (i = 0; i < STATES_MAX; i++)
    {
        if (map[i].visited != 0)
            cnt++;
    }
    printf("search step total: %d\n", cnt);
}

void print_best_path(location_t *location)
{
    int i = 0;
    while(location)
    {
        printf("%09d\n", location->state);
        location = location->parent;
        i++;
    }
    printf("[%d]\n", i);
}

int BFS(char *input) {
    int start_state = atoi(input);
    int end_state = 123456780;
    int curent_state = 0;
    int next_state = 0;
    int i = 0;
    point_t blank_block = {0, 0}, new_blank = {0, 0};
    queue_t *Q = queue_init();
    int next_key = 0, curent_key = 0;

    //四个方向
    int dir[][2] = {
        {0, 1}, {1, 0},
        {0, -1}, { -1, 0}
    };

    if(start_state == end_state)
    {
        printf("llm->%s(%d) ok\n", __FUNCTION__, __LINE__);
        return 0;
    }

    queue_push(Q, start_state);
    visited(start_state);
    while (!queue_empty(Q))
    {
        curent_state = queue_front(Q);
        // printf("%d\n", curent_state);
        //找到可以移动的方块，在四个方向上移动
        blank_block = find_blank_block(curent_state);
        for (i = 0; i < 4; i++)
        {
            new_blank.x = blank_block.x + dir[i][0];
            new_blank.y = blank_block.y + dir[i][1];
            if (is_valid(&new_blank))
            {
                next_state = update_state(curent_state, &blank_block, &new_blank);
                
                // state_convert_array返回静态数组指针，及时存一下
                next_key = encode(state_convert_array(next_state));
                curent_key = encode(state_convert_array(curent_state));
                if (next_state == end_state)
                {
                    visited(next_state);
                    printf("ok\n");
                    //打印搜索状态数
                    print_search_step_cnt();
                    //打印最短步数
            
                    map[next_key].parent = &map[curent_key];
                    print_best_path(&map[encode(state_convert_array(next_state))]);
                    return 0;
                }

                if (!is_visit(next_state))
                {
                    visited(next_state);
                    queue_push(Q, next_state);
                    map[next_key].parent = &map[curent_key];
                }
            }
        }
        queue_pop(Q);
    }
    print_search_step_cnt();
    printf("不能完成\n");
    return 0;
}

/*********************** DFS ************************/

int finded(int state)
{
    return state == 123456780;
}

int clear_visited(int state)
{
    int *array = state_convert_array(state);
    int key = encode(array);

    if(key < 0 || key > STATES_MAX)
    {
        printf("hash error\n");
        return -1;
    }
    map[key].state = 0;
    map[key].visited = 0;
    return 0;
}

point_t find_point(int state, int num)
{
    int i = 8;
    point_t p = {0, 0};

    while (state % 10 != num)
    {
        state /= 10;
        i--;
    }
    p.x = i / 3;
    p.y = i % 3;
    return p;
}

/* 当前状态需要恢复的最短距离，瞎想的，不能确保正确 */
int MHD_length(int state)
{
    point_t point1, point2;
    int i, j;
    int *array;
    int len = 0;

    for(i = 0; i < 9; i++)
    {
        array = state_convert_array(state);
        if(array[i] == (i + 1)%9)
            continue;
        point1.x = i / 3;
        point1.y = i % 3;
        point2 = find_point(state, (i + 1)%9);
        len = len + abs(point2.x - point1.x) + abs(point2.y - point1.y);
        state = update_state(state, &point1, &point2);
    }
    return len;
}

int DFS(int state, int max_deep, int cur_deep)
{  
    point_t blank_block, new_blank;
    int ret, i;
    int dir[][2] = {
        {0, 1}, {1, 0},
        {0, -1}, { -1, 0}
    };
    int next_state;

    if(finded(state))    //达到终点
    {
        // 搜到了，退出   
        printf("OK! [%d]\n", cur_deep);
        return 0;
    }

    if(cur_deep + MHD_length(state) > max_deep)
    {
        // 达到最大深度，退出搜索
        return -1;
    }

    // visited(state);    //标记节点被访问
    // 遍历叶子节点
    blank_block = find_blank_block(state);
    for (i = 0; i < 4; i++)
    {
        new_blank.x = blank_block.x + dir[i][0];
        new_blank.y = blank_block.y + dir[i][1];
        if (!is_valid(&new_blank))
            continue;
        next_state = update_state(state, &blank_block, &new_blank);
        visited(next_state);
        ret = DFS(next_state, max_deep, cur_deep + 1);
        clear_visited(next_state);
        if(ret == 0)
            return 0;   //递归返回
    }
    return -2;
}

/*
    第一代叠加加深的DFS，搜123450678这种情况，需要用时0.4s，深度14。当深度到达17时，搜索速度变得很慢很慢，无法满足需求。注意当无解时，会陷入死循环，所以需要判断是否有解。

    设计一个函数，计算当前局面需要恢复的最少步数min。如果当前cur_deep + min > max_deep，则返回，不再继续这条错误的路线，即剪枝。

    第二代IDA*算法，效率明显提高，对求解071534628（最短24步）可以在3s运算出，也算很大的进步了，但运气成分明显。对于求解012345678（最短22步）需要20多秒。

    深度加深时每次都从头开始遍历，这太浪费了，如果能接着之前的状态继续跑不是更好。

-------------
对于广度优先似乎也可以加一个评估函数，如果下一状态比上一状态糟糕，那么放弃这条路线    
*/

int ID_DFS(int start_state)
{
    int i = 0;
    int ret;

    for(i = 0; i < 30; i++)
    {
        printf("i = %d\n", i);
        visited(start_state);
        ret = DFS(start_state, i, 0);
        if(ret == 0)
            break;
        else if(ret == -1)
            continue;
        else
            continue;
    }
}

int main(int argc, char *argv[])
{
    if(argc < 2)
        return -1;
    // BFS(argv[1]);
    ID_DFS(atoi(argv[1]));
}