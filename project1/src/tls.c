#include "tls.h"

SSL_CTX* ssl_init() {

  SSL_CTX* ctx;
  SSL_library_init();

  if ((ctx = SSL_CTX_new(TLSv1_2_server_method())) == NULL) {
    err_return("new ctx failed\n");
    return NULL;
  }
  
  if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0) {
    err_return("load cert_file failed\n");
    SSL_CTX_free(ctx);
    return NULL;
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, priv_key_file, SSL_FILETYPE_PEM) <= 0) {
    err_return("load priv_key_file failed\n");
    SSL_CTX_free(ctx);
    return NULL;
  }

  if (!SSL_CTX_check_private_key(ctx)) {
    err_return("check priv key failed\n");
    SSL_CTX_free(ctx);
    return NULL;
  }

  return ctx;
}

SSL_CTX* Ssl_init() {
  SSL_CTX* ctx = ssl_init();
  if (ctx == NULL) {
    err_exit("ssl_init failed\n");
  }
  return ctx;
}

SSL* get_ssl(SSL_CTX* ctx, int fd) {
  SSL* ssl;
  if ((ssl = SSL_new(ctx)) == NULL) {
    err_return("new ssl failed\n");
    return NULL;
  }
  if (SSL_set_fd(ssl, fd) <= 0) {
    err_return("ssl set fd failed\n");
    return NULL;
  }
  if (SSL_accept(ssl) <= 0) {
    err_return("ssl accept failed\n");
    return NULL;
  }
  return ssl;
}

SSL* Get_ssl(SSL_CTX* ctx, int fd) {
  SSL* ssl = get_ssl(ctx, fd);
  if (ssl == NULL) {
    err_return("get ssl failed\n");
  }
  return ssl;
}

void free_ssl(SSL* ssl) {
  if (ssl != NULL) {
    SSL_free(ssl);
  }
}
