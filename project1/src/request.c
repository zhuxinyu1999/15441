#include "request.h"

request_t* request_init(int type) {
  request_t* req = Calloc(1, sizeof(request_t));
  if (req != NULL) {
    req->type = type;
  }
  return req;
}

request_t* Request_init(int type) {
  request_t* req = request_init(type);
  if (req == NULL) {
    err_return("req init failed\n");
  }
  return req;
}

void request_deinit(request_t* req) {
  if (req->body != NULL) {
    free(req->body);
  }
  free(req);
}


int parse(httpio_t* hio, request_t* req, int* pipeline, int* conn) {
  
  int state;

  if (state = Parse_request_line(hio, req)) {
    return state;
  }

  if (state = Parse_request_header(hio, req)) {
    return state;
  }

  if (state = Get_request_body(hio, req)) {
    return state;
  }

  *pipeline = !httpio_empty(hio);
  *conn = req->header.connection;
  
  return 0;
}

int Parse(httpio_t* hio, request_t* req, int* pipeline, int* conn) {
  int r = parse(hio, req, pipeline, conn);
  if (r == ERR_SYS) {
    err_return("parse failed (err_io)\n");
  } else if (r > 0) {
    err_return("parse failed (err_request)\n");
  }
  return r;
}



int parse_request_line(httpio_t* hio, request_t* req) {
  int buflen;
  char buf[MAXLINE];

  if ((buflen = Httpio_readline(hio, buf, MAXLINE)) > 0) {
    sscanf(buf, "%s %s %s", req->method, req->uri, req->version);
  } else if (buflen == ERR_SYS) {
    return ERR_SYS;
  } else if (buflen == ERR_LEN) {
    return BAD_REQUEST;
  }
  write_log(buf);

  if (strcmp(req->method, "GET") &&
      strcmp(req->method, "HEAD") &&
      strcmp(req->method, "POST")) {
    return NOT_IMPLEMENTED;
  }

  if (strcasecmp("HTTP/1.1", req->version)) {
    return HTTP_VERSION_NOT_SUPPORTED;
  }

  if (Parse_uri(req)) {
    return BAD_REQUEST;
  }

  /* HEAD has no request body */
  if (req->header.content_length != 0 && !strcmp(req->method, "GET")) {
    return BAD_REQUEST;
  }

  /* do not support HEAD + cgi */
  if (req->is_cgi == CGI && !strcmp(req->method, "HEAD")) {
    return BAD_REQUEST;
  }

  /* do not support POST + static */
  if (req->is_cgi == STATIC && !strcmp(req->method, "POST")) {
    return BAD_REQUEST;
  }

  

  return 0;
}

int Parse_request_line(httpio_t* hio, request_t* req) {
  int state = parse_request_line(hio, req);
  if (state) {
    err_return("parse req line failed\n");
  }
  return state;
}

int parse_uri(request_t* req) {
  /* path begin */
  char* ptr1;
  if ((ptr1 = (strstr(req->uri, "://"))) != NULL) {
    if ((ptr1 = strstr(ptr1 + 3, "/")) == NULL) {
      return BAD_REQUEST;
    }
  } else {
    ptr1 = req->uri;
  }

  if (strstr(ptr1, cgi_path) != NULL) {
    req->is_cgi = CGI;
  } else {
    req->is_cgi = STATIC;
  }

  /* query string begin */
  char* ptr2;

  if (req->is_cgi == CGI) {
    if ((ptr2 = strstr(ptr1, "?")) != NULL) {
      strncpy(req->filename, ptr1, ptr2 - ptr1);
      ++ptr2;
      strcpy(ptr2, req->qs);
    } else {
      strcpy(req->filename, ptr1);
    }
  } else {
    if ((ptr2 = strstr(ptr1, "?")) != NULL) {
      return BAD_REQUEST;
    }
    /* abs_path */
    if (strstr(ptr1, www_folder) != NULL) {
      strcpy(req->filename, ptr1);
    } else { /* relative_path */
      strcpy(req->filename, www_folder);
      if (!strcmp(ptr1, "/")) {
        strcat(req->filename, "/home.html");
      } else {
        strcat(req->filename, ptr1);
      }
    }
  }

  get_filetype(req);
  return 0;
}

int Parse_uri(request_t* req) {
  int state = parse_uri(req);
  if (state) {
    err_return("parse_uri failed\n");
  }
  return state;
}

void get_filetype(request_t* req) {
  char* name = req->filename;
  char* type = req->filetype;
  if (strstr(name, ".html")) {
    strcpy(type, "text/html");
  } else if (strstr(name, ".gif")) {
    strcpy(type, "image/gif");
  } else if (strstr(name, ".png")) {
    strcpy(type, "image/png");
  } else if (strstr(name, ".jpg")) {
    strcpy(type, "image/jpeg");
  } else {
    strcpy(type, "text/plain");
  }
} 


int parse_request_header(httpio_t* hio, request_t* req) {
  int buflen;
  char buf[MAXLINE];

  while ((buflen = Httpio_readline(hio, buf, MAXLINE)) > 0 && strcmp(buf, "\r\n")) {
    write_log(buf);
    if (Parse_hdr(buf, &(req->header)) < 0) {
      return BAD_REQUEST;
    }
  }
  if (buflen == ERR_SYS) {
    return ERR_SYS;
  }
  if (buflen == ERR_LEN || buflen == 0) {
    return BAD_REQUEST;
  }
  write_log(buf);
  return 0;
}

int Parse_request_header(httpio_t* hio, request_t* req) {
  int state = parse_request_header(hio, req);
  if (state) {
    err_return("parse req header failed\n");
  }
  return state;
}


int get_request_body(httpio_t* hio, request_t* req) {
  if (req->header.transfer_encoding == TE_IDENTITY && req->header.content_length == 0) {
    return 0;
  }
  int total_len;
  if (req->header.transfer_encoding == TE_CHUNKED) {
    if ((total_len = Chunked_cpy_body(hio, req)) == ERR_SYS) {
      return ERR_SYS;
    } else if (total_len == ERR_LEN) {
      return REQUEST_ENTITY_TOO_LARGE;
    }
  } else {
    if ((total_len = Cpy_body(hio, req)) == ERR_SYS) {
      return ERR_SYS;
    } else if (total_len == ERR_LEN) {
      return REQUEST_ENTITY_TOO_LARGE;
    }
  }
  write_nlog(req->body, total_len);
  write_nlog("\n", 1);
  return 0;
}

int Get_request_body(httpio_t* hio, request_t* req) {
  int state = get_request_body(hio, req);
  if (state) {
    err_return("get req body failed\n");
  }
  return state;
}

int cpy_body(httpio_t* hio, request_t* req) {
  int body_len = req->header.content_length;
  if (body_len == 0) {
    return 0;
  }
  if (body_len > MAX_BODY) {
    return ERR_LEN;
  }
  if ((req->body = Calloc(1, body_len)) == NULL) {
    return ERR_SYS;
  }
  if (Httpio_readn(hio, body_len, req->body, MAXLINE) != body_len) {
    return ERR_SYS;
  }
  return body_len;
}

int Cpy_body(httpio_t* hio, request_t* req) {
  int rn = cpy_body(hio, req);
  if (rn == ERR_SYS) {
    err_return("cpy_body failed\n");
  }
  return rn;
}

int chunked_cpy_body(httpio_t* hio, request_t* req) {
  if ((req->body = Calloc(1, MAX_BODY)) == NULL) {
    return ERR_SYS;
  }
  char* body = req->body;
  int len, nleft = MAX_BODY;
  char buf[MAXLINE];
  while(1) {
    if (Httpio_readline(hio, buf, MAXLINE) < 0) {
      return ERR_SYS;
    }
    /* hex */
    sscanf(buf, "%x", len);
    if (len == 0) {
      break;
    }
    if (nleft < len) {
      return ERR_LEN;
    }
    if (Httpio_readline(hio, buf, MAXLINE) != len) {
      return ERR_SYS;
    }
    memcpy(body, buf, len);
    body += len;
    nleft -= len;
  }
  req->header.content_length = MAX_BODY - nleft;
  return MAX_BODY - nleft;
}

int Chunked_cpy_body(httpio_t* hio, request_t* req) {
  int rn = chunked_cpy_body(hio, req);
  if (rn == ERR_SYS) {
    err_return("chunked_cpy_body failed(err_io)\n");
  } else if (rn == ERR_LEN) {
    err_return("chunked_cpy_body failed(err_len)\n");
  }
  return rn;
}

