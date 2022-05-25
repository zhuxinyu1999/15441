#ifndef _HTTP_H
#define _HTTP_H

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include "common.h"
#include "httpio.h"
#include "request.h"
#include "pool.h"

#define BACKLOG (1 << 4)

/* cgi script parameter */
typedef struct {
  char** argv;
  char** env;
} param_t;

/* start listen http/https */
int start_http(int type);
int Start_http(int type);

/* serve a client */
int serve_http(int ep_fd, pool_t* pool, int index);

/* serve static resource */
int serve_static(httpio_t* hio, request_t* req);
int Serve_static(httpio_t* hio, request_t* req);

/* serve cgi */
int serve_cgi(httpio_t* hio, request_t* req, int* pfd);
int Serve_cgi(httpio_t* hio, request_t* req, int* pfd);

/* set env in cgi script parameter */
void get_cgi_env(request_t* req, param_t* p);
void insert_env(char** env, int i, char* buf);

/* send http error response */
void http_error(httpio_t* hio, int errnum);

#endif