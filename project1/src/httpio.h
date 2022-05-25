#ifndef _HTTPIO_H
#define _HTTPIO_H

#include <sys/socket.h>
#include <memory.h>
#include <openssl/ssl.h>
#include "common.h"

#define MAX_HTTPIO_BUF (1 << 10)

typedef struct {
  int fd;
  /* HTTP or HTTPS */
  int type;
  /* TLS read and write */
  SSL* ssl;
  /* recv buf */
  int recv_i;
  int recv_n;
  char recvbuf[MAX_HTTPIO_BUF];
  /* send buf */
  int send_i;
  char sendbuf[MAX_HTTPIO_BUF];
} httpio_t;

/* init a httpio */
void httpio_init(httpio_t* hio, int fd, int type, SSL* ssl);

/* http recv by fd */
int httpio_recv_http(httpio_t* hio);

/* https recv by ssl */
int httpio_recv_https(httpio_t* hio);

/* when recvbuf is empty, recv */
int httpio_recv(httpio_t* hio);
int Httpio_recv(httpio_t* hio);

/* http send by fd */
int httpio_send_http(httpio_t* hio);

/* https send by ssl */
int httpio_send_https(httpio_t* hio);

/* when sendbuf is full, send */
int httpio_send(httpio_t* hio);
int Httpio_send(httpio_t* hio);

/* Interface: read a line from httpio */
int httpio_readline(httpio_t* hio, char* usrbuf, int maxlen);
int Httpio_readline(httpio_t* hio, char* usrbuf, int maxlen);

/* Interface: read n bytes from httpio */
int httpio_readn(httpio_t* hio, int n, char* usrbuf, int maxlen);
int Httpio_readn(httpio_t* hio, int n, char* usrbuf, int maxlen);

/* Interface: recv buf is empty? */
int httpio_empty(httpio_t* hio);

/* Interface: write n bytes to httpio */
int httpio_writen(httpio_t* hio, int n, char* usrbuf);
int Httpio_writen(httpio_t* hio, int n, char* usrbuf);

#endif