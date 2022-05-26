#ifndef _SIG_H
#define _SIG_H

#include <signal.h>
#include <sys/errno.h>

#include "common.h"

/* signal handler */
typedef void handler_t(int);

/* signal*/
handler_t *Signal(int signum, handler_t *handler);

void sig_alarm_handler(int sig);

#endif