#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
	pid_t pid, group_id;
	int status;
	
	pid = getpid();
	if ((group_id = getpgid(pid)) < 0) {
		perror("getpgid");
		exit(-1);
	}
	printf("Group ID = %d\n",group_id);
	if (setpgid(pid,pid) < 0) {
		perror("setpgid");
		exit(1);
	}
	if ((group_id = getpgid(pid)) < 0) {
		perror("getpgid");
		exit(-1);
	}
	printf("Group ID = %d\n",group_id);
	printf("pid = %d\n",pid);
	exit(0);
}
