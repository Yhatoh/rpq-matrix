#include <getopt.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

void usage_and_exit(char* argv);

extern "C" {
#include "k2-tree/matrix.h"
}

int main(int argc, char* argv[]) {
  extern char *optarg;
  extern int optind, opterr, optopt;

  int c;

  while((c=getopt(argc, argv, "h")) != -1) {
    switch(c) {
      case 'h':
        usage_and_exit(argv[0]);
      case '?':
        fprintf(stderr, "Unkown option: %s", optarg);
        exit(1);
    }
  }

  optind -= 1;
  if(argc - optind != 2) usage_and_exit(argv[0]);
  argv += optind; argc -= optind;

  char fname[1000];
  strcpy(fname, argv[1]);

  FILE *f;
  f = fopen(fname, "r");
  matrix M1 = matLoad(f);
  fclose(f);
  matInfo(M1);
  matDestroy(M1);

  return 0;
}

void usage_and_exit(char* name) {
  fprintf(stderr, "Usage:\n\t%s [options] filename \n\n", name);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "\t-h      show this help message\n");    
  fprintf(stderr, "Show info about compressed matrix in filename\n\n");
  exit(1);
}
