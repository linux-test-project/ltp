// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2009 FUJITSU LIMITED
 * Author: Li Zefan <lizf@cn.fujitsu.com>
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

static int fd;

static volatile int flag_exit;
static volatile int flag_allocated;

static int opt_mmap_anon;
static int opt_mmap_file;
static int opt_mmap_lock1;
static int opt_mmap_lock2;
static int opt_shm;
static int opt_hugepage;

static int key_id;			/* used with opt_shm */
static unsigned long memsize;

#define FILE_HUGEPAGE	"/hugetlb/hugepagefile"

#define MMAP_ANON	(SCHAR_MAX + 1)
#define MMAP_FILE	(SCHAR_MAX + 2)
#define MMAP_LOCK1	(SCHAR_MAX + 3)
#define MMAP_LOCK2	(SCHAR_MAX + 4)
#define SHM		(SCHAR_MAX + 5)
#define HUGEPAGE	(SCHAR_MAX + 6)

static const struct option long_opts[] = {
	{"mmap-anon", 0, NULL, MMAP_ANON},
	{"mmap-file", 0, NULL, MMAP_FILE},
	{"mmap-lock1", 0, NULL, MMAP_LOCK1},
	{"mmap-lock2", 0, NULL, MMAP_LOCK2},
	{"shm", 0, NULL, SHM},
	{"hugepage", 0, NULL, HUGEPAGE},
	{"size", 1, NULL, 's'},
	{"key", 1, NULL, 'k'},
	{NULL, 0, NULL, 0},
};

/*
 * process_options: read options from user input
 */
static void process_options(int argc, char *argv[])
{
	int c;
	char *end;

	while ((c = getopt_long(argc, argv, "k:s:", long_opts, NULL)) != -1) {
		switch (c) {
		case 'k':
			key_id = atoi(optarg);
			break;
		case 's':
			memsize = strtoul(optarg, &end, 10);
			if (*end != '\0')
				errx(1, "wrong -s argument!");
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
		default:
			errx(1, "unknown option: %c", c);
			break;
		}
	}
}

/*
 * touch_memory: force allocating phy memory
 */
static void touch_memory(char *p, int size)
{
	int i;
	int pagesize = getpagesize();

	for (i = 0; i < size; i += pagesize)
		p[i] = 0xef;
}

static void mmap_anon(void)
{
	static char *p;

	if (!flag_allocated) {
		p = mmap(NULL, memsize, PROT_WRITE | PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
		if (p == MAP_FAILED)
			err(1, "mmap(anonymous) failed");
		touch_memory(p, memsize);
	} else {
		if (munmap(p, memsize) == -1)
			err(1, "munmap(anonymous) failed");
	}
}

static void mmap_file(void)
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
		touch_memory(p, memsize);
	} else {
		if (munmap(p, memsize) == -1)
			err(1, "munmap(file) failed");

		if (opt_hugepage) {
			close(fd_hugepage);
			unlink(FILE_HUGEPAGE);
		}
	}
}

static void mmap_lock1(void)
{
	static char *p;

	if (!flag_allocated) {
		p = mmap(NULL, memsize, PROT_WRITE | PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS | MAP_LOCKED, 0, 0);
		if (p == MAP_FAILED)
			err(1, "mmap(lock) failed");
		touch_memory(p, memsize);
	} else {
		if (munmap(p, memsize) == -1)
			err(1, "munmap(lock) failed");
	}
}

static void mmap_lock2(void)
{
	static char *p;

	if (!flag_allocated) {
		p = mmap(NULL, memsize, PROT_WRITE | PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
		if (p == MAP_FAILED)
			err(1, "mmap failed");

		if (mlock(p, memsize) == -1) {
			if (errno == EAGAIN)
				exit(2);
			else
				err(1, "mlock failed");
		}
	} else {
		if (munmap(p, memsize) == -1)
			err(1, "munmap failed");
	}
}

static void shm(void)
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
			err(1, "ftok() failed");

		shmid = shmget(key, memsize, flag);
		if (shmid == -1)
			err(1, "shmget() failed");

		p = shmat(shmid, NULL, 0);
		if (p == (void *)-1) {
			shmctl(shmid, IPC_RMID, NULL);
			err(1, "shmat() failed");
		}

		if (shmctl(shmid, IPC_RMID, NULL) == -1)
			err(1, "shmctl() failed");

		touch_memory(p, memsize);
	} else {
		if (shmdt(p) == -1)
			err(1, "shmdt() failed");
	}
}

/*
 * sigint_handler: handle SIGINT by set the exit flag.
 */
static void sigint_handler(int __attribute__ ((unused)) signo)
{
	flag_exit = 1;
}

/*
 * sigusr_handler: handler SIGUSR
 *
 * When we receive SIGUSR, we allocate some memory according
 * to the user input when the process started.
 *
 * When we receive SIGUSR again, we will free all the allocated
 * memory.
 */
static void sigusr_handler(int __attribute__ ((unused)) signo)
{
	if (opt_mmap_anon)
		mmap_anon();

	if (opt_mmap_file)
		mmap_file();

	if (opt_mmap_lock1)
		mmap_lock1();

	if (opt_mmap_lock2)
		mmap_lock2();

	if (opt_shm)
		shm();

	flag_allocated = !flag_allocated;
}

int main(int argc, char *argv[])
{
	struct sigaction sigint_action;
	struct sigaction sigusr_action;

	if ((fd = open("/dev/zero", O_RDWR)) == -1)
		err(1, "open /dev/zero failed");

	memset(&sigint_action, 0, sizeof(sigint_action));
	memset(&sigusr_action, 0, sizeof(sigusr_action));

	sigemptyset(&sigint_action.sa_mask);
	sigint_action.sa_handler = &sigint_handler;
	if (sigaction(SIGINT, &sigint_action, NULL))
		err(1, "sigaction(SIGINT)");

	sigemptyset(&sigusr_action.sa_mask);
	sigusr_action.sa_handler = &sigusr_handler;
	if (sigaction(SIGUSR1, &sigusr_action, NULL))
		err(1, "sigaction(SIGUSR1)");

	process_options(argc, argv);

	tst_reinit();

	TST_CHECKPOINT_WAKE(0);

	while (!flag_exit)
		sleep(1);

	close(fd);

	return 0;
}
