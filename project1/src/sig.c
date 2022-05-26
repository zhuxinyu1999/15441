#include "sig.h"

handler_t *Signal(int signum, handler_t *handler) {
  struct sigaction action, old_action;

  action.sa_handler = handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_RESTART;

  if (sigaction(signum, &action, &old_action) < 0) {
    char buf[MAXLINE];
    sprintf(buf, "sigaction: %s\n", strerror(errno));
    err_exit(buf);
    return -1;
  }
        
  return (old_action.sa_handler);
}

void sig_alarm_handler(int sig) {
  check_tag = 1;
}