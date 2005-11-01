#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <wait.h>

int clone(int (*fn)(void *), void *child_stack, int flags, void *arg);

int writer(void *data)
{
	int fd;
	FILE *flog;

	flog = fopen("writelog", "w");
	if (!flog) {
		printf("writer: failed to open log...\n");
		fclose(flog);
		fclose(flog);
		exit(4);
	}

	sleep (1);

	fprintf(flog, "writer: first open(write) (want to fail)\n");
	fd = open("shared.so", O_RDWR);
	if (fd != -1) {
		fprintf(flog, "writer: succeeded first open(write) (BAD)\n");
		fclose(flog);
		exit(2);
	}
	fprintf(flog, "writer: failed first open(write) (GOOD)\n");

	sleep(2);

	fprintf(flog, "writer: second open(write) (want to succeed)\n");
	fd = open("shared.so", O_RDWR);
	if (fd == -1) {
		fprintf(flog, "writer: failed second write (BAD) (%d)\n");
		fclose(flog);
		exit(2);
	}

	sleep(2);
	fprintf(flog, "writer closing fd\n");
	close(fd);
	fprintf(flog, "writer exiting (GOOD)\n");
	fclose(flog);
	return 0;
}

int executor(void *data)
{
	void *handle;
	FILE *flog;

	flog = fopen("execlog", "w");
	if (!flog) {
		printf("executor: failed to open log...\n");
		fclose(flog);
		exit(4);
	}

	fprintf(flog, "executor: dlopening (want to succeed)\n");
	handle = dlopen("./shared.so", RTLD_NOW);
	if (!handle) {
		fprintf(flog, "executor: failed first dlopen (BAD)\n");
		fputs (dlerror(), stderr);
		fclose(flog);
		exit(3);
	}

	fprintf(flog, "executor: succeeded first dlopen (GOOD)\n");

	sleep(2);
	fprintf(flog, "executor: dlclose()ing\n");
	dlclose(handle);
	sleep(2);

	fprintf(flog, "executor: dlopening (want to fail)\n");
	handle = dlopen("./shared.so", RTLD_NOW);
	if (handle) {
		fprintf(flog, "executor: succeeded second dlopen (BAD)\n");
		dlclose(handle);
		fclose(flog);
		exit(3);
	}
	fprintf(flog, "executor exiting (GOOD)\n");
	fclose(flog);
	return 0;
}

#define STACKSIZE 1024*64

int main(int argc, char *argv[])
{
	char *buf1, *buf2;
	int pid1, pid2, ret;
	int status;

	buf1 = malloc(STACKSIZE);
	buf2 = malloc(STACKSIZE);
	if (!buf1 || !buf2) {
		printf("error allocating memory\n");
		exit(1);
	}
	pid1 = clone(writer, buf1+STACKSIZE, CLONE_FILES, (void *)0);
	if (pid1 == -1) {
		printf("Error clone(writer)\n");
		exit(1);
	}
	pid2 = clone(executor, buf2+STACKSIZE, CLONE_FILES, (void *)0);
	if (pid2 == -1) {
		printf("Error clone(executor)\n");
		exit(1);
	}
	printf("both clones successfull (%d, %d)\n", pid1, pid2);
	ret = waitpid(pid1, &status, 0);
	printf("waitpid %d returned %d (status %d)\n", pid1, ret, status);
	ret = waitpid(pid2, &status, 0);
	printf("waitpid %d returned %d (status %d)\n", pid2, ret, status);

	/* Serge: don't you know how to use waitpid? */
	sleep(10);

	return 0;
}
