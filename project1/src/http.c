#include "http.h"

int start_http(int type) {
  int fd;
  struct sockaddr_in addr;

  if ((fd = Socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    return -1;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons((atoi(type == EV_LISTEN_HTTP ? http_port : https_port)));
  addr.sin_addr.s_addr = INADDR_ANY;

  if (Bind(fd, (struct sockaddr *)(&addr), sizeof(addr)) < 0) {
    Close(fd);
    return -1;
  }

  if (Listen(fd, BACKLOG) < 0) {
    Close(fd);
    return -1;
  }

  return fd;
}

int Start_http(int type) {
  int fd = start_http(type);
  if (fd < 0) {
    if (type == HTTP) {
      err_return("start http failed\n");
    } else {
      err_return("start https failed\n");
    }
  }
  return fd;
}

int serve_http(int ep_fd, pool_t* pool, int index) {
  /* update last request time */
  pool_update(pool, index);

  client_t* client = &(pool->info[index]);
  httpio_t* hio = &(client->httpio);

  /* handle pipe line request */
  while (1) {
    int status, pipeline;
    request_t* req = request_init(hio->type);

    /* parse request */
    if (status = Parse(hio, req, &pipeline)) {
      if (status > 0) {
        http_error(hio, status);
      }
      request_deinit(req);
      return status;
    }

    client->conn = req->header.connection;
    client->pipe_fd = 0;

    /* STATIC or CGI */
    if (req->is_cgi == STATIC) {
      status = Serve_static(hio, req);
    } else {
      status = Serve_cgi(hio, req, &(client->pipe_fd));
    }

    request_deinit(req);
    
    if (status > 0) {
      http_error(hio, status);
    } else if (status < 0) {
      return ERR_SYS;
    }

    /* 
     * send response from cgi pipe_fd 
     * add a event to epoll_fd
     */
    if (client->pipe_fd > 0) {
      struct epoll_event ev;
      ev.data.u32 = -index;
      ev.events = EPOLLIN;
      if (Epoll_ctl(ep_fd, EPOLL_CTL_ADD, client->pipe_fd, &ev) < 0) {
        return -1;
      }
      /* return WAIT_PIPE, do not close client_fd */
      return WAIT_PIPE;
    }

    /* no more request */
    if (pipeline == 0) {
      break;
    }
  }

  /* keep-alive or close or ERR_SYS or ERR_HTTP */
  return client->conn;
}

int serve_static(httpio_t* hio, request_t* req) {
  /* check filename */
  struct stat sbuf;
  if (Stat(req->filename, &sbuf) < 0) {
    http_error(hio, NOT_FOUND);
    return NOT_FOUND;
  }
  if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
    http_error(hio, FORBIDDEN);
    return FORBIDDEN;
  }

  /* response line + header */
  char buf[MAXLINE];
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Httpio_writen(hio, strlen(buf), buf);
  sprintf(buf, "Connection: %s\r\n", req->header.connection ? "keep-alive" : "close");
  Httpio_writen(hio, strlen(buf), buf);

  int filesize = sbuf.st_size;
  sprintf(buf, "Content-length: %d\r\n", filesize);
  Httpio_writen(hio, strlen(buf), buf);
  sprintf(buf, "Content-type: %s\r\n\r\n", req->filetype);
  Httpio_writen(hio, strlen(buf), buf);
  
  /* HEAD response has no response body */
  if (!strcmp(req->method, "HEAD")) {
    sprintf(buf, "\r\n");
    Httpio_writen(hio, strlen(buf), buf);
    Httpio_send(hio);
    return 0;
  }

  /* respnse body */
  int file_fd;
  char* filep;
  if ((file_fd = Open(req->filename, O_RDONLY, 0)) < 0) {
    return -1;
  }
  filep = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, file_fd, 0);
  Close(file_fd);
  Httpio_writen(hio, filesize, filep);
  Munmap(filep, filesize);
  Httpio_send(hio);
  return 0;
}

int Serve_static(httpio_t* hio, request_t* req) {
  int status = serve_static(hio, req);
  if (status < 0) {
    err_return("serve_static failed\n");
  }
  return status;
}

int serve_cgi(httpio_t* hio, request_t* req, int* pfd) {
  struct stat sbuf;
  if (Stat(req->filename, &sbuf) < 0) {
    http_error(hio, NOT_FOUND);
    return NOT_FOUND;
  }
  if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
    http_error(hio, FORBIDDEN);
    return FORBIDDEN;
  }

  /* 2 pipe for parent and child */
  int pipefd_in[2];
  int pipefd_out[2];
  if (Pipe(pipefd_in) < 0) {
    return -1;
  }

  if (Pipe(pipefd_out) < 0) {
    return -1;
  }

  char buf[MAXLINE];
  sprintf(buf, "HTTP/1.1 200 OK\r\n");
  Httpio_writen(hio, strlen(buf), buf);

  pid_t pid;
  if ((pid = Fork()) < 0) {
    return -1;
  }

  if (pid == 0) {

    Close(pipefd_in[1]);
    Close(pipefd_out[0]);
    Dup2(pipefd_in[0], STDIN_FILENO);
    Dup2(pipefd_out[1], STDOUT_FILENO);

    param_t param;
    get_cgi_env(req, &param);
    Execve(param.argv[0], param.argv, param.env);
  } else {
    Close(pipefd_in[0]);
    Close(pipefd_out[1]);
    /* request body is child process STDIN */
    if (Write(pipefd_in[1], req->body, req->header.content_length) != req->header.content_length) {
      return -1;
    }
    Close(pipefd_in[1]);
  }

  /* read from pfd and send response to client */
  *pfd = pipefd_out[0];
  return 0;
}

int Serve_cgi(httpio_t* hio, request_t* req, int* pfd) {
  int status = serve_cgi(hio, req, pfd);
  if (status < 0) {
    err_return("serve_cgi failed\n");
  }
  return status;
}

void get_cgi_env(request_t* req, param_t* p) {
  p->argv = (char**)Calloc(2, sizeof(char*));
  p->argv[0] = (char*)Malloc(strlen(req->filename) + 1);
  strcpy(p->argv[0], req->filename);

  p->env = (char**)Calloc(ENV_TYPES + 1, sizeof(char*));
  char buf[MAXLINE];
  sprintf(buf, "QUERY_STRING=%s", req->qs);
  insert_env(p->env, 0, buf);
  sprintf(buf, "CONTENT_LENGTH=%d", req->header.content_length);
  insert_env(p->env, 1, buf);
  sprintf(buf, "PATH_INFO=%s", req->filename);
  insert_env(p->env, 2, buf);
  sprintf(buf, "REQUEST_METHOD=%s", req->method);
  insert_env(p->env, 3, buf);
  sprintf(buf, "SERVER_NAME=Zzxy");
  insert_env(p->env, 4, buf);
  sprintf(buf, "SERVER_PORT=%s", req->type == HTTP ? http_port : https_port);
  insert_env(p->env, 5, buf);
  sprintf(buf, "GATEWAY_INTERFACE=CGI/1.1");
  insert_env(p->env, 6, buf);
}

void insert_env(char** env, int i, char* buf) {
  env[i] = Malloc(strlen(buf) + 1);
  strcpy(env[i], buf);
}

void http_error(httpio_t* hio, int num) {
  char buf[MAXLINE];
  char* msg;
  int i;
  for (i = 0; i < ERR_TYPES; ++i) {
    if (num == http_err[i].errnum) {
      msg = http_err[i].errmsg;
      break;
    }
  }
  if (i == ERR_TYPES) {
    return;
  }
  sprintf(buf, "HTTP/1.1 %d %s\r\n", num, msg);
  Httpio_writen(hio, strlen(buf), buf);
  sprintf(buf, "Content-type: text/html\r\n\r\n");
  Httpio_writen(hio, strlen(buf), buf);
  sprintf(buf, "<html><h1>%d %s</h1></html>\r\n", num, msg);
  Httpio_writen(hio, strlen(buf), buf);
  Httpio_send(hio);
  return 0;
}