/*
 * Several thread-safe functions handling HTTP Requests and HTTP responses.
 *
 */
#ifndef __HTTP_H__
#define __HTTP_H__
#include <stddef.h>

struct http_header {
  char *header_name;
  char *content;
  struct http_header *next;
};

struct http_request {
  char *host;
  char *port;
  char *uri;
  char *method;
  char *http_version;
  struct http_header *headers;
  char *body;
};

struct http_response {};

const char *http_error_msg(int error_code);

int http_request_parse(int connfd, struct http_request *request);

int http_request_free(struct http_request *request);

int http_request_get_header(const struct http_request *request,
                            const char *header_name, char *content,
                            size_t content_len);
#endif /* __HTTP_H__ */
