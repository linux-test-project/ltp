#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	pid_t pid, group_id;

	if (argc != 2) {
		fprintf(stderr,"Usage: %s pid\n",argv[0]);
		exit(-1);
	}
	pid = (pid_t) atol(argv[1]);
	printf("pid = %d \n",pid); 
	if ((group_id = getpgid(pid)) < 0) {
		perror("getpgid");
		exit(1);
	}
	printf("group ID = %d\n",group_id);
	exit(0);
}
