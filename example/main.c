#include <cfgfile.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "error: number of parameters mistmatch"
               " (expected 2, specified %d)\n", argc);
    printf("usage: %s filename\n", argv[0]);
    return 1;
  }
  CFGFILE cfg;
  if (cfgopen(argv[1], &cfg) != 0) {
    fprintf(stderr, "error: unable to open the configuration file '%s'\n", argv[1]);
    return 2;
  }
  char *v = NULL;
  if (cfgevals(&cfg, "PATH", &v) != 0) {
    fprintf(stderr, "error: unable to evaluate PATH value"
               " in the configuration file '%s'\n", argv[1]);
    return 3;
  }
  printf("PATH='%s'\n", v);
  free(v);
  if (cfgclose(&cfg) != 0) {
    fprintf(stderr, "error: unable to close the configuration file '%s'\n", argv[1]);
    return 4;
  }
  return 0;
}
