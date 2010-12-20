/*
 * Test program for memory error handling for hugepages
 * Author: Naoya Horiguchi <n-horiguchi@ah.jp.nec.com>
 */
#define _GNU_SOURCE 1
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include "hugepage.h"

#define FILE_BASE  "test"

#define HPAGE_SIZE (2UL*1024*1024)
#define BUF_SIZE   256
#define PROTECTION (PROT_READ | PROT_WRITE)

#ifndef SHM_HUGETLB
#define SHM_HUGETLB 04000
#endif

/* Control early_kill/late_kill */
#define PR_MCE_KILL 33
#define PR_MCE_KILL_CLEAR   0
#define PR_MCE_KILL_SET     1
#define PR_MCE_KILL_LATE    0
#define PR_MCE_KILL_EARLY   1
#define PR_MCE_KILL_DEFAULT 2
#define PR_MCE_KILL_GET 34

#define MADV_HWPOISON		100
#define MADV_SOFT_OFFLINE	101

int PS; /* Page size */
int file_size; /* Memory allocation size (hugepage unit) */
/* Error injection position (page offset from the first hugepage head) */
int corrupt_page;
char filename[BUF_SIZE] = "/test";
char filepath[BUF_SIZE];

#define DEB printf("DEBUG [%d:%s:%d]\n", getpid(), __FILE__, __LINE__);

static void usage(void)
{
	printf(
"./thugetlb [-m memory] [-o offset] [-f file] [-xOeSAaFpch] hugetlbfs_directory\n"
"            -m|--memory size(hugepage unit)    Size of hugetlbfs file\n"
"            -o|--offset offset(page unit)      Position of error injection\n"
"            -x|--inject                        Error injection switch\n"
"            -O|--offline                       Soft offline switch\n"
"            -e|--early-kill                    Set PR_MCE_KILL_EARLY\n"
"            -S|--shm                           Use shmem with SHM_HUGETLB\n"
"            -A|--anonymous                     Use MAP_ANONYMOUS\n"
"            -a|--avoid-touch                   Avoid touching error page\n"
"            -F|--fork\n"
"            -p|--private\n"
"            -c|--cow\n"
"            -f|--filename string\n"
"            -h|--help\n"
"\n"
	);
}

/*
 * semaphore get/put wrapper
 */
int get_semaphore(int sem_id, struct sembuf *sembuffer)
{
	sembuffer->sem_num = 0;
	sembuffer->sem_op  = -1;
	sembuffer->sem_flg = SEM_UNDO;
	return semop(sem_id, sembuffer, 1);
}

int put_semaphore(int sem_id, struct sembuf *sembuffer)
{
	sembuffer->sem_num = 0;
	sembuffer->sem_op  = 1;
	sembuffer->sem_flg = SEM_UNDO;
	return semop(sem_id, sembuffer, 1);
}

static struct option opts[] = {
	{ "memory"          , 1, NULL, 'm' },
	{ "offset"          , 1, NULL, 'o' },
	{ "inject"          , 0, NULL, 'x' },
	{ "offline"         , 0, NULL, 'O' },
	{ "early_kill"      , 0, NULL, 'e' },
	{ "shm"             , 0, NULL, 'S' },
	{ "anonymous"       , 0, NULL, 'A' },
	{ "avoid-touch"     , 0, NULL, 'a' },
	{ "fork"            , 0, NULL, 'F' },
	{ "private"         , 0, NULL, 'p' },
	{ "cow"             , 0, NULL, 'c' },
	{ "filename"        , 1, NULL, 'f' },
	{ "help"            , 0, NULL, 'h' },
	{ NULL              , 0, NULL,  0  }
};

int main(int argc, char *argv[])
{
	void *addr;
	int i;
	int ret;
	int fd = 0;
	int semid;
	int semaphore;
	int inject = 0;
	int madvise_code = MADV_HWPOISON;
	int early_kill = 0;
	int avoid_touch = 0;
	int anonflag = 0;
	int shmflag = 0;
	int shmkey = 0;
	int forkflag = 0;
	int privateflag = 0;
	int cowflag = 0;
	char c;
	pid_t pid = 0;
	void *expected_addr = NULL;
	struct sembuf sembuffer;

	PS = getpagesize();
	HPS = HPAGE_SIZE;
	file_size = 1;
	corrupt_page = -1;

	if (argc == 1) {
		usage();
		exit(EXIT_FAILURE);
	}

	while ((c = getopt_long(argc, argv,
				"m:o:xOeSAaFpcf:h", opts, NULL)) != -1) {
		switch (c) {
		case 'm':
			file_size = strtol(optarg, NULL, 10);
			break;
		case 'o':
			corrupt_page = strtol(optarg, NULL, 10);
			break;
		case 'x':
			inject = 1;
			break;
		case 'O':
			madvise_code = MADV_SOFT_OFFLINE;
			break;
		case 'e':
			early_kill = 1;
			break;
		case 'S':
			shmflag = 1;
			break;
		case 'A':
			anonflag = 1;
			break;
		case 'a':
			avoid_touch = 1;
			break;
		case 'F':
			forkflag = 1;
			break;
		case 'p':
			privateflag = 1;
			break;
		case 'c':
			cowflag = 1;
			break;
		case 'f':
			strcat(filename, optarg);
			shmkey = strtol(optarg, NULL, 10);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
		default:
			usage();
			exit(EXIT_FAILURE);
		}
	}

	if (inject && corrupt_page * PS > file_size * HPAGE_SIZE)
		errmsg("Target page is out of range.\n");

	if (avoid_touch && corrupt_page == -1)
		errmsg("Avoid which page?\n");

	/* Construct file name */
	if (access(argv[argc - 1], F_OK) == -1) {
		usage();
		exit(EXIT_FAILURE);
	} else {
		strcpy(filepath, argv[argc - 1]);
		strcat(filepath, filename);
	}

	if (shmflag) {
		addr = alloc_shm_hugepage(&shmkey, file_size * HPAGE_SIZE);
		if (!addr)
			errmsg("Failed in alloc_shm_hugepage()");
	} else if (anonflag) {
		addr = alloc_anonymous_hugepage(file_size * HPAGE_SIZE,
						privateflag);
		if (!addr)
			errmsg("Failed in alloc_anonymous_hugepage()");
	} else {
		addr = alloc_filebacked_hugepage(filepath,
						 file_size * HPAGE_SIZE,
						 privateflag, &fd);
		if (!addr)
			errmsg("Failed in alloc_filebacked_hugepage()");
	}

	if (corrupt_page != -1 && avoid_touch)
		expected_addr = (void *)(addr + corrupt_page / 512 * HPAGE_SIZE);

	if (forkflag) {
		semid = semget(IPC_PRIVATE, 1, 0666|IPC_CREAT);
		if (semid == -1) {
			perror("semget");
			goto cleanout;
		}
		semaphore = semctl(semid, 0, SETVAL, 1);
		if (semaphore == -1) {
			perror("semctl");
			goto cleanout;
		}
		if (get_semaphore(semid, &sembuffer)) {
			perror("get_semaphore");
			goto cleanout;
		}
	}

	write_hugepage(addr, file_size, 0);
	read_hugepage(addr, file_size, 0);

	if (early_kill)
		prctl(PR_MCE_KILL, PR_MCE_KILL_SET, PR_MCE_KILL_EARLY,
		      NULL, NULL);

	/*
	 * Intended order:
	 *   1. Child COWs
	 *   2. Parent madvise()s
	 *   3. Child exit()s
	 */
	if (forkflag) {
		pid = fork();
		if (!pid) {
			/* Semaphore is already held */
			if (cowflag) {
				write_hugepage(addr, 0, expected_addr);
				read_hugepage(addr, 0, expected_addr);
			}
			if (put_semaphore(semid, &sembuffer))
				err("put_semaphore");
			usleep(1000);
			/* Wait for madvise() to be done */
			if (get_semaphore(semid, &sembuffer))
				err("put_semaphore");
			if (put_semaphore(semid, &sembuffer))
				err("put_semaphore");
			return 0;
		}
	}

	/* Wait for COW */
	if (forkflag && get_semaphore(semid, &sembuffer)) {
		perror("get_semaphore");
		goto cleanout;
	}

	if (inject && corrupt_page != -1) {
		ret = madvise(addr + corrupt_page * PS, PS, madvise_code);
		if (ret) {
			printf("madivise return %d :", ret);
			perror("madvise");
			goto cleanout;
		}
	}

	if (forkflag && put_semaphore(semid, &sembuffer)) {
		perror("put_semaphore");
		goto cleanout;
	}

	if (madvise_code != MADV_SOFT_OFFLINE);
		write_hugepage(addr, file_size, expected_addr);
	read_hugepage(addr, file_size, expected_addr);

	if (forkflag) {
		if (wait(&i) == -1)
			err("wait");
		if (semctl(semid, 0, IPC_RMID) == -1)
			err("semctl(IPC_RMID)");
	}
cleanout:
	if (shmflag) {
		if (free_shm_hugepage(shmkey, addr) == -1)
			exit(2);
	} else if (anonflag) {
		if (free_anonymous_hugepage(addr, file_size * HPAGE_SIZE) == -1)
			exit(2);
	} else {
		if (free_filebacked_hugepage(addr, file_size * HPAGE_SIZE,
					     fd, filepath) == -1)
			exit(2);
	}

	return 0;
}
