#ifndef _TLS_H
#define _TLS_H

#include <openssl/ssl.h>

#include "common.h"

/* ssl init, return context */
SSL_CTX* ssl_init();
SSL_CTX* Ssl_init();

/* get a ssl bind to fd */
SSL* get_ssl(SSL_CTX* ctx, int fd);
SSL* Get_ssl(SSL_CTX* ctx, int fd);

/* free a ssl */
void free_ssl(SSL* ssl);

#endif  