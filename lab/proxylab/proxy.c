#include "csapp.h"
#include "http.h"
#include "queue.h"
#include <stdio.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define THREAD_NUM 20
#define BUF_SIZE 100

static const char *USER_AGENT_HDR =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:70.0) Gecko/20100101 "
    "Firefox/70.0\r\n";
static const char *CONNECTION_HDR = "Connection: close\r\n";
static const char *PROXY_CONNECTION_HDR = "Proxy-Connection: close\r\n";

void *thread(void *arg);

void doit(int fd) {
  struct http_request request;
  int rc;
  char buf[MAXLINE];

  if ((rc = http_request_parse(fd, &request)) != 0) {
    printf("ERROR: %s\n", http_error_msg(rc));
    exit(-1);
  }

  printf("hostname: %s\n", request.host);
  printf("uri: %s\n", request.uri);
  printf("version: %s\n", request.http_version);
  printf("method: %s\n", request.method);
  printf("headers:\n");
  for (struct http_header *p = request.headers; p; p = p->next) {
    printf("%s: %s\n", p->header_name, p->content);
  }

  printf("####REQUEST-BEGIN#####\n");
  int connfd = Open_clientfd(request.host, request.port ? request.port : "80");
  sprintf(buf, "%s %s %s\r\n", request.method, request.uri, "HTTP/1.0");
  printf("%s %s %s\r\n", request.method, request.uri, "HTTP/1.0");
  Rio_writen(connfd, buf, strlen(buf));
  for (struct http_header *p = request.headers; p; p = p->next) {
    if (/*strcmp(p->header_name, "User-Agent") == 0 || */
        strcmp(p->header_name, "Connection") == 0 ||
        strcmp(p->header_name, "Proxy-Connection") == 0)
      continue;
    sprintf(buf, "%s: %s\r\n", p->header_name, p->content);
    printf("%s: %s\r\n", p->header_name, p->content);
    Rio_writen(connfd, buf, strlen(buf));
  }
  Rio_writen(connfd, CONNECTION_HDR, strlen(CONNECTION_HDR));
  printf(CONNECTION_HDR);
  // Rio_writen(connfd, USER_AGENT_HDR, strlen(USER_AGENT_HDR));
  printf(USER_AGENT_HDR);
  Rio_writen(connfd, PROXY_CONNECTION_HDR, strlen(PROXY_CONNECTION_HDR));
  printf(PROXY_CONNECTION_HDR);
  Rio_writen(connfd, "\r\n", strlen("\r\n"));
  printf("\r\n");
  printf("####REQUEST-END#####\n");

  rio_t rio;
  Rio_readinitb(&rio, connfd);
  printf("#####RESPONSE-BEGIN#####\n");
  while (Rio_readlineb(&rio, buf, MAXLINE) != 0) {
    printf("%s", buf);
    Rio_writen(fd, buf, strlen(buf));
  }
  printf("#####RESPONSE-END#####\n");
  http_request_free(&request);
}

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
  listenfd = Open_listenfd(argv[1]);

  queue_init(&que, BUF_SIZE);

  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_t id;
    Pthread_create(&id, NULL, thread, &que);
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
  return 0;
}

void *thread(void *arg) {
  int connfd;
  struct queue *que = (struct queue *)arg;

  Pthread_detach(Pthread_self());

  while (1) {
    connfd = queue_get(que);
    // TODO: do it
    doit(connfd);
    Close(connfd);
  }
  return NULL;
}
