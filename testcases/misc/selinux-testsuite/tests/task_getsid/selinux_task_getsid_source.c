#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	pid_t pid, session_id;

	if (argc != 2) {
		fprintf(stderr,"Usage: %s pid\n",argv[0]);
		exit(-1);
	}
	pid = (pid_t) atol(argv[1]);
	printf("pid = %d \n",pid); 
	if ((session_id = getsid(pid)) < 0) {
		perror("getsid");
		exit(1);
	}
	printf("session ID = %d\n",session_id);
	exit(0);
}
