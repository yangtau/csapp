#include "http.h"
#include "csapp.h"
#include <string.h>

static char *ERROR_MSG[] = {
    "",                    /* padding */
    "Invalid request uri", /* error code 1 */
};

/* string_starts_with: return 0 if s1 starts with s2 */
static int string_starts_with(const char *s1, const char *s2) {
  while (*s1 && *s2 && *s1++ == *s2++)
    ;
  return *s2 == '\0' ? 0 : 1;
}

/* __request_init: set all fields of requests into NULL */
static void __request_init(struct http_request *request) {
  request->uri = NULL;
  request->headers = NULL;
  request->http_version = NULL;
  request->method = NULL;
  request->host = NULL;
}

static int __request_parse_uri(const char *request_uri,
                               struct http_request *request) {
  int rc = 0;

  if (string_starts_with(request_uri, "http://") == 0) {
    /* suppored after HTTP1.1 */
    /* example: http://www.cmu.edu/hub/index.html */
    char *host_start = strchr(request_uri, '/');
    host_start += 2; /* move the pointer to the start of host name */

    char *uri_start = strchr(host_start, '/');
    if (uri_start == NULL) {
      rc = 1;
      goto on_error;
    }

    *uri_start = '\0'; /* temporarily break */

    request->host = strdup(host_start);

    *uri_start = '/'; /* recover */

    request->uri = strdup(uri_start);
  } else {
    request->uri = strdup(request_uri);
  }

on_error:
  return rc;
}

static int __request_parse_method(const char *method,
                                  struct http_request *request) {
  int rc = 0;
  int len;
  char *p;

  len = strlen(method);
  request->method = (char *)malloc(sizeof(char) * len);
  p = request->method;

  if (strcasecmp(method, "GET") == 0) {
    while ((*p++ = toupper(*method++))) /* capital copy */
      ;
  } else {
    /* TODO: parse other methods */
  }

  return rc;
}

static int __request_parse_headers(const char *headers,
                                   struct http_request *request) {
  int rc = 0;
  char buf[MAXLINE];
  /*
  for (;;) {
    sscanf(headers, "%s", buf);
  }
  */

  return rc;
}

int http_request_parse(const char *request_raw, struct http_request *request) {
  int rc = 0;
  char buf[MAXLINE];

  __request_init(request);

  /* get the method name */
  sscanf(request_raw, "%s", buf);
  if ((rc = __request_parse_method(buf, request)) != 0)
    goto on_error;

  request_raw += strlen(buf) + 1; /* skip space */
  if (!isspace(request_raw[-1])) {
    rc = 1;
    goto on_error;
  }

  /* get the uri */
  sscanf(request_raw, "%s", buf);
  if ((rc = __request_parse_uri(buf, request)) != 0)
    goto on_error;

  request_raw += strlen(buf) + 1; /* skip space */
  if (!isspace(request_raw[-1])) {
    rc = 1;
    goto on_error;
  }

  /* get HTTP version */
  sscanf(request_raw, "%s", buf);
  request->http_version = strdup(buf);

  request_raw += strlen(buf) + 2; /* skip CR LF */
  if (request_raw[-2] != '\r' || request_raw[-1] != '\n') {
    rc = 1;
    goto on_error;
  }

  /* get HTTP headers */
  if ((rc = __request_parse_headers(request_raw, request)) != 0)
    goto on_error;

on_error:
  return rc;
}

int http_request_free(struct http_request *request) {
  if (request->uri)
    free(request->uri);

  if (request->method)
    free(request->method);

  if (request->http_version)
    free(request->http_version);

  if (request->host)
    free(request->host);

  if (request->headers) {
    /* TODO: free the list of headers */
    free(request->headers);
  }
  return 0;
}
