// std includes
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <dirent.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <ctime>
#include <climits>

void usage_and_exit(char* name);

extern "C" {
#include "k2-tree/matrix.h"
}

int main(int argc, char** argv) {

  extern char *optarg;
  extern int optind, opterr, optopt;

  int c;
  bool tA, tB;
  tA = tB = 0;

  while((c=getopt(argc, argv, "t")) != -1) {
    switch(c) {
      case 't':
        tA = 1; break;
      case 'h':
        usage_and_exit(argv[0]);
      case '?':
        fprintf(stderr, "Unkown option: %s", optarg);
        exit(1);
    }
  }

  optind -= 1;
  if(argc - optind != 3) usage_and_exit(argv[0]);
  argv += optind; argc -= optind;

  std::string k2_path = argv[1];
  std::string vec_path = argv[2];

  FILE *f;
  f = fopen(k2_path.c_str(), "r");
  matrix M1 = matLoad(f);
  fclose(f);
  if(tA) *M1 = matTranspose(M1);

  double* v_test = (double*) malloc(sizeof(double) * M1->width);
  f = fopen(vec_path.c_str(), "r");
  size_t bits_read = fread(v_test, sizeof(double), M1->width, f);
  if(bits_read != M1->width) {
    fprintf(stderr, "reading incorrectly from vecto file");
  }

  double not_opt = 0;
  for(size_t i = 0; i < 1000; i++) {
    double* ret = matVectorMult(M1, v_test);
    free(v_test); v_test = NULL;
    v_test = ret;
    not_opt += v_test[0];
  }

  printf("%f\n", not_opt);
  matDestroy(M1);

}

void usage_and_exit(char* name) {
  fprintf(stderr, "Usage:\n\t%s [options] filename1 filename2 \n\n", name);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "\t-t      transpose matrix in filename1\n");    
  fprintf(stderr, "\t-h      show this help message\n");    
  fprintf(stderr, "matrix-vector multiplication by filename1 and filename2\n\n");
  exit(1);
}
