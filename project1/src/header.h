#ifndef _HEADER_H
#define _HEADER_H

#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HEADER_TYPES (1 << 6)
#define MAX_HEADER_NAME (1 << 5)

/* transfer encoding */
#define TE_IDENTITY 0
#define TE_CHUNKED 1

/* request header info */
typedef struct {
  int connection;
  int content_length;
  int transfer_encoding;
} request_hdr_t;

/* request header handler struct */
typedef struct {
  char hdr[MAXLINE]; // header name
  int (*f)(char* hdr_val, request_hdr_t* request_hdr); // handler, return success(0) or failure(-1)
} hdr_handler_t;

/* use hdr_handler to parse different request header */
extern hdr_handler_t hdr_handler[HEADER_TYPES];

/* persistent connection */
int hdr_handler_connection(char* hdr_val, request_hdr_t* request_hdr);

/* get content length */
int hdr_handler_content_length(char* hdr_val, request_hdr_t* request_hdr);

/* set transfer encoding */
int hdr_handler_transfer_encoding(char* hdr_val, request_hdr_t* request_hdr);

/* parse a header line */
int parse_hdr(char* hdr, request_hdr_t* request_hdr);
int Parse_hdr(char* hdr, request_hdr_t* request_hdr);

/* get header name from a header line */
char* get_header_name(char* hdr, char* header_name);
char* Get_header_name(char* hdr, char* header_name);

#endif