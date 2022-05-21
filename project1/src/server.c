#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "common.h"
#include "pool.h"
#include "http.h"

void parse_cmd(char** argv);
int open_logfile();
void add_client(pool_t* pool, int server_fd, int ep_fd, int ev_type);
void http_init(int ep_fd, pool_t* pool);
void response_http_client(struct epoll_event* ev, pool_t* pool, int index);

int main(int argc, char** argv) {

  Signal(SIGPIPE, SIG_IGN);

  if (argc != 9) {
    printf("usage: %s <HTTP port> <HTTPS port> <log file> <lock file> <www folder> <CGI script path> <private key file> <certificate file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* parse_cmd */
  parse_cmd(argv);

  /* log_fd */
  log_fd = open_logfile();

  int ep_fd = Epoll_create1(0);

  /* pool init */
  pool_t pool;
  pool_init(&pool);

  /* init */
  http_init(ep_fd, &pool);

  /* https_listen_fd */
  int client_fd, index, ready, i;
  struct epoll_event ready_list[READY_LIST_SIZE];
  /* while */
  while (1) {
    ready = Epoll_wait(ep_fd, ready_list, READY_LIST_SIZE, -1);
    for (i = 0; i < ready; ++i) {
      index = ready_list[i].data.u32;
      switch (pool.info[index].type) {
        case EV_LISTEN_HTTP: {
          add_client(&pool, pool.info[index].fd, ep_fd, EV_HTTP);
          break;
        } 
        case EV_LISTEN_HTTPS: {
          add_client(&pool, pool.info[index].fd, ep_fd, EV_HTTPS);
          break;
        }
        case EV_HTTP: {
          /* serve http */
          response_http_client(&(ready_list[i].events), &pool, index);
          break;
        }
        case EV_HTTPS: {
          break;
        }
        default: {
          err_return("undefined event type\n");
        }
      }
    }
  }

  pool_deinit(&pool);
}

void parse_cmd(char** argv) {
  http_port = argv[1];
  https_port = argv[2];
  log_file = argv[3];
  lock_file = argv[4];
  www_folder = argv[5];
  cgi_path = argv[6];
  priv_key_file = argv[7];
  certificate_file = argv[8];
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

void http_init(int ep_fd, pool_t* pool) {
  int fd = Start_http();

  if (fd < 0) {
    err_exit("http init failed\n");
  }
  
  struct epoll_event ev;
  if ((ev.data.u32 = Pool_insert(pool, fd, EV_LISTEN_HTTP, "")) < 0) {
    err_exit("http init failed\n");
  }

  if (Epoll_ctl(ep_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
    err_exit("http init failed\n");
  }

  write_log("http init\n\n");
}

void add_client(pool_t* pool, int server_fd, int ep_fd, int ev_type) {
  int fd;
  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(struct sockaddr);

  if ((fd = Accept(server_fd, (struct sockaddr*)(&addr), &addr_len)) < 0) {
    err_return("add client failed\n");
  }
  char* addr_s = inet_ntoa(addr.sin_addr);

  struct epoll_event ev;
  ev.events = EPOLLIN|EPOLLRDHUP|EPOLLET;
  if ((ev.data.u32 = Pool_insert(pool, fd, ev_type, addr_s)) < 0) {
    err_return("add client failed\n");
  }

  if (Epoll_ctl(ep_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
    err_return("add client failed\n");
  }

  char buf[MAXLINE];
  sprintf(buf, "add client %s\n\n", addr_s);
  write_log(buf);
}

void response_http_client(struct epoll_event* ev, pool_t* pool, int index) {

  int fd = pool->info[index].fd;
  char buf[MAXLINE];

  /* connection closed */
  if (ev->events & EPOLLRDHUP) {
    /* 
     * fd has been closed
     * but epoll wait invoked by child process
     */
    if (fd != 0) {
      sprintf(buf, "client %s close the connection\n", pool->info[index].addr);
      pool_remove(pool, index);
      Close(fd);
    }
    return;
  }

  sprintf(buf, "serve client %s: begin\n", pool->info[index].addr);
  write_log(buf);
  pool_update(pool, index);
  
  int state = serve_http(fd, HTTP);
  if (state == CONN_KEEPALIVE) {
    sprintf(buf, "serve client %s: end(keep-alive)\n\n", pool->info[index].addr);
    write_log(buf);
  } else {
    if (state == CONN_CLOSE) {
      sprintf(buf, "serve client %s: end(close)\n\n", pool->info[index].addr);
    } else if (state == ERR_SYS) {
      sprintf(buf, "serve client %s: end(sys error)\n\n", pool->info[index].addr);
    } else {
      sprintf(buf, "serve client %s: end(http error)\n\n", pool->info[index].addr);
    }
    write_log(buf);
    pool_remove(pool, index);
    Close(fd);
  }
}