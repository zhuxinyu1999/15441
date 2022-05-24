#include "pool.h"

void pool_init(pool_t* pool) {
  pool->n = 0;
  pool->last_check_time = time(NULL);
  pool->info = calloc(MAX_CLIENT, sizeof(client_t));
}

void pool_deinit(pool_t* pool) {
  free(pool->info);
}

int pool_insert(pool_t* pool, int fd, int type, SSL* ssl, char* addr) {
  if (pool->n == MAX_CLIENT) {
    check(pool);
  }
  if (pool->n == MAX_CLIENT) {
    return -1;
  }
  int i;
  for (i = 0; i < MAX_CLIENT; ++i) {
    if (pool->info[i].fd == 0) {
      pool->info[i].fd = fd;
      pool->info[i].time = time(NULL);
      pool->info[i].type = type;
      pool->info[i].ssl = ssl;
      strcpy(pool->info[i].addr, addr);
      httpio_init(&(pool->info[i].httpio), fd, type, ssl);
      pool->info[i].pipe_fd = 0;
      ++pool->n;
      break;
    }
  }
  return i;
}

int Pool_insert(pool_t* pool, int fd, int type, SSL* ssl, char* addr) {
  int index = pool_insert(pool, fd, type, ssl, addr);
  if (index < 0) {
    err_return("no slot in client pool.\n");
  }
  return index;
}

void check(pool_t* pool) {
  time_t cur_time = time(NULL);
  int i;
  for (i = 0; i < MAX_CLIENT; ++i) {
    if (pool->info[i].type != EV_LISTEN_HTTP && 
        pool->info[i].type != EV_LISTEN_HTTPS && 
        cur_time - pool->info[i].time >= CLOSE_TIME) {
      Close(pool->info[i].fd);
      Close(pool->info[i].pipe_fd);
      if (pool->info[i].ssl != NULL) {
        SSL_free(pool->info[i].ssl);
      }
      pool_remove(pool, i);
    }
  }
  pool->last_check_time = cur_time;
}

void Check(pool_t* pool) {
  if (time(NULL) - pool->last_check_time >= CHECK_INTERVAL) {
    check(pool);
  }
}

void pool_remove(pool_t* pool, int index) {
  pool->info[index].fd = 0;
  pool->info[index].pipe_fd = 0;
  --pool->n;
}

void pool_update(pool_t* pool, int index) {
  pool->info[index].time = time(NULL);
}