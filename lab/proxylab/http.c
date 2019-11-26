#include "http.h"
#include "csapp.h"
#include <string.h>

static const char *ERROR_MSG[] = {
    "",                                                 /* padding */
    "http_request_parse: Invalid request uri",          /* error code: 1 */
    "http_request_parse: Invalid request header",       /* error code: 2 */
    "http_request_parse: Invalid request line",         /* error code: 3 */
    "http_request_parse: Invalid request",              /* error code: 4 */
    "http_request_parse: No host specified in request", /* error code: 5 */
    "http_request_get_header: No header found",         /* error code: 6 */
};

/* string_starts_with: return 0 if s1 starts with s2 */
static int string_starts_with(const char *s1, const char *s2) {
  while (*s1 && *s2 && *s1++ == *s2++)
    ;
  return *s2 == '\0' ? 0 : 1;
}

/* string_strip: remove '\t', ' ', '\r' and  '\n' at the begin of s  */
static void string_strip(const char **s) {
  while (**s == '\t' || **s == '\n' || **s == ' ' || **s == '\r')
    (*s)++;
}

/* string_strip_dup: return a malloc'd string that strips ' ', '\t', '\n', '\t'
 * at the begin and the end of s. */
static char *string_strip_dup(const char *s) {
  char *res, *t;
  const char *p;
  while (*s == '\t' || *s == '\n' || *s == ' ' || *s == '\r')
    s++;

  p = s;
  while (*p++)
    ;
  p -= 2; /* back to the last valid char */

  if (p < s)
    return strdup("");

  while (*p == '\t' || *p == '\n' || *p == ' ' || *p == '\r')
    p--;
  p++;

  res = t = (char *)malloc((p - s) * sizeof(char));
  while (s != p) {
    *t++ = *s++;
  }

  return res;
}

/* string_word_dup: return a malloc'd string of the first word of s */
static char *string_word_dup(const char *s) {
  char *res, *t;
  const char *p;
  while (*s == '\t' || *s == '\n' || *s == ' ' || *s == '\r')
    s++;

  while (*p && *p != '\t' && *p != '\n' && *p != ' ' && *p != '\r')
    p++;

  res = t = (char *)malloc((p - s) * sizeof(char));
  while (s != p) {
    *t++ = *s++;
  }

  return res;
}

/* __request_init: set all fields of requests into NULL */
static void __request_init(struct http_request *request) {
  request->uri = NULL;
  request->headers = NULL;
  request->http_version = NULL;
  request->method = NULL;
  request->host = NULL;
}

static int __request_parse_uri(char *request_uri, char **uri, char **host) {
  int rc = 0;

  if (string_starts_with(request_uri, "http://") == 0) {
    /* supported after HTTP1.1 */
    /* example: http://www.cmu.edu/hub/index.html */
    char *host_start = strchr(request_uri, '/');
    host_start += 2; /* move the pointer to the start of host name */

    char *uri_start = strchr(host_start, '/');
    if (uri_start == NULL) {
      rc = 1;
      goto on_error;
    }

    *uri_start = '\0'; /* temporarily break */
    *host = strdup(host_start);

    *uri_start = '/'; /* recover */
    *uri = strdup(uri_start);
  } else {
    *uri = strdup(request_uri);
  }
on_error:
  return rc;
}

static int __request_parse_method(char *request_method, char **method) {
  int rc = 0;
  int len;
  char *p;

  len = strlen(request_method);
  *method = (char *)malloc(sizeof(char) * len);
  p = *method;

  if (strcasecmp(request_method, "GET") == 0) {
    while ((*p++ = toupper(*request_method++))) /* capital copy */
      ;
  } else {
    /* TODO: parse other methods */
  }

  return rc;
}

static int __request_parse_headers(char *header_str,
                                   struct http_header **header) {
  int rc = 0;
  char *cp;
  struct http_header *hp =
      (struct http_header *)malloc(sizeof(struct http_header));

  /* header name */
  if ((cp = strchr(header_str, ':')) == NULL) {
    rc = 2;
    goto on_error;
  }
  *cp = '\0'; /* break header_str in ':' */
  hp->header_name = strdup(header_str);

  /* header content */
  header_str = cp + 2; /* skip ": "  */

  hp->content = string_strip_dup(header_str);

  /* add to headers list */
  hp->next = *header;
  *header = hp;

on_error:
  if (rc)
    free(hp);

  return rc;
}

int http_request_parse(int connfd, struct http_request *request) {
  int rc = 0;
  char buf[MAXLINE], *cp, *space;
  rio_t rio;

  __request_init(request);

  Rio_readinitb(&rio, connfd);

  if (Rio_readlineb(&rio, buf, MAXLINE) == 0) {
    rc = 4;
    goto on_error;
  }

  /* get the method name */
  if ((space = strchr(buf, ' ')) == NULL) {
    rc = 3;
    goto on_error;
  }
  *space = '\0';
  if ((rc = __request_parse_method(buf, &request->method)) != 0)
    goto on_error;

  /* get the uri */
  cp = space + 1;
  if ((space = strchr(cp, ' ')) == NULL) {
    rc = 3;
    goto on_error;
  }
  *space = '\0';
  if ((rc = __request_parse_uri(cp, &request->uri, &request->host)) != 0)
    goto on_error;

  /* get HTTP version */
  cp = space + 1;
  request->http_version = string_strip_dup(cp);

  /* get HTTP headers */
  while (Rio_readlineb(&rio, buf, MAXLINE) != 0) {
    if (string_starts_with(buf, "\r\n") == 0)
      break;
    // printf("header line: \n%s", buf);
    __request_parse_headers(buf, &request->headers);
  }

  if (request->host == NULL) {
    if ((rc = http_request_get_header(request, "Host", buf, MAXLINE)) != 0) {
      rc = 5;
      goto on_error;
    }
    request->host = strdup(buf);
  }

  /* HTTP body TODO */

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
    struct http_header *p, *q;
    p = request->headers;
    while (p) {
      q = p->next;
      free(p);
      p = q;
    }
  }
  return 0;
}

int http_request_get_header(const struct http_request *request,
                            const char *header_name, char *content,
                            size_t content_len) {
  struct http_header *p;

  for (p = request->headers; p; p = p->next) {
    if (strcmp(header_name, p->header_name) == 0) {
      strncpy(content, p->content, content_len);
      return 0;
    }
  }

  return 6; /* no header found */
}
