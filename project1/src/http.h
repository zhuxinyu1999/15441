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

#define BACKLOG (1 << 4)

#define HTTP 0
#define HTTPS 1

int start_http();
int Start_http();

int serve_http(int fd, int type);

int serve_static(httpio_t* hio, request_t* req);
int Serve_static(httpio_t* hio, request_t* req);

int serve_cgi(httpio_t* hio, request_t* req);
int Serve_cgi(httpio_t* hio, request_t* req);

char** get_cgi_env(request_t* req);

void insert_env(char** env, int i, char* buf);

void http_error(httpio_t* hio, int errnum);

#endif