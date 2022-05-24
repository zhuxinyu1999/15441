#ifndef _TLS_H
#define _TLS_H

#include <openssl/ssl.h>

#include "common.h"

SSL_CTX* ssl_init();
SSL_CTX* Ssl_init();

SSL* get_ssl(SSL_CTX* ctx, int fd);
SSL* Get_ssl(SSL_CTX* ctx, int fd);

void free_ssl(SSL* ssl);

#endif  