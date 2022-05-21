#ifndef _HEADER_H
#define _HEADER_H

#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HEADER_TYPES (1 << 6)
#define MAX_HEADER_NAME (1 << 5)
#define TE_IDENTITY 0
#define TE_CHUNKED 1

typedef struct {
  int connection;
  int content_length;
  int transfer_encoding;
} request_hdr_t;

typedef struct {
  char hdr[MAXLINE];
  int (*f)(char* hdr_val, request_hdr_t* request_hdr);
} hdr_handler_t;

extern hdr_handler_t hdr_handler[HEADER_TYPES];

int hdr_handler_connection(char* hdr_val, request_hdr_t* request_hdr);

int hdr_handler_content_length(char* hdr_val, request_hdr_t* request_hdr);

int hdr_handler_transfer_encoding(char* hdr_val, request_hdr_t* request_hdr);

int parse_hdr(char* hdr, request_hdr_t* request_hdr);
int Parse_hdr(char* hdr, request_hdr_t* request_hdr);

char* get_header_name(char* hdr, char* header_name);
char* Get_header_name(char* hdr, char* header_name);

#endif