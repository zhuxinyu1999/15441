#ifndef _REQUEST_H
#define _REQUEST_H

#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "httpio.h"
#include "header.h"

#define MAX_BODY (1 << 14)

#define STATIC 0
#define CGI 1

typedef struct {
  int type;

  char method[MAXLINE];
  char uri[MAXLINE];
  char version[MAXLINE];
  request_hdr_t header;
  char* body;

  char filename[MAXLINE];
  char filetype[MAXLINE];
  char qs[MAXLINE]; // query string
  int is_cgi;
} request_t;


request_t* request_init(int type);
request_t* Request_init(int type);

void request_deinit(request_t* req);

int parse(httpio_t* hio, request_t* req, int* pipeline, int* conn);
int Parse(httpio_t* hio, request_t* req, int* pipeline, int* conn);

int parse_request_line(httpio_t* hio, request_t* req);
int Parse_request_line(httpio_t* hio, request_t* req);
int parse_uri(request_t* req);
int Parse_uri(request_t* req);

int parse_request_header(httpio_t* hio, request_t* req);
int Parse_request_header(httpio_t* hio, request_t* req);

int get_request_body(httpio_t* hio, request_t* req);
int Get_request_body(httpio_t* hio, request_t* req);
int cpy_body(httpio_t* hio, request_t* req);
int Cpy_body(httpio_t* hio, request_t* req);
int chunked_cpy_body(httpio_t* hio, request_t* req);
int Chunked_cpy_body(httpio_t* hio, request_t* req);

void get_filetype(request_t* req);

#endif