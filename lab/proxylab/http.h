/*
 * Several thread-safe functions handling HTTP Requests and HTTP responses.
 *
 */
#ifndef __HTTP_H__
#define __HTTP_H__

struct http_header {
  char *header_name;
  char *content;
  struct http_header *next;
};

struct http_body {
  char *content;
};

struct http_request {
  char *host;
  char *uri;
  char *method;
  char *http_version;
  struct http_header *headers;
  struct http_body *body;
};

struct http_response {};

int http_request_parse(const char *request_raw, struct http_request *request);

int http_request_free(struct http_request *request);

int http_request_get_header(const struct http_request *request,
                            const char *hader_name, char *content);

int http_request_get_host(const struct http_request *request, char *host);

#endif /* __HTTP_H__ */
