#ifndef _POOL_H
#define _POOL_H

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <openssl/ssl.h>
#include "common.h"
#include "httpio.h"

/* pool capacity */
#define MAX_CLIENT (1 << 10)

/* time to close */
#define CLOSE_TIME (1 << 12)

/* check time */
#define INTERVAL_TIME (1 << 14)

/* client addr length */
#define ADDR_LENGTH (1 << 4)

typedef struct {
  int fd;                 // file descriptor
  int type;               // HTTP or HTTPS
  time_t time;            // last_request_time
  SSL* ssl;               // TLS: ssl
  char addr[ADDR_LENGTH]; // client addr
  httpio_t httpio;        // httpio
  int pipe_fd;            // cgi_pipe fd
  int conn;               // persistent
} client_t;

/* fd pool */
typedef struct {
  int n;
  int last_check_time;
  client_t* info;
} pool_t;

/* init */
void pool_init(pool_t* pool);

/* deinit */
void pool_deinit(pool_t* pool);

/* insert a fd with ssl and addr, return index */
int pool_insert(pool_t* pool, int fd, int type, SSL* ssl, char* addr);
int Pool_insert(pool_t* pool, int fd, int type, SSL* ssl, char* addr);

/* check and close long time no_request fd */
void check(pool_t* pool);

/* remove fd by index */
void pool_remove(pool_t* pool, int index);
void pool_update(pool_t* pool, int index);


#endif