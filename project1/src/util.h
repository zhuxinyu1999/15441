#ifndef _UTIL_H
#define _UTIL_H

#include "common.h"
#include "pool.h"
#include "http.h"
#include "tls.h"

void parse_cmd(int argc, char** argv);

int open_logfile();

void http_init(pool_t* pool, int ep_fd);

/* accept new client */
void add_client(SSL_CTX* ctx, pool_t* pool, int server_fd, int ep_fd, int ev_type);

/* recv request and send response */
void response_http_client(int ep_fd, pool_t* pool, struct epoll_event* ev, int index);

/* response client from cgi pipe */
void pipe_response(pool_t* pool, int index);

#endif  