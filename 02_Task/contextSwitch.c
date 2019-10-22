#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sched.h>
#include <time.h>




int main(int argc, char ** argv) {

	if(argc != 4){
		printf("arguments are too less or too much");
		return 1;
	}


	cpu_set_t set;

	int parentCPU = atoi(argv[1]);
	int childCPU = atoi(argv[2]);
	int iteration = atoi(argv[3]);
	unsigned long * myCurrentTime =  (unsigned long*)malloc(sizeof(unsigned long) * 2);
	unsigned long * myTime =   (unsigned long*)malloc(sizeof(unsigned long) * 4 * iteration);;


	struct timespec start, end;
	//int result = 0;

	CPU_ZERO(&set);
	int pipe1[2];
	int pipe2[2];

	pipe(pipe1);
	pipe(pipe2);

	int ret = fork();

	if(ret == -1){
		perror("fork failed");
		exit(1);
	}

	if (ret == 0) {

		int result = 0;
		CPU_SET(childCPU, &set);
	        sched_setaffinity(getpid(), sizeof(set), &set);

		close(pipe2[1]); //close pipe2 output
		close(pipe1[0]); //close pipe1 input
		int a = 0;


		for(int i = 0; i < iteration; ++i){
			//write to parent through pipe1

			clock_gettime(CLOCK_REALTIME, &end);//end1
			write(pipe1[1], myCurrentTime, sizeof(unsigned long));
			clock_gettime(CLOCK_REALTIME, &start);//start2


			//read from parent through pipe2 -----------------------------
			read(pipe2[0], myCurrentTime, sizeof(unsigned long) * 2);

			myTime[a++] = end.tv_sec*1000000000 + end.tv_nsec;
			myTime[a++] = start.tv_sec*1000000000 + start.tv_nsec;
			myTime[a++] = myCurrentTime[0];
			myTime[a++] = myCurrentTime[1];
		}


		for(int i = 0; i < iteration*4; ++i){
			if((i % 4)==0){
				result += myTime[i] - myTime[i+2] + myTime[i+3] - myTime[i+1];
			}
		}
		printf("my RESULT !!!! %d\n",result/(iteration*2));

		exit(0);

	} else { // parent


	CPU_SET(parentCPU, &set);
 	sched_setaffinity(getpid(), sizeof(set), &set);
	close(pipe1[1]); //close pipe1 output
	close(pipe2[0]); //close pipe2 input

	unsigned long *nothing = (unsigned long*)malloc(sizeof(unsigned long));

	while(1){

		clock_gettime(CLOCK_REALTIME, &start);//Start1
		read(pipe1[0], nothing, sizeof(unsigned long));
		clock_gettime(CLOCK_REALTIME, &end);//end2

		myCurrentTime[0] = start.tv_sec*1000000000 + start.tv_nsec;
		myCurrentTime[1] = end.tv_sec*1000000000 + end.tv_nsec;

		write(pipe2[1], myCurrentTime, sizeof(unsigned long) * 2);
	}
}
return 0;
}
