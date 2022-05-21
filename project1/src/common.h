#ifndef _COMMON_H
#define _COMMON_H


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>

#define MAXLINE (1 << 11)

/* logfile可读写 */
#define MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#define MASK (S_IXUSR|S_IXGRP|S_IXOTH)

/* epoll wait list */
#define READY_LIST_SIZE (1 << 4)

/* time out seconds */
#define TIMEOUT_S 0
#define TIMEOUT_MS 10

#define ERR_SYS -1
#define ERR_LEN -2

#define CONN_CLOSE 0
#define CONN_KEEPALIVE 1

/* http error */
#define ERR_TYPES (1 << 4)
#define BAD_REQUEST 400
#define FORBIDDEN 403
#define NOT_FOUND 404
#define REQUEST_ENTITY_TOO_LARGE 413
#define NOT_IMPLEMENTED 501
#define HTTP_VERSION_NOT_SUPPORTED 505

#define HTTP 0
#define HTTPS 1

#define ENV_TYPES 18

typedef void handler_t(int);

typedef struct {
  int errnum;
  char* errmsg;
} http_err_t;

extern http_err_t http_err[ERR_TYPES];

extern char* http_port;
extern char* https_port;
extern char* log_file;
extern char* lock_file;
extern char* www_folder;
extern char* cgi_path;
extern char* priv_key_file;
extern char* certificate_file;
extern int log_fd;

void write_log(char* msg);
void write_nlog(char* msg, int n);
void err_exit(char* msg);
void err_return(char* msg);

void* Malloc(size_t size);
void* Calloc(size_t nmemb, size_t size);
int Open(char *pathname, int flags, mode_t mode);
int Write(int fd, char* buf, int len);
void Close(int fd);
int Stat(char* file, struct stat* buf);
void* Mmap(void* addr, size_t len, int prot, int flags, int fd, __off_t offset);
int Munmap (void* addr, size_t len);
int Fork();
void Execve(char* path, char** argv, char** env);
void Dup2(int fd, int fd2);
int Pipe(int pipedes[2]);

int Epoll_create1(int flags);
int Epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);
int Epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);

int Socket(int domain, int type, int protocol);
int Accept(int listen_fd, struct sockaddr* addr, socklen_t* addr_len);
int Bind(int fd, const struct sockaddr* addr, socklen_t len);
int Listen(int fd, int n);
int Recv(int fd, void* buf, size_t n, int flags);

handler_t *Signal(int signum, handler_t *handler);


#endif