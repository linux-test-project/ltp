#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
	int pid;

	if ((pid = fork()) < 0) {
		perror("fork"); 
		exit(1);
	}
	fprintf(stderr,"%d\n",pid);
	exit(0);
}
