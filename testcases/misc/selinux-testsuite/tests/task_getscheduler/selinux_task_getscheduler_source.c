#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

int main(int argc, char *argv[])
{
	pid_t pid;
	int policy;

	if (argc != 2) {
		fprintf(stderr,"Usage: %s pid\n",argv[0]);
		exit(-1);
	}
	pid = (pid_t) atol(argv[1]);
	printf("pid = %d \n",pid); 
	if ((policy = sched_getscheduler(pid)) < 0) {
		perror("sched_getscheduler");
		exit(1);
	}
	printf("policy = %d\n",policy);
	exit(0);
}
