/*
Project 3: Part 5
Group 2
*/

#include <iostream>
#include <stdio.h>
#include <algorithm>
#include "timing.h"
#include <string.h>
#include <pthread.h>
#include <time.h>
using namespace std;
// User defined sizeof macro
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
static uint64_t inline hash(uint8_t str[], int len)
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

static void inline permutation(uint8_t prefix[], int length, int index, uint64_t hashPassword, int constLength, int output[]){
  //if (crackFlag == 1) return; // have been cracked password
  if(length == 0){
  if(::hash(prefix,constLength)==hashPassword){
    printf("======Successfully Craked!!!======\n");
    const int lastone = 5;
    output[lastone] = constLength;
    for (int n = 0; n<constLength; n++){
      output[n] = prefix[n];
    }
    printf("The cracked password in ASCII is: ");
        for(int j = 0; j < output[5]; j++){
            printf("%d ", output[j]);
        }
        printf("\n");
        //crackFlag = 1;
   }
   return;
  }else{
    /*
    foreach(i=0...sizeof(alphanumerical)/sizeof(int8)){
      prefix[index] = alphanumerical[i];
      permutation(prefix, length-1,index+1, hashPassword, constLength);
    }
    */
    
   for (int i = 0; i < sizeof(alphanumerical)/sizeof(int8_t); i++) {
    prefix[index] = alphanumerical[i];
    permutation(prefix, length-1,index+1, hashPassword, constLength, output);
   }
   
  }
 }

void add(int n, float *x, float *y)
{
  for (int i = 0; i < n; i++)
      y[i] = x[i] + y[i];
}

void pwdcraker_serial( int maxLen,  int output[])
{
  //printf("Length of the password: %d\n", sizeof(password)/sizeof(int8_t));
  //printf("Length of alphanumerical pool: %d\n", sizeof(alphanumerical)/sizeof(int8_t));
  //crackFlag = 0;
  int i = 0;
  uint64_t hashPassword = ::hash(password,my_sizeof(password)/my_sizeof(password[0]));
  printf("======Start cracking using Serial====== \n");
  for(int length=1;length<=5;length++){
    uint8_t temp[5] = {0};
    permutation(temp,length,0,hashPassword,length, output);
  }
}

void *pwdcraker_pthread(void *pArg){
  //crackFlag = 0;
  int *inputs = (int*)pArg;
  uint64_t hashPassword = ::hash(password,my_sizeof(password)/my_sizeof(password[0]));
  int output[5];
  uint8_t temp[5] = {0};
  for(int length=1;length<=inputs[0];length++){
    for(int m = inputs[1];m<=inputs[2];m++){
      temp[0] = alphanumerical[m-1];
      permutation(temp,length-1,1,hashPassword,length,output);
    } 
  }
}





int main(int argc, char *argv[]) {
    int maxLen = 5;
    // We assume that the password is not case-sensitive, and can be only set from the a-z 0-9.
    //char *password = "bv37q";
    int *output = new int[maxLen+1];
    struct timespec start, stop;
/*
    double minSerial = 1e30;
    for(unsigned int i = 0 ; i < 3; i ++){
        reset_and_start_timer();
        pwdcraker_serial(maxLen, (int *)output);
        double dt = get_elapsed_mcycles();
        printf("\n");
        printf("@time of serial run:\t\t\t[%.3f] million cycles\n", dt);
        minSerial = std::min(minSerial, dt); 
    }
    printf("[root calculate serial]:\t\t[%.3f] million cycles\n", minSerial);
*/

    printf("======Start cracking using Pthread (balanced distributed)======\n");
    double minPthread = 1e30;
    int threadNum = atoi(argv[1]);
    pthread_t thread_id[threadNum];
    int inputs[3*threadNum];
    int range = (int) 36/threadNum;
    for(unsigned int i = 0 ; i < 3; i ++){
        //reset_and_start_timer();
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for(int i=0; i < threadNum; i++){
      inputs[i*3] = maxLen; //thread ID, starting from 0
      inputs[i*3+1] = 1+range*i;//starting alpbet
      inputs[i*3+2] = min(range*(i+1),36);//ending alpbet
      pthread_create( &thread_id[i], NULL, &pwdcraker_pthread, &inputs[i*3]);
    }

    for(int j=0; j < threadNum; j++){
      pthread_join( thread_id[j], NULL);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
    double dt = (stop.tv_sec - start.tv_sec) * 1e6 + (stop.tv_nsec - start.tv_nsec) / 1e3;    // in microseconds
    //printf("@time of pthread run:\t\t\t[%.3f] million cycles\n", dt);
        minPthread = std::min(minPthread, dt); 
    }
    printf("@time of pthread run:\t\t[%.3f] microseconds\n", minPthread);
    
  //printf("\t\t\t\t(%.2fx speedup from pthread \n", minSerial/minPthread);
    return 0;
}

