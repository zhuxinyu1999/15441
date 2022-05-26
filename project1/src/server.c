#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <openssl/ssl.h>

#include "common.h"
#include "util.h"
#include "pool.h"
#include "tls.h"

int main(int argc, char** argv) {

  parse_cmd(argc, argv);

  log_fd = open_logfile();

  int ep_fd = Epoll_create1(0);

  pool_t pool;
  pool_init(&pool);

  SSL_CTX* ctx;
  ctx = Ssl_init();

  http_init(&pool, ep_fd);
  
  int client_fd, index, ready, i, ev_type;
  struct epoll_event ready_list[READY_LIST_SIZE];

  while (1) {
    ready = Epoll_wait(ep_fd, ready_list, READY_LIST_SIZE, -1);
    for (i = 0; i < ready; ++i) {
      index = ready_list[i].data.u32;
      /* http/https listen or client request */
      if (index >= 0) {
        ev_type = pool.info[index].type;
        if (ev_type == EV_LISTEN_HTTP) {
          add_client(ctx, &pool, pool.info[index].fd, ep_fd, HTTP);
        } else if (ev_type == EV_LISTEN_HTTPS) {
          add_client(ctx, &pool, pool.info[index].fd, ep_fd, HTTPS);
        } else {
          response_http_client(ep_fd, &pool, &(ready_list[i]), index);
        }
      } else {
        /* cgi pipe response */
        index = -index;
        pipe_response(&pool, index);
      }
    }
    if (check_tag == 1) {
      check(&pool);
      check_tag = 0;
    }
  }
  
  pool_deinit(&pool);
  SSL_CTX_free(ctx);
}