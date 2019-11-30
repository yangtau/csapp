#include "cache.h"
#include "csapp.h"
#include "http.h"
#include "queue.h"
#include <stdio.h>

/* Recommended max cache and object sizes */
#define THREAD_NUM 20
#define BUF_SIZE 100

static const char *USER_AGENT_HDR =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:70.0) Gecko/20100101 "
    "Firefox/70.0\r\n";
static const char *CONNECTION_HDR = "Connection: close\r\n";
static const char *PROXY_CONNECTION_HDR = "Proxy-Connection: close\r\n";

static struct cache cache;

int put_into_cache(struct http_request *request,
                   struct http_response *response);

/* find_in_cache: return 0 if hit */
int find_in_cache(struct http_request *request, struct http_response *response);

void *task(void *arg);

void forward_request(int connfd, struct http_request request);

void bad_request(int connfd);

void forward_response(int fd, struct http_response response);

void show_response(struct http_response response);

void show_request(struct http_request request);

int main(int argc, char **argv) {
  struct queue que;
  int listenfd, connfd;
  char client_hostname[MAXLINE], client_port[MAXLINE];
  struct sockaddr_storage clientaddr;
  socklen_t clientlen = sizeof(clientaddr);

  if (argc != 2) {
    printf("Usage: %s port\n", argv[0]);
    return 0;
  }

  queue_init(&que, BUF_SIZE);
  cache_init(&cache);

  listenfd = Open_listenfd(argv[1]);

  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_t id;
    Pthread_create(&id, NULL, task, &que);
  }

  while (1) {
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                client_port, MAXLINE, 0);

    printf("Accepted connection from (%s, %s)\n", client_hostname, client_port);

    queue_put(&que, connfd);
  }

  // Never be executed
  Close(listenfd);
  queue_free(&que);
  cache_free(&cache);
  return 0;
}

void *task(void *arg) {
  int client_fd, server_fd;
  struct queue *que = (struct queue *)arg;
  struct http_request request;
  struct http_response response;
  int rc;

  Pthread_detach(Pthread_self());

  while (1) {
    client_fd = queue_get(que);

    if ((rc = http_request_parse(client_fd, &request)) != 0) {
      printf("Log http_request_parse_error: %s\n", http_error_msg(rc));
      bad_request(client_fd);

      http_request_free(&request);
      Close(client_fd);
      continue;
    }
    show_request(request);

    if (find_in_cache(&request, &response) == 0) {
      printf("LOG: found in cache\n");
    } else {
      /* not found in cache */
      server_fd =
          Open_clientfd(request.host, request.port ? request.port : "80");
      forward_request(server_fd, request);

      if ((rc = http_response_parse(server_fd, &response)) != 0) {
        printf("Log http_response_parse_error: %s\n", http_error_msg(rc));

        http_response_free(&response);
        Close(server_fd);
        continue;
      }
    }
    show_response(response);

    put_into_cache(&request, &response);

    forward_response(client_fd, response);

    http_request_free(&request);
    http_response_free(&response);
    Close(server_fd);
    Close(client_fd);
  }
  return NULL;
}

void forward_request(int fd, struct http_request request) {
  char buf[MAXLINE];

  /* request line: replace http_version with HTTP/1.0 */
  sprintf(buf, "%s %s %s\r\n", request.method, request.uri, "HTTP/1.0");
  Rio_writen(fd, buf, strlen(buf));

  /* headers */
  for (struct http_header *p = request.headers; p; p = p->next) {
    if (/*strcmp(p->header_name, "User-Agent") == 0 || */
        strcmp(p->header_name, "Connection") == 0 ||
        strcmp(p->header_name, "Proxy-Connection") == 0)
      continue;
    sprintf(buf, "%s: %s\r\n", p->header_name, p->content);
    Rio_writen(fd, buf, strlen(buf));
  }

  /* custom Connection and Proxy-Connection */
  Rio_writen(fd, CONNECTION_HDR, strlen(CONNECTION_HDR));
  Rio_writen(fd, PROXY_CONNECTION_HDR, strlen(PROXY_CONNECTION_HDR));
  // Rio_writen(connfd, USER_AGENT_HDR, strlen(USER_AGENT_HDR));
  Rio_writen(fd, "\r\n", strlen("\r\n"));
}

void forward_response(int fd, struct http_response response) {
  char buf[MAXLINE];

  /* status line */
  sprintf(buf, "%s %d %s\r\n", "HTTP/1.0", response.status_code,
          response.reason_pharse);
  Rio_writen(fd, buf, strlen(buf));
  /* headers */
  for (struct http_header *p = response.headers; p; p = p->next) {
    sprintf(buf, "%s: %s\r\n", p->header_name, p->content);
    Rio_writen(fd, buf, strlen(buf));
  }

  Rio_writen(fd, "\r\n", strlen("\r\n")); /* CR LF */

  /* body */
  if (response.body_len != 0) {
    Rio_writen(fd, response.body, response.body_len);
  }
}

void bad_request(int fd) {
  char buf[MAXLINE];

  /* status line */
  sprintf(buf, "%s %d %s\r\n", "HTTP/1.0", 400, "Bad Request");
  Rio_writen(fd, buf, strlen(buf));

  Rio_writen(fd, "\r\n", strlen("\r\n")); /* CR LF */
}

void show_response(struct http_response response) {
  printf("#####RESPONSE-BEGIN#####\n");
  printf("http_version:%s\n", response.http_version);
  printf("status code:%d\n", response.status_code);
  printf("status msg:%s\n", response.reason_pharse);
  printf("headers:\n");
  for (struct http_header *p = response.headers; p; p = p->next) {
    printf("%s: %s\n", p->header_name, p->content);
  }
  printf("#####RESPONSE-END#####\n");
}

void show_request(struct http_request request) {
  printf("#####REQUEST-BEGIN#####\n");
  printf("host: %s\n", request.host);
  printf("port: %s\n", request.port);
  printf("uri: %s\n", request.uri);
  printf("version: %s\n", request.http_version);
  printf("method: %s\n", request.method);
  printf("headers:\n");
  for (struct http_header *p = request.headers; p; p = p->next) {
    printf("%s: %s\n", p->header_name, p->content);
  }
  printf("#####REQUEST-END#####\n");
}

int put_into_cache(struct http_request *request,
                   struct http_response *response) {
  int rc = 0;
  char id_buf[MAXLINE];

  sprintf(id_buf, "%s %s:%s%s", request->method, request->host,
          request->port ? request->port : "", request->uri);

  if ((rc = cache_put(&cache, id_buf, response->body, response->body_len)) !=
      0) {
    printf("Cache Log: %s\n", cache_error_msg(rc));
  }

  return rc;
}

int find_in_cache(struct http_request *request,
                  struct http_response *response) {
  int rc, len;
  char str_buf[MAXLINE];
  char buf[MAX_OBJECT_SIZE];

  sprintf(str_buf, "%s %s:%s%s", request->method, request->host,
          request->port ? request->port : "", request->uri);

  if ((rc = cache_get(&cache, str_buf, buf, MAX_OBJECT_SIZE, &len)) != 0) {
    return rc;
  }

  // TODO: format response in http.c
  response->status_code = 200;
  response->reason_pharse = strdup("OK");
  response->body_len = len;
  response->http_version = NULL;

  response->body = malloc(len);
  memcpy(response->body, buf, len);

  struct http_header *hp = malloc(sizeof(struct http_header));
  hp->next = NULL;
  hp->header_name = strdup("Content-Length");
  sprintf(str_buf, "%d", len);
  hp->content = strdup(str_buf);

  response->headers = hp;

  return rc;
}
