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


typedef struct {
  char** argv;
  char** env;
} param_t;

int start_http(int type);
int Start_http(int type);

int serve_http(int ep_fd, pool_t* pool, int index);

int serve_static(httpio_t* hio, request_t* req);
int Serve_static(httpio_t* hio, request_t* req);

int serve_cgi(httpio_t* hio, request_t* req, int* pfd);
int Serve_cgi(httpio_t* hio, request_t* req, int* pfd);

void get_cgi_env(request_t* req, param_t* p);

void insert_env(char** env, int i, char* buf);

void http_error(httpio_t* hio, int errnum);

#endif