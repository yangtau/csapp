#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char **argv) {
  struct addrinfo *listp, hints;
  int rc, flags;
  char buf[1000];

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET; /* IPv4 only */
  hints.ai_socktype = SOCK_STREAM;
  // hints.ai_flags = AI_CANONNAME;

  if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0) {
    printf("%s\n", "Error: getaddrinfo");
  }

  for (struct addrinfo *p = listp; p; p = p->ai_next) {
    if ((rc = getnameinfo(p->ai_addr, p->ai_addrlen, buf, 1000, NULL, 0,
                          NI_NUMERICHOST)) != 0) {
      printf("%s\n", "Error: getnameinfo");
    }
    printf("%s\n", buf);
  }

  freeaddrinfo(listp);

  return 0;
}
