#include "http.h"

int start_http() {
  int fd;
  struct sockaddr_in addr;

  if ((fd = Socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    return -1;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons((atoi(http_port)));
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

int Start_http() {
  int fd = start_http();
  if (fd < 0) {
    err_return("start http failed\n");
  }
  return fd;
}

int serve_http(int fd, int type) {
  int conn;
  httpio_t httpio;
  httpio_init(&httpio, fd, type);
  while (1) {
    request_t* req = request_init(type);

    int state, pipeline;

    if (state = Parse(&httpio, req, &pipeline, &conn)) {
      if (state > 0) {
        http_error(&httpio, state);
      }
      request_deinit(req);
      return state;
    }

    if (req->is_cgi == STATIC) {
      state = Serve_static(&httpio, req);
    } else {
      state = Serve_cgi(&httpio, req);
    }

    if (state > 0) {
      http_error(&httpio, state);
    } else if (state < 0) {
      return ERR_SYS;
    }

    request_deinit(req);

    if (pipeline == 0) {
      break;
    }
  }

  return conn;
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
  int state = serve_static(hio, req);
  if (state < 0) {
    err_return("serve_static failed\n");
  }
  return state;
}

int serve_cgi(httpio_t* hio, request_t* req) {
  struct stat sbuf;
  if (Stat(req->filename, &sbuf) < 0) {
    http_error(hio, NOT_FOUND);
    return NOT_FOUND;
  }
  if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
    http_error(hio, FORBIDDEN);
    return FORBIDDEN;
  }

  pid_t pid;
  int pipefd[2];
  if (Pipe(pipefd) < 0) {
    return -1;
  }

  char buf[MAXLINE];
  sprintf(buf, "HTTP/1.1 200 OK\r\n");
  Httpio_writen(hio, strlen(buf), buf);

  if ((pid = Fork()) < 0) {
    return -1;
  }

  if (pid == 0) {
    Close(pipefd[1]);
    Dup2(pipefd[0], STDIN_FILENO);
    Dup2(hio->fd, STDOUT_FILENO);
    
    char* f = malloc(1024);
    strcpy(f, req->filename);
    char* argv[] = {f, NULL};
    char** env = get_cgi_env(req);
    get_cgi_env(req);

    //Execve(f, argv, env);
    Execve(req->filename, argv, env);
  } else {
    if (Write(pipefd[1], req->body, req->header.content_length) != req->header.content_length) {
      return -1;
    }
    Close(pipefd[0]);
    Close(pipefd[1]);
  }

  return 0;
}

int Serve_cgi(httpio_t* hio, request_t* req) {
  int state = serve_cgi(hio, req);
  if (state < 0) {
    err_return("serve_cgi failed\n");
  }
  return state;
}

char** get_cgi_env(request_t* req) {
  char** env = (char**) Calloc(ENV_TYPES + 1, sizeof(char*));
  char buf[MAXLINE];
  sprintf(buf, "QUERY_STRING=%s", req->qs);
  insert_env(env, 0, buf);
  sprintf(buf, "CONTENT_LENGTH=%d", req->header.content_length);
  insert_env(env, 1, buf);
  sprintf(buf, "PATH_INFO=%s", req->filename);
  insert_env(env, 2, buf);
  sprintf(buf, "REQUEST_METHOD=%s", req->method);
  insert_env(env, 3, buf);
  sprintf(buf, "SERVER_NAME=Zzxy");
  insert_env(env, 4, buf);
  sprintf(buf, "SERVER_PORT=%s", req->type == HTTP ? http_port : https_port);
  insert_env(env, 5, buf);
  sprintf(buf, "GATEWAY_INTERFACE=CGI/1.1");
  insert_env(env, 6, buf);
  return env;
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
  sprintf(buf, "<html><title>Request Error</title>");
  Httpio_writen(hio, strlen(buf), buf);
  sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
  Httpio_writen(hio, strlen(buf), buf);
  sprintf(buf, "%d: %s\r\n", num, msg);
  Httpio_writen(hio, strlen(buf), buf);
  sprintf(buf, "<hr><em>Zzxy Web server</em>\r\n");
  Httpio_writen(hio, strlen(buf), buf);
  Httpio_send(hio);
  return 0;
}