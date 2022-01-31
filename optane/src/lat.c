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

#define MAX_NUM_CHAINS 16

typedef struct {
  uint64_t next;
  char padding[0];
} element_t;

typedef struct {
  uint64_t N;
  uint64_t element_size;
  element_t* head;
  int fd;
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
  //uint64_t next = e->next;
  size_t i;
  //uint64_t tmp; 
  int next, tmp;
  for (i = 0; i < access_size; i+=8) {
    __asm__("mov %1, %0;"
            : "=r" (tmp)
            : "m" (e->padding[i])
            : "memory");
    // __asm__("mov %0, %%eax;"
    //         : 
    //         : "m" (e->padding[i])
    //         : );
  }
  return next;
  // uint64_t i;
  // element_t *elem = element(chain, index);
  // buf_size = min(chain->element_size, buf_size);
  
  // memcpy(buf, &elem->padding[0], buf_size - sizeof(elem->next));
  // for (i = buf_size; i <= chain->element_size - buf_size; i += buf_size) {
  //   memcpy(buf, &elem->padding[i], buf_size);
  // }
}

static inline uint64_t read_element_fileio(chain_t* chain, off_t index, size_t access_size)
{
  // uint64_t i;
  // element_t *elem = element(chain, index);
  // buf_size = min(chain->element_size, buf_size);
  
  // memcpy(buf, &elem->padding[0], buf_size - sizeof(elem->next));
  // for (i = buf_size; i <= chain->element_size - buf_size; i += buf_size) {
  //   memcpy(buf, &elem->padding[i], buf_size);
  // }
}

// Allocates chain
// 
// mmap_or_fileio == 0: Allocate chain for use with mmap
// mmap_or_fileio == 1: Allocate chain for use with file I/O
static chain_t* alloc_chain(const char* path, uint64_t seedin, uint64_t N, uint64_t element_size, int mmap_or_fileio)
{
  int fd, ret;
  uint64_t sum, p, i;
  element_t *B;
  char *A, *Aaligned, *M;
  uint64_t seed = seedin;
  chain_t* chain;
  size_t file_size;

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
  if (mmap_or_fileio == 0) {
    file_size = 2 * PAGESZ + (1+N) * element_size;
  }
  if (mmap_or_fileio == 1) {
    file_size = N * element_size;
  }
    
  CK2((fd = open(path, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR| S_IWUSR)) != -1, strerror(errno));
  CK2((ret = ftruncate(fd, element_size * N)) > -1, strerror(errno));
  CK2((M = (char*) mmap(NULL, file_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0)) != NULL, strerror(errno));

  if (mmap_or_fileio == 0) {
    // Ensure first element of the chain is aligned at page boundary.
    while ( 0 != (M - (char *)0) % PAGESZ )
      M++;
  }
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
  free(A);
  if (mmap_or_fileio == 0) {
    close(fd);
  }
  if (mmap_or_fileio == 1) {
    munmap(chain->head, file_size);
  }
  return chain;
}

static chain_t* alloc_chain_mmap(const char* path, uint64_t seedin, uint64_t N, uint64_t element_size) 
{
  return alloc_chain(path, seedin, N, element_size, /*mmap_or_file=*/0);
}

static chain_t* alloc_chain_fileio(const char* path, uint64_t seedin, uint64_t N, uint64_t element_size) 
{
  return alloc_chain(path, seedin, N, element_size, /*mmap_or_file=*/1);
}


uint64_t trash_cache(uint64_t N)
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


int measure_latency(const char* path, uint64_t seedin, size_t nchains, size_t nelems, size_t element_size, size_t access_size) 
{
  uint64_t seed, j, i, T1, T2;
  uint64_t sumv[MAX_NUM_CHAINS];
  uint64_t nextp[MAX_NUM_CHAINS];
  chain_t *C[MAX_NUM_CHAINS];
  char *buf;
  uint64_t buf_size = 16384;

  assert(nelems < UINT64_MAX);
  assert(nchains < MAX_NUM_CHAINS);

  //DBG_LOG(INFO, "measuring latency: nchains %d, nelems %zu, elem_sz %d, access_sz %d, from_node_id %d, to_node_id %d\n", nchains, nelems, element_size, access_size, from_node_id, to_node_id);

  for (j=0; j < nchains; j++) {
    seed = seedin + j*j;
    C[j] = alloc_chain_mmap(path, seed, nelems, element_size);
  }

  trash_cache(nelems);

  buf = (char*) malloc(buf_size);
  assert(buf != NULL);

  read_element_t read_element = read_element_memory;

  /* chase the pointers */
  if (nchains == 1) {
    T1 = T();
    sumv[0] = 0;
    // for (i = 0; 0 != element(C[0], i)->next; i = element(C[0], i)->next) {
    //   sumv[0] += element(C[0], i)->next;
    //   if (access_size > element_size) {
    //     read_element(C[0], i, buf, buf_size);
    //   }
    // }
    uint64_t next;
    for (i = 0; 0 != (next = read_element(C[0], i, access_size)); i = next) {
      sumv[0] += next;
    }
    T2 = T();
  } else {
    T1 = T();
    for (j=0; j < nchains; j++) {
      sumv[j] = 0;
      nextp[j] = 0;
    }
    // for (; 0 != element(C[0], nextp[0])->next; ) {
    //   for (j=0; j < nchains; j++) {
    //     sumv[j] += element(C[j], nextp[j])->next;
    //     if (access_size > element_size) {
    //       read_element(C[j], nextp[j], buf, buf_size);
    //     }
    //     nextp[j] = element(C[j], nextp[j])->next;
    //   }
    // }
    T2 = T();
  }
  assert((nelems+1)*nelems/2 == sumv[0]);  /* Euler's formula */
  uint64_t time_per_op_ns = ((T2-T1)*1000)/nelems;

  //DBG_LOG(INFO, "measuring latency: latency is %lu ns\n", time_per_op_ns);

  for (j=0; j < nchains; j++) {
    free(C[j]);
  }
  free(buf);

  return time_per_op_ns;
}

int main(int argc, char** argv) {
  const char* path = "/tmp/test";
  uint64_t seedin = 1;
  size_t nchains = 1;
  size_t nelems = 1000000;
  size_t element_size = 64;
  size_t access_size = 8;
  int lat = measure_latency(path, seedin, nchains, nelems, element_size, access_size);
  printf("%d\n", lat);
}

