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

  while((c=getopt(argc, argv, "tT")) != -1) {
    switch(c) {
      case 't':
        tA = 1; break;
      case 'T':
        tB = 1; break;
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

  std::string k2_1_path = argv[1];
  std::string k2_2_path = argv[2];

  FILE *f;
  f = fopen(k2_1_path.c_str(), "r");
  matrix M1 = matLoad(f);
  fclose(f);
  if(tA) *M1 = matTranspose(M1);


  f = fopen(k2_2_path.c_str(), "r");
  matrix M2 = matLoad(f);
  fclose(f);
  if(tB) *M2 = matTranspose(M2);

  matrix M3 = matMult(M1, M2);
  k2_1_path += ".prod";
  f = fopen(k2_1_path.c_str(), "w");
  matSave(M3, f);
  fclose(f);

  matDestroy(M1);
  matDestroy(M2);
  matDestroy(M3);

}

void usage_and_exit(char* name) {
  fprintf(stderr, "Usage:\n\t%s [options] filename1 filename2 \n\n", name);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "\t-t      transpose matrix in filename1\n");    
  fprintf(stderr, "\t-T      transpose matrix in filename2\n");    
  fprintf(stderr, "\t-h      show this help message\n");    
  fprintf(stderr, "Multiply matrices in filename1 and filename2\n\n");
  exit(1);
}
