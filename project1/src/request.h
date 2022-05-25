#ifndef _REQUEST_H
#define _REQUEST_H

#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "httpio.h"
#include "header.h"

/* requset body */
#define MAX_BODY (1 << 14)

/* serve static src or cgi scripts */
#define STATIC 0
#define CGI 1

typedef struct {
  int type; // HTTP / HTTPS

  char method[MAXLINE];
  char uri[MAXLINE];
  char version[MAXLINE];
  request_hdr_t header; //request header
  char* body; // request body

  char filename[MAXLINE]; // filename in uri
  char filetype[MAXLINE]; // filetype in uri
  char qs[MAXLINE]; // query string in uri
  int is_cgi; // static resource or cgi scripts
} request_t;

/* init a request */
request_t* request_init(int type);
request_t* Request_init(int type);

/* free a request */
void request_deinit(request_t* req);

/* parse a request */
int parse(httpio_t* hio, request_t* req, int* pipeline);
int Parse(httpio_t* hio, request_t* req, int* pipeline);

/* parse request line: method uri version */
int parse_request_line(httpio_t* hio, request_t* req);
int Parse_request_line(httpio_t* hio, request_t* req);

/* parse uri */
int parse_uri(request_t* req);
int Parse_uri(request_t* req);

/* get filetype from file name */
void get_filetype(request_t* req);

/* parse header */
int parse_request_header(httpio_t* hio, request_t* req);
int Parse_request_header(httpio_t* hio, request_t* req);

/* copy request body */
int get_request_body(httpio_t* hio, request_t* req);
int Get_request_body(httpio_t* hio, request_t* req);

/* copy directly */
int cpy_body(httpio_t* hio, request_t* req);
int Cpy_body(httpio_t* hio, request_t* req);

/* copy chunked body */
int chunked_cpy_body(httpio_t* hio, request_t* req);
int Chunked_cpy_body(httpio_t* hio, request_t* req);



#endif