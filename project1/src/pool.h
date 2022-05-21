#ifndef _POOL_H
#define _POOL_H

#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include "common.h"

#define EV_LISTEN_HTTP 1
#define EV_LISTEN_HTTPS 2
#define EV_HTTP 3
#define EV_HTTPS 4

#define ADDR_LENGTH (1 << 4)

#define MAX_CLIENT (1 << 10)
#define CLOSE_TIME (1 << 12)
#define CHECK_INTERVAL (1 << 12)

/* 记录不同的fd + 类型 + 访问时间 */
typedef struct {
  int fd;
  int type;
  time_t time;
  char addr[ADDR_LENGTH];
} client_t;

/* pool */
typedef struct {
  /* 池内fd数量 */
  int n;
  int last_check_time;
  client_t* info;
} pool_t;

/* 未使用空间的client初始化为0 */
void pool_init(pool_t* pool);

/* deinit */
void pool_deinit(pool_t* pool);

/* pool中插入fd，返回插入的位置 */
int pool_insert(pool_t* pool, int fd, int type, char* addr);
int Pool_insert(pool_t* pool, int fd, int type, char* addr);

/* 将pool中过久没有数据传输的连接关闭 */
void check(pool_t* pool);
void Check(pool_t* pool);

/* 将pool中index处连接关闭，并删除记录 */
void pool_remove(pool_t* pool, int index);

/* index处的连接有了新的数据传输，更新时间 */
void pool_update(pool_t* pool, int index);


#endif