#include "httpio.h"

void httpio_init(httpio_t* hio, int fd, int type, SSL* ssl) {
  hio->fd = fd;
  hio->type = type;
  hio->ssl = ssl;
  hio->recv_i = 0;
  hio->recv_n = 0;
  hio->send_i = 0;
}

int httpio_recv_http(httpio_t* hio) {
  hio->recv_n = recv(hio->fd, hio->recvbuf, MAX_HTTPIO_BUF, 0);
  hio->recv_i = 0;
  return hio->recv_n;
}

int httpio_recv_https(httpio_t* hio) {
  hio->recv_n = SSL_read(hio->ssl, hio->recvbuf, MAX_HTTPIO_BUF);
  hio->recv_i = 0;
  return 0;
}

int httpio_recv(httpio_t* hio) {
  if (hio->type == HTTP) {
    return httpio_recv_http(hio);
  } else {
    return httpio_recv_https(hio);
  }
}

int Httpio_recv(httpio_t* hio) {
  int n = httpio_recv(hio);
  if (n < 0) {
    err_return("httpio_recv failed\n");
  }
  return n;
}

int httpio_send_http(httpio_t* hio) {
  int sn = send(hio->fd, hio->sendbuf, hio->send_i, 0);
  hio->send_i = 0;
  return sn;
}

int httpio_send_https(httpio_t* hio) {
  int sn = SSL_write(hio->ssl, hio->sendbuf, hio->send_i);
  hio->send_i = 0;
  return sn;
}

int httpio_send(httpio_t* hio) {
  if (hio->type == HTTP) {
    return httpio_send_http(hio);
  } else {
    return httpio_send_https(hio);
  }
}

int Httpio_send(httpio_t* hio) {
  int sn = httpio_send(hio);
  if (sn < 0) {
    err_return("httpio_send failed\n");
  }
  return sn;
}

/* 
 * read a line(end with a '\n') from recvbuf
 * if recvbuf empty, Httpio_recv from client to fill
 */
int httpio_readline(httpio_t* hio, char* usrbuf, int maxlen) {
  int j = 0;
  while (j != maxlen - 1) {
    if (hio->recv_i >= hio->recv_n) {
      if (Httpio_recv(hio) < 0) {
        return ERR_SYS;
      }
      if (hio->recv_n == 0) {
        usrbuf[j++] = '\n';
        usrbuf[j] = 0;
        return j;
      }
    }
    if ((usrbuf[j++] = hio->recvbuf[(hio->recv_i)++]) == '\n') {
      usrbuf[j] = 0;
      return j;
    }
  }
  return ERR_LEN;
}

int Httpio_readline(httpio_t* hio, char* usrbuf, int maxlen) {
  int rn = httpio_readline(hio, usrbuf, maxlen);
  if (rn == ERR_SYS) {
    err_return("httpio_readline failed(err_io)\n");
  } else if (rn == ERR_LEN) {
    err_return("httpio_readline failed(err_len)\n");
  }
  return rn;
}

int httpio_readn(httpio_t* hio, int n, char* usrbuf, int maxlen) {
  if (n + 1 > maxlen) {
    return ERR_SYS;
  }
  int j = 0;
  while (j != n) {
    if (hio->recv_i >= hio->recv_n) {
      if (Httpio_recv(hio) <= 0) {
        return ERR_SYS;
      }
    }
    usrbuf[j++] = hio->recvbuf[(hio->recv_i)++];
  }
  usrbuf[j] = 0;
  return j;
}

int Httpio_readn(httpio_t* hio, int n, char* usrbuf, int maxlen) {
  int rn = httpio_readn(hio, n, usrbuf, maxlen);
  if (rn == ERR_SYS) {
    err_return("httpio_readn failed\n");
  }
  return rn;
}

int httpio_empty(httpio_t* hio) {
  if (hio->recv_i < hio->recv_n) {
    return 0;
  }
  if (httpio_recv(hio) < 0) {
    return 1;
  }
  return hio->recv_n <= 0;
}

/*
 * write n bytes from usrbuf to sendbuf
 * when sendbuf full, Httpio_send to client to flush
 */
int httpio_writen(httpio_t* hio, int n, char* usrbuf) {
  int i;
  for (i = 0; i < n; ++i) {
    if (hio->send_i == MAX_HTTPIO_BUF) {
      if (Httpio_send(hio) < 0) {
        return ERR_SYS;
      }
    }
    hio->sendbuf[(hio->send_i)++] = usrbuf[i];
  }
  return 0;
}

int Httpio_writen(httpio_t* hio, int n, char* usrbuf) {
  int wn = httpio_writen(hio, n, usrbuf);
  if (wn < 0) {
    err_return("httpio_writen failed\n");
  }
  return wn;
}