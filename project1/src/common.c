#include "common.h"

http_err_t http_err[ERR_TYPES] = {
  {BAD_REQUEST, "Bad Request"},
  {NOT_FOUND, "Not Found"},
  {NOT_IMPLEMENTED, "Not Implemented"},
  {HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"},
  {REQUEST_ENTITY_TOO_LARGE, "Request Entity Too Large"},
  {FORBIDDEN, " Forbidden"}
};

char* http_port;
char* https_port;
char* log_file;
char* lock_file;
char* www_folder;
char* cgi_path;
char* priv_key_file;
char* cert_file;

int log_fd;

void write_log(char* msg) {
  write(log_fd, msg, strlen(msg));
}

void write_nlog(char* msg, int len) {
  write(log_fd, msg, len);
}

void err_exit(char* msg) {
  char buf[MAXLINE];
  sprintf(buf, "[error] %s", msg);
  write_log(buf);
  exit(0);
}

void err_return(char* msg) {
  char buf[MAXLINE];
  sprintf(buf, "[error] %s", msg);
  write_log(buf);
}

void* Malloc(size_t size) {
  void* p = malloc(size);
  if (p == NULL) {
    char buf[MAXLINE];
    sprintf(buf, "malloc: %s\n", strerror(errno));
    err_return(buf);
  }
  return p;
}

void* Calloc(size_t nmemb, size_t size) {
  void* p = calloc(nmemb, size);
  if (p == NULL) {
    char buf[MAXLINE];
    sprintf(buf, "calloc: %s\n", strerror(errno));
    err_return(buf);
  }
  return p;
}

int Epoll_create1(int flags) {
  int fd = epoll_create1(flags);
  if (fd < 0) {
    char buf[MAXLINE];
    sprintf(buf, "epoll_create1: %s\n", strerror(errno));
    err_exit(buf);
  }
  return fd;
}

int Epoll_ctl(int epfd, int op, int fd, struct epoll_event* event) {
  int state = epoll_ctl(epfd, op, fd, event);
  if (state < 0) {
    char buf[MAXLINE];
    sprintf(buf, "epoll_ctl: %s\n", strerror(errno));
    err_return(buf);
  }
  return state;
}

int Epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout) {
  int ready = epoll_wait(epfd, events, maxevents, timeout);
  if (ready < 0) {
    char buf[MAXLINE];
    sprintf(buf, "epoll_wait: %s\n", strerror(errno));
    err_return(buf);
  }
  return ready;
}

int Open(char *pathname, int flags, mode_t mode) {
  int fd = open(pathname, flags, mode);
  if (fd < 0) {
    char buf[MAXLINE];
    sprintf(buf, "open: %s\n", strerror(errno));
    err_return(buf);
  }
  return fd;
}

int Write(int fd, char* buf, int len) {
  int wn = write(fd, buf, len);
  if (wn < 0) {
    char buf[MAXLINE];
    sprintf(buf, "write: %s\n", strerror(errno));
    err_return(buf);
  }
  return wn;
}

void Close(int fd) {
  if (close(fd) < 0) {
    char buf[MAXLINE];
    sprintf(buf, "close: %s\n", strerror(errno));
    err_return(buf);
  }
}

int Stat(char* file, struct stat* buf) {
  int state = stat(file, buf);
  if (state < 0) {
    char buf[MAXLINE];
    sprintf(buf, "stat: %s\n", strerror(errno));
    err_return(buf);
  }
  return state;
}

void* Mmap(void* addr, size_t len, int prot, int flags, int fd, __off_t offset) {
  void* p = mmap(addr, len, prot, flags, fd, offset);
  if (!p) {
    char buf[MAXLINE];
    sprintf(buf, "mmap: %s\n", strerror(errno));
    err_return(buf);
  }
  return p;
}

int Munmap(void* addr, size_t len) {
  int state = munmap(addr, len);
  if (state < 0) {
    char buf[MAXLINE];
    sprintf(buf, "munmap: %s\n", strerror(errno));
    err_return(buf);
  }
  return state;
}

int Accept(int listen_fd, struct sockaddr* addr, socklen_t* addr_len) {
  int fd = accept(listen_fd, addr, addr_len);
  if (fd < 0) {
    char buf[MAXLINE];
    sprintf(buf, "accept: %s\n", strerror(errno));
    err_return(buf);
    return fd;
  }
  struct timeval time_out = {TIMEOUT_S, TIMEOUT_MS};
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &time_out, sizeof(time_out));
  return fd;
}

int Socket(int domain, int type, int protocol) {
  int fd = socket(domain, type, protocol);
  if (fd < 0) {
    char buf[MAXLINE];
    sprintf(buf, "socket: %s\n", strerror(errno));
    err_return(buf);
    return fd;
  }
  return fd;
}

int Bind(int fd, const struct sockaddr* addr, socklen_t len) {
  int state = bind(fd, addr, len);
  if (state < 0) {
    char buf[MAXLINE];
    sprintf(buf, "bind: %s\n", strerror(errno));
    err_return(buf);
  }
  return state;
}

int Listen(int fd, int n) {
  int state = listen(fd, n);
  if (state < 0) {
    char buf[MAXLINE];
    sprintf(buf, "listen: %s\n", strerror(errno));
    err_return(buf);
  }
  return state;
}

int Recv(int fd, void* buf, size_t n, int flags) {
  int rn = recv(fd, buf, n, flags);
  if (rn < 0) {
    char buf[MAXLINE];
    sprintf(buf, "recv: %s\n", strerror(errno));
    err_return(buf);
  }
  return rn;
}


handler_t *Signal(int signum, handler_t *handler) {
  struct sigaction action, old_action;

  action.sa_handler = handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_RESTART;

  if (sigaction(signum, &action, &old_action) < 0) {
    char buf[MAXLINE];
    sprintf(buf, "sigaction: %s\n", strerror(errno));
    err_return(buf);
    return -1;
  }
        
  return (old_action.sa_handler);
}

int Pipe(int pipedes[2]) {
  int state = pipe(pipedes);
  if (state < 0) {
    char buf[MAXLINE];
    sprintf(buf, "pipe: %s\n", strerror(errno));
    err_return(buf);
  }
  return state;
}

int Fork() {
  pid_t pid = fork();
  if (pid < 0) {
    char buf[MAXLINE];
    sprintf(buf, "fork: %s\n", strerror(errno));
    err_return(buf);
  }
  return pid;
}

void Execve(char* path, char** argv, char** env) {
  if (execve(path, argv, env) < 0) {
    char buf[MAXLINE];
    sprintf(buf, "execve: %s\n", strerror(errno));
    err_return(buf);
  }
}

void Dup2(int fd, int fd2) {
  if (dup2(fd, fd2) < 0) {
    char buf[MAXLINE];
    sprintf(buf, "dup2: %s\n", strerror(errno));
    err_return(buf);
  }
}