#ifndef _HTTPIO_H
#define _HTTPIO_H

#include <sys/socket.h>
#include <memory.h>
#include "common.h"

#define MAX_HTTPIO_BUF (1 << 10)

typedef struct {
  int fd;
  int type;
  int recv_i;
  int recv_n;
  int send_i;
  char recvbuf[MAX_HTTPIO_BUF];
  char sendbuf[MAX_HTTPIO_BUF];
} httpio_t;

void httpio_init(httpio_t* hio, int fd, int type);

int httpio_recv_http(httpio_t* hio);
int httpio_recv_https(httpio_t* hio);

int httpio_recv(httpio_t* hio);
int Httpio_recv(httpio_t* hio);

int httpio_readline(httpio_t* hio, char* usrbuf, int maxlen);
int Httpio_readline(httpio_t* hio, char* usrbuf, int maxlen);

int httpio_readn(httpio_t* hio, int n, char* usrbuf, int maxlen);
int Httpio_readn(httpio_t* hio, int n, char* usrbuf, int maxlen);

int httpio_empty(httpio_t* hio);

int httpio_send_http(httpio_t* hio);
int httpio_send_https(httpio_t* hio);

int httpio_send(httpio_t* hio);
int Httpio_send(httpio_t* hio);

int httpio_writen(httpio_t* hio, int n, char* usrbuf);
int Httpio_writen(httpio_t* hio, int n, char* usrbuf);

#endif