#ifndef _POOL_H
#define _POOL_H

#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <openssl/ssl.h>
#include "common.h"
#include "httpio.h"

#define ADDR_LENGTH (1 << 4)

#define MAX_CLIENT (1 << 10)
#define CLOSE_TIME (1 << 12)
#define CHECK_INTERVAL (1 << 12)

typedef struct {
  /* file descriptor */
  int fd;
  /* HTTP or HTTPS */
  int type;
  /* duration */
  time_t time;
  /* TLS: ssl */
  SSL* ssl;
  /* client addr */
  char addr[ADDR_LENGTH];
  /* httpio */
  httpio_t httpio;
  /* cgi_pipe */
  int pipe_fd;
  /* persistent? */
  int conn;
} client_t;


typedef struct {
  int n;
  int last_check_time;
  client_t* info;
} pool_t;


void pool_init(pool_t* pool);

void pool_deinit(pool_t* pool);

int pool_insert(pool_t* pool, int fd, int type, SSL* ssl, char* addr);
int Pool_insert(pool_t* pool, int fd, int type, SSL* ssl, char* addr);

void check(pool_t* pool);
void Check(pool_t* pool);

void pool_remove(pool_t* pool, int index);

void pool_update(pool_t* pool, int index);


#endif