#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

static char *prompt = "Usage: myls option [dirname]";

static void listdir(const char *dirname, int all_flag, int list_flag) {
  DIR *dir;
  if ((dir = opendir(dirname)) == NULL) {
    printf("Failed to open dir: %s\n", dirname);
  }

  struct dirent *dire;
  while ((dire = readdir(dir)) != NULL) {
    if (!all_flag && dire->d_name[0] == '.') {
      continue;
    }
    printf("%s%c", dire->d_name, list_flag ? '\n' : '\t');
  }
  if (!list_flag) {
    printf("\n");
  }
  closedir(dir);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("%s\n", prompt);
    return -1;
  }

  char opt;
  int all_flag = 0;
  int list_flag = 0;

  while ((opt = getopt(argc, argv, "al")) != EOF) {
    switch (opt) {
    case 'a':
      all_flag = 1;
      break;
    case 'l':
      list_flag = 1;
      break;
    case '?':
    default:
      /* Error: argc */
      exit(-1);
      break;
    }
  }

  /* the last argument should be the dirname */
  if (argv[argc - 1][0] == '-') {
    printf("%s\n", prompt);
    exit(-1);
  }

  listdir(argv[argc - 1], all_flag, list_flag);
  return 0;
}
