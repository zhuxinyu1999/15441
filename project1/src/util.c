#include "util.h"

void parse_cmd(int argc, char** argv) {
  if (argc != 9) {
    printf("usage: %s <HTTP port> <HTTPS port> <log file> <lock file> <www folder> <CGI script path> <private key file> <certificate file>\n", argv[0]);
    exit(0);
  }
  http_port = argv[1];
  https_port = argv[2];
  log_file = argv[3];
  lock_file = argv[4];
  www_folder = argv[5];
  cgi_path = argv[6];
  priv_key_file = argv[7];
  cert_file = argv[8];
}

int open_logfile() {
  umask(MASK);
  int fd = open(log_file, O_CREAT|O_TRUNC|O_WRONLY, MODE);
  if (fd < 0) {
    printf("failed to open log file\n\n");
    exit(0);
  }
  return fd;
}

void http_init(pool_t* pool, int ep_fd) {
  
  struct epoll_event ev;

  /* https */
  int http_fd = Start_http(EV_LISTEN_HTTP);
  if (http_fd < 0) {
    err_exit("http init failed\n");
  }
  if ((ev.data.u32 = Pool_insert(pool, http_fd, EV_LISTEN_HTTP, NULL, "")) < 0) {
    err_exit("http init failed\n");
  }
  if (Epoll_ctl(ep_fd, EPOLL_CTL_ADD, http_fd, &ev) < 0) {
    err_exit("http init failed\n");
  }
  write_log("http init\n\n");

  /* https */
  int https_fd = Start_http(EV_LISTEN_HTTPS);
  if (https_fd < 0) {
    err_exit("https init failed\n");
  }
  if ((ev.data.u32 = Pool_insert(pool, https_fd, EV_LISTEN_HTTPS, NULL, "")) < 0) {
    err_exit("https init failed\n");
  }
  if (Epoll_ctl(ep_fd, EPOLL_CTL_ADD, https_fd, &ev) < 0) {
    err_exit("https init failed\n");
  }
  write_log("https init\n\n");

}

void add_client(SSL_CTX* ctx, pool_t* pool, int server_fd, int ep_fd, int ev_type) {
  int fd;
  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(struct sockaddr);

  if ((fd = Accept(server_fd, (struct sockaddr*)(&addr), &addr_len)) < 0) {
    err_return("add client failed\n");
  }
  char* addr_s = inet_ntoa(addr.sin_addr);
  
  SSL* ssl = NULL;
  if (ev_type == EV_HTTPS && (ssl = Get_ssl(ctx, fd)) == NULL) {
    Close(fd);
    err_return("add client failed\n");
  }
  
  struct epoll_event ev;
  ev.events = EPOLLIN|EPOLLRDHUP|EPOLLET;
  if ((ev.data.u32 = Pool_insert(pool, fd, ev_type, ssl, addr_s)) < 0) {
    free_ssl(ssl);
    Close(fd);
    err_return("add client failed\n");
  }

  if (Epoll_ctl(ep_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
    free_ssl(ssl);
    Close(fd);
    err_return("add client failed\n");
  }

  char buf[MAXLINE];
  sprintf(buf, "add client %s\n\n", addr_s);
  write_log(buf);
}

void response_http_client(int ep_fd, pool_t* pool, struct epoll_event* ev, int index) {

  int fd = pool->info[index].fd;
  int type = pool->info[index].type;
  SSL* ssl = pool->info[index].ssl;
  char* addr = pool->info[index].addr;

  char buf[MAXLINE];
  if (ev->events & EPOLLRDHUP) {
    if (fd != 0) {
      sprintf(buf, "client %s close the connection\n", addr);
      write_log(buf);
      free_ssl(ssl);
      pool_remove(pool, index);
      Close(fd);
    }
    /* fd has been closed, but epoll wait invoked by child process */
    return;
  }

  sprintf(buf, "serve client %s: begin\n", addr);
  write_log(buf);
  
  int state = serve_http(ep_fd, pool, index);

  if (state == WAIT_PIPE) {
    sprintf(buf, "serve client %s: wait cgi\n\n", addr);
    write_log(buf);
    return;
  }

  if (state == CONN_KEEPALIVE) {
    sprintf(buf, "serve client %s: end(keep-alive)\n\n", addr);
    write_log(buf);
  } else {
    if (state == CONN_CLOSE) {
      sprintf(buf, "serve client %s: end(close)\n\n", addr);
    } else if (state == ERR_SYS) {
      sprintf(buf, "serve client %s: end(sys error)\n\n", addr);
    } else {
      sprintf(buf, "serve client %s: end(http error)\n\n", addr);
    }
    write_log(buf);
    free_ssl(ssl);
    pool_remove(pool, index);
    Close(fd);
  }
}

void pipe_response(pool_t* pool, int index) {
  int buflen;
  char buf[MAXLINE];
  int fd = pool->info[index].fd;
  int pfd = pool->info[index].pipe_fd;
  char* addr = pool->info[index].addr;
  httpio_t* hio = &(pool->info[index].httpio);

  sprintf(buf, "cgi pipe response to client %s: begin\n", addr);
  write_log(buf);

  while ((buflen = read(pfd, buf, MAXLINE)) > 0) {
    httpio_writen(hio, buflen, buf);
  }
  Httpio_send(hio);

  Close(pfd);
  if (pool->info[index].conn == CONN_CLOSE) {
    free_ssl(pool->info[index].ssl);
    pool_remove(pool, index);
    Close(fd);
  }

  sprintf(buf, "cgi pipe response to client %s: end(%s)\n", addr, 
         pool->info[index].conn == CONN_KEEPALIVE ? "keep-alive" : "close");
  write_log(buf);

}