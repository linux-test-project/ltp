#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>

int clone(int (*fn)(void *), void *child_stack, int flags, void *arg);

int system(const char *string);

int writer(void *data)
{
	int fin, ret;

	fin = open("shared.so", O_RDWR);
	if (fin == -1)
		ret = 0;
	else
		ret = 1;

	sleep(3);

	if (fin != -1)
		close(fin);
	return ret;
}

#define CMD "./example"
#define LONGCMD "./example ab"
int executor(void *data)
{
	int ret;
	int hold = (int)data;

	ret = system(LONGCMD);
	return ret;
}

int main(int argc, char *argv[])
{
	char buf1[500], buf2[500];
	int retval1, retval2, ret;
	int pid1, pid2, fin;

	printf("Testing execute after write\n");
	pid1 = clone(writer, buf1, 0, (void *)1);
	ret = system(CMD);
	if (ret != 0)
		printf("GOOD test: unable to dlopen shared.so.\n");
	else
		printf("BAD test: able to dlopen shared.so (%d)\n", ret);

	printf("Testing write after execute\n");
	pid1 = clone(executor, buf2, 0, (void *)0);
	fin = open("shared.so", O_RDWR);
	if (fin == -1)
		printf("GOOD test: could not open shared.so for writing.\n");
	else {
		printf("BAD test: could open shared.so for writing.\n");
		close(fin);
	}
}
