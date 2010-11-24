#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int HPS;
char hugetlbfsdir[256];
#ifndef UTILS
#include "utils.h"
#endif
#define errmsg(x) fprintf(stderr, x), exit(1)

void write_hugepage(char *addr, int size, char *avoid)
{
	int i, j;
	for (i = 0; i < size; i++) {
		if (addr == avoid)
			continue;
		for (j = 0; j < HPS; j++) {
			*(addr + i * HPS + j) = (char)('a' + ((i + j) % 26));
		}
	}
}

/* return -1 if buffer content differs from the expected ones */
int read_hugepage(char *addr, int size, char *avoid)
{
	int i, j;
	int ret = 0;

	for (i = 0; i < size; i++) {
		if (addr == avoid)
			continue;
		for (j = 0; j < HPS; j++) {
			if (*(addr + i * HPS + j) != (char)('a' + ((i + j) % 26))) {
				printf("Mismatch at %d\n", i + j);
				ret = -1;
				break;
			}
		}
	}
	return ret;
}

int hugetlbfs_root(char *dir)
{
	int found = 0;
	FILE *f = fopen("/proc/mounts", "r");
	if (!f) err("open /proc/mounts");
	char *line = NULL;
	size_t linelen = 0;
	char dummy[100];
	while (getline(&line, &linelen, f) > 0) {
		if (sscanf(line, "none %s hugetlbfs %[^ ]",
			   dir, dummy) >= 2) {
			found = 1;
			break;
		}
	}
	free(line);
	fclose(f);
	if (!found)
		printf("cannot find hugetlbfs directory in /proc/mounts\n");
	return found;
}

/* Assume there is only one types of hugepage size for now. */
int gethugepagesize(void)
{
	int hpagesize = 0;
	struct dirent *dent;
	DIR *dir;
	dir = opendir("/sys/kernel/mm/hugepages");
	if (!dir) err("open /sys/kernel/mm/hugepages");
	while ((dent = readdir(dir)) != NULL)
		if (sscanf(dent->d_name, "hugepages-%dkB", &hpagesize) >= 1)
			break;
	closedir(dir);
	return hpagesize * 1024;
}

void *alloc_shm_hugepage(int *key, int size)
{
	void *addr;
	int shmid;
	if ((shmid = shmget(*key, size,
			    SHM_HUGETLB | IPC_CREAT | SHM_R | SHM_W)) < 0) {
		perror("shmget");
		return NULL;
	}
	addr = shmat(shmid, (void *)0x0UL, 0);
	if (addr == (char *)-1) {
		perror("Shared memory attach failure");
		shmctl(shmid, IPC_RMID, NULL);
		return NULL;
	}
	*key = shmid;
	return addr;
}

void *alloc_anonymous_hugepage(int size, int private)
{
	void *addr;
	int mapflag = MAP_ANONYMOUS | 0x40000; /* MAP_HUGETLB */
	if (private)
		mapflag |= MAP_PRIVATE;
	else
		mapflag |= MAP_SHARED;
	if ((addr = mmap(0, size,
			 PROT_READ|PROT_WRITE, mapflag, -1, 0)) == MAP_FAILED) {
		perror("mmap");
		return NULL;
	}
	return addr;
}

void *alloc_filebacked_hugepage(char *filepath, int size, int private, int *fd)
{
	int mapflag = MAP_SHARED;
	void *addr;
	if (private)
		mapflag = MAP_PRIVATE;
	if ((*fd = open(filepath, O_CREAT | O_RDWR, 0777)) < 0) {
		perror("open");
		return NULL;
	}
	if ((addr = mmap(0, size,
			 PROT_READ|PROT_WRITE, mapflag, *fd, 0)) == MAP_FAILED) {
		perror("mmap");
		unlink(filepath);
		return NULL;
	}
	return addr;
}

int free_shm_hugepage(int key, void *addr)
{
	if (shmdt((const void *)addr) != 0) {
		perror("Detach failure");
		shmctl(key, IPC_RMID, NULL);
		return -1;
	}
	shmctl(key, IPC_RMID, NULL);
	return 0;
}

int free_anonymous_hugepage(void *addr, int size)
{
	int ret = 0;
	if (munmap(addr, size)) {
		perror("munmap");
		ret = -1;
	}
	return ret;
}

int free_filebacked_hugepage(void *addr, int size, int fd, char *filepath)
{
	int ret = 0;
	if (munmap(addr, size)) {
		perror("munmap");
		ret = -1;
	}
	if (close(fd)) {
		perror("close");
		ret = -1;
	}
	if (filepath) {
		if (unlink(filepath)) {
			perror("unlink");
			ret = -1;
		}
	} else {
		fprintf(stderr, "Filepath not specified.\n");
		ret = -1;
	}
	return ret;
}
