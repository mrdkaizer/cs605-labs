#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "check.h"

#define P  (void)printf
#define FP (void)fprintf

#define PAGESZ 4096

#define ALIGNMENT PAGESZ

typedef struct {
  uint64_t next;
  char padding[0];
} element_t;

typedef struct {
  uint64_t N;
  uint64_t element_size;
  element_t* head;
  int fd;
  char* buf;
} chain_t;

typedef uint64_t (*read_element_t)(chain_t* chain, off_t index, size_t access_size);

static inline uint64_t min(uint64_t a, uint64_t b)
{
  return a < b ? a : b;
}

/* G. Marsaglia, 2003.  "Xorshift RNGs", Journal of Statistical
   Software v. 8 n. 14, pp. 1-6, discussed in _Numerical Recipes_
   3rd ed. */
static uint64_t prng(uint64_t* seed) {
  uint64_t x = *seed;
  x ^= x >> 21;
  x ^= x << 35;
  x ^= x >>  4;
  *seed = x;
  return x;
}

static uint64_t T(void) {
  struct timeval tv;

#ifndef NDEBUG
  int r =
#endif
  gettimeofday(&tv, NULL);

  assert(0 == r);

  return (uint64_t)(tv.tv_sec) * 1000000 + tv.tv_usec;
}

element_t* element(chain_t* chain, uint64_t index) 
{
  char* p = (char*) chain->head + index * chain->element_size;
  return (element_t *) p;
}

static inline uint64_t read_element_memory(chain_t* chain, off_t index, size_t access_size)
{
  element_t* e = element(chain, index);
  memcpy(chain->buf, e->padding, access_size-sizeof(*e));
  return e->next;
}

static inline uint64_t read_element_fileio(chain_t* chain, off_t index, size_t access_size)
{
  int ret;
  element_t* e = (element_t*) chain->buf;
  CK2((ret = pread(chain->fd, chain->buf, access_size, index * chain->element_size)) != -1, strerror(errno));
  return e->next;
}

// Creates chain into file <path>
static void create_chain(const char* path, uint64_t seedin, uint64_t N, size_t element_size, size_t access_size)
{
  int fd;
  int ret;
  uint64_t sum, p, i;
  element_t *B;
  char *A, *Aaligned, *M;
  uint64_t seed = seedin;
  chain_t* chain;
  size_t file_size;
  int oflag;

  // fill B[] with random permutation of 1..N
  chain = (chain_t*) malloc(sizeof(chain_t));
  chain->N = N;
  chain->element_size = element_size;
  Aaligned = A = (char *) malloc(2 * PAGESZ + N * sizeof(element_t));
  assert(NULL != A);
  while ( 0 != (Aaligned - (char *)0) % PAGESZ )
    Aaligned++;
  B = (element_t *) Aaligned;
  for (i = 0; i < N; i++)
    B[i].next = 1+i;
  for (i = 0; i < N; i++) {
    uint64_t r, t;
    r = prng(&seed);
    r = r % N;  // should be okay for N << 2^64
    t = B[i].next;
    B[i].next = B[r].next;
    B[r].next = t;
  }

  sum = 0;
  for (i = 0; i < N; i++)
    sum += B[i].next;
  assert((N+1)*N/2 == sum);  // Euler's formula

  // Set up C[] such that "chasing pointers" through it visits
  // every element exactly once
  CK(posix_memalign((void**) &chain->buf, ALIGNMENT, access_size) == 0);
  file_size = (N+1) * element_size;
  CK2((fd = open(path, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR| S_IWUSR)) != -1, strerror(errno));
  CK2((ret = ftruncate(fd, file_size)) > -1, strerror(errno));
  CK2((M = (char*) mmap(NULL, file_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) != (void*) -1, strerror(errno));
  
  bzero(M, N*element_size); // force physical memory allocation
  
  chain->head = (element_t *) M;
  for (i = 0; i < N; i++) {
    element(chain, i)->next = UINT64_MAX;
  }
  p = 0;
  for (i = 0; i < N; i++) {
    p = element(chain, p)->next = B[i].next;
  }
  element(chain, p)->next = 0;
  for (i = 0; i <= N; i++) {
    assert(N >= element(chain, i)->next);
  }

  munmap(M, file_size);
  close(fd);
  free(A);
  free(chain);
}

// Opens chain
static chain_t* open_chain(const char* path, uint64_t seedin, uint64_t N, size_t element_size, size_t access_size, int access_method)
{
  int ret;
  uint64_t sum, p, i;
  element_t *B;
  char *A, *Aaligned, *M;
  uint64_t seed = seedin;
  chain_t* chain;
  size_t file_size;
  int oflag;

  file_size = (N+1) * element_size;
  oflag = O_RDWR;
  if (access_method == 'd') {
    oflag = oflag|O_DIRECT;
  }

  chain = (chain_t*) malloc(sizeof(chain_t));
  chain->N = N;
  chain->element_size = element_size;
  CK(posix_memalign((void**) &chain->buf, ALIGNMENT, access_size) == 0);

  CK2((chain->fd = open(path, oflag, S_IRUSR| S_IWUSR)) != -1, strerror(errno));
  if (access_method == 'm') {
    CK2((M = (char*) mmap(NULL, file_size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_POPULATE, chain->fd, 0)) != (void*) -1, strerror(errno));
  }
  chain->head = (element_t *) M;
  return chain;
}

static uint64_t trash_cache(uint64_t N)
{
  uint64_t T1, i, sum;
  char* A;
  char* ptr;
  element_t* B;
  ptr = A = (char *) malloc(2 * PAGESZ + N * sizeof(element_t));
  assert(NULL != A);
  while ( 0 != (A - (char *)0) % PAGESZ ) {
    A++;
    __asm__(""); /* prevent optimizer from removing loop */
  }
  B = (element_t *)A;

  /* trash the CPU cache */
  T1 = T() % 1000;
  for (i = 0; i < N; i++) {
    B[i].next = T1 * i + i % (T1+1);
    __asm__(""); /* prevent optimizer from removing loop */
  }
  sum = 0;
  for (i = 0; i < N; i++) {
    sum += B[i].next;
    __asm__(""); /* prevent optimizer from removing loop */
  }
  free(ptr);
  return sum;
}

// Measures access latency
int measure_latency(const char* path, uint64_t seed, size_t nelems, size_t element_size, size_t access_size, int access_method) 
{
  uint64_t j, i, T1, T2;
  uint64_t next;
  uint64_t sum;
  chain_t *chain;
  read_element_t read_element;

  assert(nelems < UINT64_MAX);

  switch (access_method) {
    case 'm':
      read_element = read_element_memory;
      break;
    case 'f':
    case 'd':
      read_element = read_element_fileio;
      break;
  }

  chain = open_chain(path, seed, nelems, element_size, access_size, access_method);

  trash_cache(nelems);

  /* chase the pointers */
  T1 = T();
  sum = 0;
  for (i = 0; 0 != (next = read_element(chain, i, access_size)); i = next) {
    sum += next;
  }
  T2 = T();

  assert((nelems+1)*nelems/2 == sum);  /* Euler's formula */
  uint64_t time_per_op_ns = ((T2-T1)*1000)/nelems;

  free(chain->buf);
  free(chain);

  return time_per_op_ns;
}

void usage(int argc, char** argv)
{
  fprintf(stderr, "Usage: %s <path> <seed> <nelems> <element_size> <access_size> <access_method>\n", argv[0]);
  fprintf(stderr, "\n");
  fprintf(stderr, "  access_method    c for create, m for mmap, f for fileio, d for directio\n", argv[0]);
}

int main(int argc, char** argv) {
  if (argc != 7) {
    usage(argc, argv);
    return;  
  }
  const char* path = argv[1];
  uint64_t seed = atoi(argv[2]);
  size_t nelems = atoi(argv[3]);
  size_t element_size = atoi(argv[4]);
  size_t access_size = atoi(argv[5]);
  char access_method = argv[6][0];
  switch(access_method) {
    case 'c':
      create_chain(path, seed, nelems, element_size, access_size);
      break;
    case 'm':
    case 'f':
    case 'd': {
      int lat = measure_latency(path, seed, nelems, element_size, access_size, access_method);
      printf("%d\n", lat);
      break;
    }
  }
}

