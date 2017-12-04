
// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <stdint.h>
#include "timing.h"
#include <inttypes.h>
#include <time.h>

// includes CUDA
#include <cuda_runtime.h>

// includes, project
#include <helper_cuda.h>
#include <helper_functions.h> // helper functions for SDK examples
//const int N = 16;
//const int blocksize = 16;
/*
__global__
void hello(char *a, int *b)
{
  a[threadIdx.x] += b[threadIdx.x];
}*/

# define my_sizeof(type) ((char *)(&type+1)-(char*)(&type))
//static int crackFlag = 0;

static uint8_t alphanumerical[36] = {
    97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,49,50,51,52,53,54,55,56,57,58
};
//"abcdefghijklmnopqrstuvwxyz"
//"0123456789"
static uint8_t password[5] = {
    98,118,51,55,113
  //58
};
//bv37q

//http://www.cse.yorku.ca/~oz/hash.html
//sdbm hash function
uint64_t inline hash(uint8_t str[], int len)
{
    uint64_t hash = 0;
    int c;
    int i = 0;

    for(i = 0; i< len; i++){
        c = str[i];
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}
__device__
uint64_t inline cudahash(uint8_t str[], int len)
{
    uint64_t hash = 0;
    int c;
    int i = 0;

    for(i = 0; i< len; i++){
        c = str[i];
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}
__device__
void permutation(uint8_t prefix[], int length, int index, uint64_t *cudahashPassword, int constLength, int output[], uint8_t *cudaalphanumerical){
  //  if (crackFlag == 1) return; // have been cracked password
  if(length == 0){
    //if(prefix[0]==98 && prefix[1] == 104){
    //printf("%d %d %d %d %d\n",prefix[0],prefix[1],prefix[2],prefix[3],prefix[4]);
    //}
  if(cudahash(prefix,constLength)==cudahashPassword[0]){
    printf("======Successfully Cracked!!!======\n");
/*    const int lastone = 5;
    output[lastone] = constLength;
    for (int n = 0; n<constLength; n++){
      output[n] = prefix[n];
    }*/
    printf("The cracked password in ASCII is: ");
        for(int j = 0; j < constLength; j++){
            printf("%d ", prefix[j]);
        }
        printf("\n");
//        crackFlag = 1;
   }
   return;
  }else{
    //for (int i = 0; i < sizeof(cudaalphanumerical)/sizeof(int8_t); i++) {
    for (int i = 0; i < 36; i++) {
      prefix[index] = cudaalphanumerical[i];
      permutation(prefix, length-1,index+1, cudahashPassword, constLength, output, cudaalphanumerical);
    }

  }
 }

__global__
void cudaPermutation(int length, int index, uint64_t hashPassword[], int constLength, int output[], uint8_t *cudaalphanumerical){
  uint8_t prefix[5] = {0};
  prefix[0] = cudaalphanumerical[threadIdx.x];
  //for(int i =0; i<36;i++)
  //printf("%d ", cudaalphanumerical[i]);
  permutation(prefix,length-1,1,hashPassword,length,output,cudaalphanumerical);
}

int main()
{
  int maxLen = 5;
  int *output = new int[maxLen+1];
  uint64_t hashPassword = ::hash(password,my_sizeof(password)/my_sizeof(password[0]));
  //printf("CPU hash: %" PRIu64 "\n", hashPassword);
  const size_t poolsize = 36*sizeof(uint8_t);
  const size_t hashPwdsize = sizeof(uint64_t);
  uint8_t *cudaalphanumerical;
  uint64_t *cudahashPassword;
  cudaMalloc( &cudaalphanumerical, poolsize );
  cudaMalloc( &cudahashPassword, hashPwdsize );
  cudaMemcpy( cudaalphanumerical, alphanumerical, poolsize, cudaMemcpyHostToDevice );
  cudaMemcpy( cudahashPassword, &hashPassword, hashPwdsize, cudaMemcpyHostToDevice );
  printf("======Start cracking using Cuda======\n");
  double minCuda = 1e30;
  struct timespec start, stop;
  for(unsigned int i = 0 ; i < 3; i ++){
     //reset_and_start_timer();
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
     for(int length=1;length<=maxLen;length++){
     //int length = 5;
      cudaPermutation<<<1,36>>>(length,0,cudahashPassword,length,output,cudaalphanumerical);
      cudaDeviceSynchronize();
     }

     //double dt = get_elapsed_mcycles();
     clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
     double dt = (stop.tv_sec - start.tv_sec) * 1e6 + (stop.tv_nsec - start.tv_nsec) / 1e3;    // in microseconds
     minCuda = std::min(minCuda, dt);
   }
  printf("@time of minCuda run:\t\t\t[%.3f] microseconds\n", minCuda);
  return 0;
}