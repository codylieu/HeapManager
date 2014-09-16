#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include "dmm.h"

#ifdef HAVE_DRAND48
#define RAND() (drand48())
#define SEED(x) (srand48((x)))
#else
#define RAND() ((double)random()/RAND_MAX)
#define SEED(x) (srandom((x)))
#endif

#define RSEED 0 // 1 for non-deterministic seeding
#define MAX_SIZE 50
#define LPCNT 1000000
#define TIMEOUT 180 //seconds

//Test Case 6: check for O(1) implementation - call with varying number of blocks (nblocks) in argument
//Measures execution time to allocate/free memory with (nblocks/2) free blocks
//First it fills heap with specified blocks of random size
//Then it frees every other block
//Finally, it allocates/free memory with 50% probability for each and measures the time for the same
// return 0 if fails and 1 if passes
int test_case6(int nblock, double *t){
	clock_t begin, end;
	void *ptr[nblock];
	int ind_pool[LPCNT];
	int size_pool[LPCNT];
	int size, i, j;

	//fill the heap by allocating #NBLOCKS of random size in [1,MAX_SIZE]
	for (i = 0; i < nblock; i++) {
		size = (int) (RAND() * MAX_SIZE);
		if (size < 1)
			size = 1;
		ptr[i] = dmalloc(size);
		if (!ptr[i]) {
			printf("TC6: Error in Init Alloc\n");
			return 0; 
		}
	}

	//free every other block
	for (i = 0; i < nblock; i = i + 2) {
		if (ptr[i])
			dfree(ptr[i]);
		ptr[i] = NULL;
	}

	//generate LPCNT indexes to allocate/free in range [0,NBLOCK-1] and random size for allocation
	for (i = 0; i < LPCNT; i++) {
		ind_pool[i] = (int) (RAND() * nblock - 1);
		size_pool[i] = (int) (RAND() * MAX_SIZE);
		if (size_pool[i] < 1)
			size_pool[i] = 1;
	}

	//if the specified block is allocated free it otherwise allocate it with random size
	begin = clock();
	for (i = 0; i < LPCNT; i++) {
		j = ind_pool[i];
		if (ptr[j]) {
			dfree(ptr[j]);
			ptr[j] = NULL;
		} else 
			ptr[j] = dmalloc(size_pool[i]);
	}
	end = clock();
	*t = (double) (end - begin) / CLOCKS_PER_SEC;

	return 1;
}

void sigalrm_handler(int sig){
	printf("Code is taking more than %d seconds. Aborting!\n",TIMEOUT);
	//Format: Success ExecTime
	fprintf(stderr,"0 0\n");
	exit(1);
}


int main(int argc, char** argv) {

	int nblock;
	double t;
	
	if(argc!=2){
		printf("Arguments: #nblocks [10,100,1000,10000]\n");
		exit(1);
	}else
		nblock = atoi(argv[1]);
	
	if (RSEED)
		SEED(time(NULL));

	signal(SIGALRM,sigalrm_handler);
	alarm(TIMEOUT);
	int rc = test_case6(nblock,&t);
	alarm(0);
	
	//Format: TC6: Success NBlock ExecTime
	//printf("TC6 %d %d %f\n",rc,nblock,t);
	//Format: Success ExecTime
	fprintf(stderr, "TC6: Success, NBlock, ExecTime\n");
	//Format: Success ExecTime
	fprintf(stderr," %d %d %f\n",rc,nblock,t);
	
	return 0;
}