/******************************************************************************/
/*                                                                            */
/* Copyright (c) 2009 FUJITSU LIMITED                                         */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/* Author: Miao Xie <miaox@cn.fujitsu.com>                                    */
/*                                                                            */
/******************************************************************************/

#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <err.h>
#include <limits.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <pthread.h>

char *TCID = "cpuset_memory_test";
int TST_TOTAL = 1;

#if HAVE_LINUX_MEMPOLICY_H

#include "../cpuset_lib/cpuset.h"

static int fd;

static int opt_mmap_anon;
static int opt_mmap_file;
static int opt_mmap_lock1;
static int opt_mmap_lock2;
static int opt_shm;
static int opt_hugepage;
static int opt_check; /* check node when munmap memory (only for mmap_anon()) */
static int opt_thread;

static int key_id;			/* used with opt_shm */
static unsigned long memsize;

#define FILE_HUGEPAGE	"/hugetlb/hugepagefile"

#define MMAP_ANON	(SCHAR_MAX + 1)
#define MMAP_FILE	(SCHAR_MAX + 2)
#define MMAP_LOCK1	(SCHAR_MAX + 3)
#define MMAP_LOCK2	(SCHAR_MAX + 4)
#define SHM		(SCHAR_MAX + 5)
#define HUGEPAGE	(SCHAR_MAX + 6)
#define CHECK		(SCHAR_MAX + 7)
#define THREAD		(SCHAR_MAX + 8)

const struct option long_opts[] = {
	{"mmap-anon", 0, NULL, MMAP_ANON},
	{"mmap-file", 0, NULL, MMAP_FILE},
	{"mmap-lock1", 0, NULL, MMAP_LOCK1},
	{"mmap-lock2", 0, NULL, MMAP_LOCK2},
	{"shm", 0, NULL, SHM},
	{"hugepage", 0, NULL, HUGEPAGE},
	{"check", 0, NULL, CHECK},
	{"thread", 0, NULL, THREAD},
	{"size", 1, NULL, 's'},
	{"key", 1, NULL, 'k'},
	{NULL, 0, NULL, 0},
};

/*
 * process_options: read options from user input
 */
void process_options(int argc, char *argv[])
{
	int c;
	char *end;

	while (1) {
		c = getopt_long(argc, argv, "s:k:", long_opts, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 's':
			memsize = strtoul(optarg, &end, 10);
			if (*end != '\0')
				errx(1, "wrong -s argument!");
			break;
		case 'k':
			key_id = atoi(optarg);
			break;
		case MMAP_ANON:
			opt_mmap_anon = 1;
			break;
		case MMAP_FILE:
			opt_mmap_file = 1;
			break;
		case MMAP_LOCK1:
			opt_mmap_lock1 = 1;
			break;
		case MMAP_LOCK2:
			opt_mmap_lock2 = 1;
			break;
		case SHM:
			opt_shm = 1;
			break;
		case HUGEPAGE:
			opt_hugepage = 1;
			break;
		case CHECK:
			opt_check = 1;
			break;
		case THREAD:
			opt_thread = 1;
			break;
		default:
			errx(1, "unknown option!\n");
			break;
		}
	}

	if (!memsize)
		memsize = sysconf(_SC_PAGESIZE);
}

/*
 * touch_memory: force allocating phy memory
 */
void touch_memory_and_echo_node(char *p, int size)
{
	int i;
	int pagesize = sysconf(_SC_PAGESIZE);

	for (i = 0; i < size; i += pagesize)
		p[i] = 0xef;

	printf("%d\n", cpuset_addr2node(p));
}

void mmap_anon(int flag_allocated)
{
	static char *p;

	if (!flag_allocated) {
		p = mmap(NULL, memsize, PROT_WRITE | PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
		if (p == MAP_FAILED)
			err(1, "mmap(anonymous) failed");
		touch_memory_and_echo_node(p, memsize);
	} else {
		if (opt_check)
			touch_memory_and_echo_node(p, memsize);
		if (munmap(p, memsize) == -1)
			err(1, "munmap(anonymous) failed");
	}
}

void mmap_file(int flag_allocated)
{
	static char *p;
	static int fd_hugepage;
	int fd_tmp;

	if (!flag_allocated) {
		if (opt_hugepage) {
			fd_hugepage = open(FILE_HUGEPAGE,
					   O_CREAT | O_RDWR, 0755);
			if (fd_hugepage < 0)
				err(1, "open hugepage file failed");
			fd_tmp = fd_hugepage;
		} else
			fd_tmp = fd;

		p = mmap(NULL, memsize, PROT_WRITE | PROT_READ,
			 MAP_SHARED, fd_tmp, 0);
		if (p == MAP_FAILED) {
			if (opt_hugepage)
				unlink(FILE_HUGEPAGE);
			err(1, "mmap(file) failed");
		}
		touch_memory_and_echo_node(p, memsize);
	} else {
		if (munmap(p, memsize) == -1)
			err(1, "munmap(file) failed");

		if (opt_hugepage) {
			close(fd_hugepage);
			unlink(FILE_HUGEPAGE);
		}
	}
}

void mmap_lock1(int flag_allocated)
{
	static char *p;

	if (!flag_allocated) {
		p = mmap(NULL, memsize, PROT_WRITE | PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS | MAP_LOCKED, 0, 0);
		if (p == MAP_FAILED)
			err(1, "mmap(lock) failed");
		touch_memory_and_echo_node(p, memsize);
	} else {
		if (munmap(p, memsize) == -1)
			err(1, "munmap(lock) failed");
	}
}

void mmap_lock2(int flag_allocated)
{
	static char *p;

	if (!flag_allocated) {
		p = mmap(NULL, memsize, PROT_WRITE | PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
		if (p == MAP_FAILED)
			err(1, "mmap failed");

		if (mlock(p, memsize))
			err(1, "mlock failed");
		touch_memory_and_echo_node(p, memsize);
	} else {
		if (munmap(p, memsize) == -1)
			err(1, "munmap failed");
	}
}

void shm(int flag_allocated)
{
	static char *p;
	static int shmid;
	unsigned long flag;

	key_t key;

	if (!flag_allocated) {
		flag = IPC_CREAT | SHM_R | SHM_W;
		if (opt_hugepage)
			flag |= SHM_HUGETLB;

		key = ftok("/dev/null", key_id);
		if (key == -1)
			err(1, "ftok() failed\n");

		shmid = shmget(key, memsize, flag);
		if (shmid == -1)
			err(1, "shmget() failed\n");
		shmctl(shmid, IPC_RMID, NULL);

		shmid = shmget(key, memsize, flag);
		if (shmid == -1)
			err(1, "shmget() failed\n");

		p = shmat(shmid, NULL, 0);
		if (p == (void *)-1) {
			shmctl(shmid, IPC_RMID, NULL);
			err(1, "shmat() failed\n");
		}
		touch_memory_and_echo_node(p, memsize);
	} else {
		if (shmdt(p) == -1)
			err(1, "shmdt() failed\n");
		if (shmctl(shmid, IPC_RMID, NULL) == -1)
			err(1, "shmctl() failed\n");
	}
}

void *thread2_routine(void __attribute__ ((unused)) * arg)
{
	sigset_t set;
	sigset_t waitset;
	int flag_allocated;

	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGINT);
	pthread_sigmask(SIG_BLOCK, &set, NULL);

	sigemptyset(&waitset);
	sigaddset(&waitset, SIGUSR2);
	pthread_sigmask(SIG_BLOCK, &waitset, NULL);

	flag_allocated = 0;

	for (;;) {
		if (sigwaitinfo(&waitset, NULL) < 0)
			err(1, "sigwaitinfo() in thread2 failed");

		mmap_anon(flag_allocated);
		flag_allocated = !flag_allocated;
	}

	return NULL;
}

/*
 * When we receive SIGUSR1, we allocate some memory according
 * to the user intput when the process started.
 * When we receive SIGUSR1 again, we will free all the allocated
 * memory.
 * Similiar for --thread option but SIGUSR2 signal is used
 * to control thread2 behaviour.
 */
int main(int argc, char *argv[])
{
	sigset_t waitset;
	int signo;
	int flag_allocated;

	pthread_t thread2;

	fd = open("/dev/zero", O_RDWR);
	if (fd < 0)
		err(1, "open /dev/zero failed");

	process_options(argc, argv);

	if (opt_thread) {
		sigset_t set;
		sigemptyset(&set);
		sigaddset(&set, SIGUSR2);

		pthread_create(&thread2, NULL, thread2_routine, NULL);

		pthread_sigmask(SIG_BLOCK, &set, NULL);
	}


	sigemptyset(&waitset);
	sigaddset(&waitset, SIGINT);
	sigaddset(&waitset, SIGUSR1);

	pthread_sigmask(SIG_BLOCK, &waitset, NULL);

	flag_allocated = 0;

	for (;;) {
		signo = sigwaitinfo(&waitset, NULL);
		if (signo < 0)
			err(1, "sigwaitinfo() failed");

		if (signo == SIGUSR1) {
			if (opt_mmap_anon)
				mmap_anon(flag_allocated);

			if (opt_mmap_file)
				mmap_file(flag_allocated);

			if (opt_mmap_lock1)
				mmap_lock1(flag_allocated);

			if (opt_mmap_lock2)
				mmap_lock2(flag_allocated);

			if (opt_shm)
				shm(flag_allocated);

			flag_allocated = !flag_allocated;
		} else {
			break;
		}
	}

	if (opt_thread) {
		void *retv;
		pthread_cancel(thread2);
		pthread_join(thread2, &retv);
	}

	close(fd);

	return 0;
}

#else
int main(void)
{
	printf("System doesn't have required mempolicy support\n");
	return 1;
}
#endif
