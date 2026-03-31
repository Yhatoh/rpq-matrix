//
// Created by Adrián on 22/4/24.
//
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <getopt.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <utility>
#include <vector>
#include <cassert>

void usage_and_exit(char* name);

extern "C" {
#include "k2-tree/matrix.h"
}

static const uint8_t debruijn64_mapping[64] = {
  63,  0, 58,  1, 59, 47, 53,  2,
  60, 39, 48, 27, 54, 33, 42,  3,
  61, 51, 37, 40, 49, 18, 28, 20,
  55, 30, 34, 11, 43, 14, 22,  4,
  62, 57, 46, 52, 38, 26, 32, 41,
  50, 36, 17, 19, 29, 10, 13, 21,
  56, 45, 25, 31, 35, 16,  9, 12,
  44, 24, 15,  8, 23,  7,  6,  5
};

static const uint64_t debruijn64 = 0x07EDD5E59A4E28C2ULL;

inline uint8_t bit_position(uint64_t x){
    return debruijn64_mapping[(x * debruijn64) >> 58];
}

inline uint8_t msb(uint64_t x, unsigned long& ret){
  if (!x)
    return false;

  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x |= x >> 32;

  x ^= x >> 1;
  ret = bit_position(x);

  return true;
}

inline uint8_t msb(uint64_t x){
  unsigned long ret = -1U;
  msb(x, ret);
  return (uint8_t)ret;
}

inline uint64_t ceil_log2(uint64_t x) {
  return (x > 1) ? msb(x - 1) + 1 : 0;
}

inline uint64_t floor_log2(uint64_t x) {
  return (x > 1) ? msb(x) : 0;
}

int main(int argc, char** argv) {

  extern char *optarg;
  extern int optind, opterr, optopt;

  int c;
  bool check = 0;
  uint32_t size = 0;

  while((c=getopt(argc, argv, "hs:c")) != -1) {
    switch(c) {
      case 's':
        size = std::atoll(optarg); break;
      case 'c':
        check = 1; break;
      case 'h':
        usage_and_exit(argv[0]); break;
      case '?':
        fprintf(stderr, "Unkown option: %s", optarg);
        exit(1);
    }
  }

  optind -= 1;
  if(argc - optind != 2) usage_and_exit(argv[0]);
  argv += optind; argc -= optind;

  std::string path_file = argv[1];

  std::vector< std::pair< uint32_t, uint32_t > > ones;

  std::ifstream ones_txt;
  ones_txt.open(path_file);

  if(!ones_txt.is_open()) {
    std::cerr << "Error opening file. Check if the file exists or the path is writed correctly" << std::endl;
    exit(1);
  }

  {
    uint32_t x, y;

    uint32_t max_index = 0;
    while(ones_txt >> x >> y) {
      ones.push_back({x, y});
      max_index = std::max(x, max_index);
      max_index = std::max(y, max_index);
    }
    if(size == 0) size = max_index + 1;
  }
  size = ceil_log2(size);
  std::sort(ones.begin(), ones.end());

  ones_txt.close();

  uint64_t m = ones.size();
  uint* coords = (uint *) myalloc(2 * m * sizeof(uint));
  for(uint64_t i = 0; i < m; i++) {
    coords[i * 2] = ones[i].first;
    coords[i * 2 + 1] = ones[i].second;
  }

  matrix M = matCreate32(1 << size, 1 << size, m, coords);

  if(check) {
    uint32_t* buffer = (uint32_t*) malloc(sizeof(uint32_t) * 2 * m);
    uint64_t m_q = matCollect32(M, 0, (1 << size) - 1, 0, (1 << size) - 1, buffer);
    std::vector< std::pair< uint32_t, uint32_t > > check_ones;
    for(uint64_t i = 0; i < 2 * m; i += 2) {
      check_ones.push_back({buffer[i], buffer[i + 1]});
    }
    std::sort(check_ones.begin(), check_ones.end());
    for(uint64_t i = 0; i < m; i++) {
      if(check_ones[i] != ones[i]) {
        std::cout << "Error during decompression" << std::endl;
        exit(1);
      }
    }
    myfree(buffer);
  }
  FILE *f;

  path_file += ".k2bfs";
  f = fopen(path_file.c_str(), "w");
  if (f == NULL) {
    fprintf(stderr, "Error: cannot create file %s\n", path_file.c_str());
    exit(1);
  }
  matSave(M, f);
  fclose(f);
  matDestroy(M);
  myfree(coords);
  if(check) {
    f = fopen(path_file.c_str(), "r");
    if (f == NULL) {
      fprintf(stderr, "Error: cannot open file %s\n", path_file.c_str());
      exit(1);
    }
    M = matLoad(f);
    uint32_t* buffer = (uint32_t*) malloc(sizeof(uint32_t) * 2 * m);
    uint64_t m_q = matCollect32(M, 0, (1 << size) - 1, 0, (1 << size) - 1, buffer);

    std::vector< std::pair< uint32_t, uint32_t > > check_ones;
    for(uint64_t i = 0; i < 2 * m; i += 2) {
      check_ones.push_back({buffer[i], buffer[i + 1]});
    }
    std::sort(check_ones.begin(), check_ones.end());
    for(uint64_t i = 0; i < m; i++) {
      if(check_ones[i] != ones[i]) {
        std::cout << "Error during decompression" << std::endl;
        exit(1);
      }
    }
  }
}

void usage_and_exit(char* name) {
  fprintf(stderr, "Usage:\n\t%s [options] filename \n\n", name);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "\t-s      size of matrix (if is not indicated size will be maxindex + 1)\n");    
  fprintf(stderr, "\t-c      check if matrix compression is correct\n");    
  fprintf(stderr, "\t-h      show this help message\n");    
  fprintf(stderr, "Compressed matrix in filename\n\n");
  exit(1);
}
