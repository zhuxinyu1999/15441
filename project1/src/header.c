#include "header.h"

hdr_handler_t hdr_handler[HEADER_TYPES] = {
  {"Accept:", NULL},
  {"Accept-Charset:", NULL},
  {"Accept-Encoding:", NULL},
  {"Accept-Language:", NULL},
  {"Accept-Ranges:", NULL},
  {"Age:", NULL},
  {"Allow:", NULL},
  {"Authorization:", NULL},
  {"Cache-Control:", NULL},
  {"Connection:", hdr_handler_connection},
  {"Content-Encoding:", NULL},
  {"Content-Language:", NULL},
  {"Content-Length:", hdr_handler_content_length},
  {"Content-Location:", NULL},
  {"Content-MD5:", NULL},
  {"Content-Range:", NULL},
  {"Content-Type:", NULL},
  {"Date:", NULL},
  {"ETag:", NULL},
  {"Expect:", NULL},
  {"Expires:", NULL},
  {"From:", NULL},
  {"Host:", NULL},
  {"If-Match:", NULL},
  {"If-Modified-Since:", NULL},
  {"If-None-Match:", NULL},
  {"If-Range:", NULL},
  {"If-Unmodified-Since:", NULL},
  {"Last-Modified:", NULL},
  {"Location:", NULL},
  {"Max-Forwards:", NULL},
  {"Pragma:", NULL},
  {"Proxy-Authenticate:", NULL},
  {"Proxy-Authorization:", NULL},
  {"Range:", NULL},
  {"Referer:", NULL},
  {"Retry-After:", NULL},
  {"Server:", NULL},
  {"TE:", NULL},
  {"Trailer:", NULL},
  {"Transfer-Encoding:", hdr_handler_transfer_encoding},
  {"Upgrade:", NULL},
  {"User-Agent:", NULL},
  {"Vary:", NULL},
  {"Via:", NULL},
  {"Warning:", NULL},
  {"WWW-Authenticate:", NULL},
  {"Upgrade-Insecure-Requests:", NULL},
  {"Origin:", NULL}
};

int hdr_handler_connection(char* hdr_val, request_hdr_t* request_hdr) {
  if (strstr(hdr_val, "close") != NULL) {
    request_hdr->connection = CONN_CLOSE;
  } else if (strstr(hdr_val, "keep-alive") != NULL) {
    request_hdr->connection = CONN_KEEPALIVE;
  } else {
    return -1;
  }
  return 0;
}

int hdr_handler_content_length(char* hdr_val, request_hdr_t* request_hdr) {
  request_hdr->content_length = atoi(hdr_val);
  return 0;
}

int hdr_handler_transfer_encoding(char* hdr_val, request_hdr_t* request_hdr) {
  if (strstr(hdr_val, "identity") != NULL) {
    request_hdr->transfer_encoding = TE_IDENTITY;
  } else if (strstr(hdr_val, "chunked") != NULL) {
    request_hdr->transfer_encoding = TE_CHUNKED;
  } else {
    return -1;
  }
  return 0;
}

int parse_hdr(char* hdr, request_hdr_t* request_hdr) {
  int i;
  char header_name[MAX_HEADER_NAME] = "";
  if ((hdr = Get_header_name(hdr, header_name)) == NULL) {
    return -1;
  }
  for (i = 0; i < HEADER_TYPES; ++i) {
    if (!strcasecmp(header_name, hdr_handler[i].hdr)) {
      if (hdr_handler[i].f != NULL) {
        if (hdr_handler[i].f(hdr, request_hdr) == -1) {
          return i;
        }
        return 0;
      }
      return 0;
    }
  }
  return -1;
}

int Parse_hdr(char* hdr, request_hdr_t* request_hdr) {
  int r = parse_hdr(hdr, request_hdr);
  if (r == -1) {
    err_return("parse_hdr failed(header name)\n");
  } else if (r > 0) {
    char buf[MAXLINE];
    sprintf(buf, "error: parse_hdr failed(header value) %s\n", hdr_handler[r].hdr);
    err_return(buf);
  }
  return r;
}

char* get_header_name(char* hdr, char* header_name) {
  while (*hdr == ' ') {
    ++hdr;
  }
  int i, len = strlen(hdr);
  for (i = 0; i < len; ++i) { 
    header_name[i] = hdr[i];
    if (hdr[i] == ':') {
      return hdr + i + 1;
    }
  }
  return NULL;
}

char* Get_header_name(char* hdr, char* header_name) {
  char* p = get_header_name(hdr, header_name);
  if (p == NULL) {
    err_return("het_header_name failed\n");
  }
  return p;
}