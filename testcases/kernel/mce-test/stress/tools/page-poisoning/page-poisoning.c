/*
 * Test program for Linux poison memory error recovery.
 * This program is extended from tinjpage with a multi-process model.
 *
 * This injects poison into various mapping cases and triggers the poison
 * handling.  Requires special injection support in the kernel.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; version
 * 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should find a copy of v2 of the GNU General Public License somewhere
 * on your Linux system; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Authors: Andi Kleen, Fengguang Wu, Haicheng Li
 *
 */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <setjmp.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <getopt.h>
#include <limits.h>

#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define MADV_POISON 100

#define PAGE_SIZE 4 * 1024
#define SHM_SIZE 1		// in page_size.
#define SHM_MODE 0600
#define FILE_SIZE 1 * 1024 * 1024 * 1024
#define LOG_BUFLEN 100

#define INSTANCE_NUM 10000

#define TEST_PASS 1
#define TEST_FAIL 0

static int PS = PAGE_SIZE;
static int instance = 0;	// index of the child process.
static int testid = 0;		// test index of the child process.
static int test_types = 0;	// totoal test types.
static int t_shm = -1;		// index of shm test case.
static int failure = 0;		// result of child process.
static int unexpected = 0;	// result of child process.
static int early_kill = 0;
struct test {
	int id;
	int result;
};
struct shm {
	int id;
	int ready;
	int done;
};
struct ipc {
	struct test test[INSTANCE_NUM];
	struct shm shm;
};
static int ipc_entry;
static int *shmptr = NULL;

static pid_t g_pid[INSTANCE_NUM] = { 0 };
static int shm_size = SHM_SIZE;
static int child_num = INSTANCE_NUM;
static int shm_child_num = 0;
static char log_file[PATH_MAX];
static FILE *log_fd = NULL;
static char result_file[PATH_MAX];
static FILE *result_fd = NULL;
static char tmp_dir[PATH_MAX] = { '\0' };
static int clean_env = 0;

static int semid_ready = 0;

static pid_t mypid;

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct semid_info *__buf;
};

enum rmode {
	MREAD = 0,
	MWRITE = 1,
	MREAD_OK = 2,
	MWRITE_OK = 3,
	MNOTHING = -1,
};

static struct option opts[] = {
	{"clean", 0, 0, 'C'},
	{"help", 0, 0, 'h'},
	{"instance", 0, 0, 'i'},
	{"log", 0, 0, 'l'},
	{"result", 0, 0, 'r'},
	{"shmsize", 0, 0, 's'},
	{"tmpdir", 0, 0, 't'},
	{"", 0, 0, '\0'}
};

static void help(void)
{
	printf("Usage: page-poisoning [OPTION]...\n"
	       "Stress test for Linux HWPOISON Page Recovery with multiple processes.\n"
	       "\n"
	       "Mandatory arguments to long options are mandatory for short options too.\n"
	       "  -C, --clean                record log and result in clean files.\n"
	       "  -h                         print this page\n"
	       "  -i, --child_num=NUM        spawn NUM processes to do test (default NUM = %d)\n"
	       "  -l, --log=LOG              record logs to file LOG.\n"
	       "  -r, --result=RESULT        record test result to file RESULT.\n"
	       "  -s, --shmsize=SIZE         each shared memory segment is SIZE-page based.\n"
	       "  -t, --tmpdir=DIR           create temporary files in DIR.\n\n",
	       INSTANCE_NUM);
}

static void err(const char *fmt, ...);
static void mylog(const char *fmt, ...)
{
	char buf[LOG_BUFLEN] = { '\0' };
	va_list args;
	if (!log_fd)
		err("no log file there\n");

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	printf("[pid %d] %s", mypid, buf);
	fprintf(log_fd, "[pid %d] %s", mypid, buf);
	fflush(log_fd);
	va_end(args);
}

static void result(const char *fmt, ...)
{
	char buf[LOG_BUFLEN] = { '\0' };
	va_list args;
	if (!result_fd)
		err("no result file there\n");

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	fprintf(result_fd, "[pid %d] %s", mypid, buf);
	fflush(result_fd);
	if (log_fd)
		mylog("%s", buf);
	va_end(args);
}

static void err(const char *fmt, ...)
{
	char buf[LOG_BUFLEN] = { '\0' };
	va_list args;
	va_start(args, fmt);

	vsprintf(buf, fmt, args);
	if (result_fd)
		result("error: %s :%s\n", buf, strerror(errno));
	else
		perror(buf);
	va_end(args);
	exit(1);
}

static void *checked_mmap(void *start, size_t length, int prot, int flags,
		   int fd, off_t offset)
{
	void *map = mmap(start, length, prot, flags, fd, offset);
	if (map == (void *)-1L)
		err("mmap");
	return map;
}

static void munmap_reserve(void *page, int size)
{
	munmap(page, size);
	mmap(page, size, PROT_NONE, MAP_PRIVATE | MAP_FIXED, 0, 0);
}

static void *xmalloc(size_t s)
{
	void *p = malloc(s);
	if (!p)
		exit(ENOMEM);
	return p;
}

static int recovercount;
static sigjmp_buf recover_ctx;
static sigjmp_buf early_recover_ctx;
static void *expected_addr;

static void sighandler(int sig, siginfo_t * si, void *arg)
{
	mylog("signal %d code %d addr %p\n", sig, si->si_code, si->si_addr);
	if (si->si_addr != expected_addr) {
		result("failed: Unexpected address in signal %p (expected %p)\n",
		       si->si_addr, expected_addr);
		failure++;
	}

	if (--recovercount == 0) {
		result("failed: I seem to be in a signal loop. bailing out.\n");
		exit(1);
	}

	if (si->si_code == 4)
		siglongjmp(recover_ctx, 1);
	else
		siglongjmp(early_recover_ctx, 1);
}

static void poison(char *msg, char *page, enum rmode mode)
{
	expected_addr = page;
	recovercount = 5;

	if (sigsetjmp(early_recover_ctx, 1) == 0) {
		if (madvise(page, PS, MADV_POISON) != 0) {
			if (errno == EINVAL) {
				result("failed: Kernel doesn't support poison injection\n");
				exit(0);
			}
			err("error: madvise: %s", strerror(errno));
			return;
		}

		if (early_kill && (mode == MWRITE || mode == MREAD)) {
			result("failed: %s: process is not early killed\n",
			       msg);
			failure++;
		}

		return;
	}

	if (early_kill) {
		if (mode == MREAD_OK || mode == MWRITE_OK) {
			result("failed: %s: killed\n", msg);
			failure++;
		} else
			mylog("pass: recovered\n");
	}
}

static void recover(char *msg, char *page, enum rmode mode)
{
	expected_addr = page;
	recovercount = 5;

	if (sigsetjmp(recover_ctx, 1) == 0) {
		switch (mode) {
		case MWRITE:
			mylog("writing 2\n");
			*page = 2;
			break;
		case MWRITE_OK:
			mylog("writing 4\n");
			*page = 4;
			return;
		case MREAD:
			mylog("reading %x\n", *(unsigned char *)page);
			break;
		case MREAD_OK:
			mylog("reading %x\n", *(unsigned char *)page);
			return;
		case MNOTHING:
			return;
		}
		/* signal or kill should have happened */
		result("failed: %s: page is not poisoned after injection\n", msg);
		failure++;
		return;
	}
	if (mode == MREAD_OK || mode == MWRITE_OK) {
		result("failed: %s: killed\n", msg);
		failure++;
	} else
		mylog("pass: recovered\n");
}

static void testmem(char *msg, char *page, enum rmode mode)
{
	mylog("%s poisoning page %p\n", msg, page);
	poison(msg, page, mode);
	recover(msg, page, mode);
}

static void expecterr(char *msg, int err)
{
	if (err) {
		mylog("pass: expected error %d on %s\n", errno, msg);
	} else {
		result("failed: unexpected no error on %s\n", msg);
		failure++;
	}
}

/*
 * Any optional error is really a deficiency in the kernel VFS error reporting
 * and should be eventually fixed and turned into a expecterr
 */
static void optionalerr(char *msg, int err)
{
	if (err) {
		mylog("pass: expected error %d on %s\n", errno, msg);
	} else {
		mylog("LATER: expected likely incorrect no error on %s\n", msg);
		unexpected++;
	}
}

static int playfile(char *buf)
{
	int fd;
	if (buf[0] == 0)
		snprintf(buf, PATH_MAX, "%s/dirty%d", tmp_dir, mypid);
	fd = open(buf, O_CREAT | O_RDWR | O_TRUNC, 0600);
	if (fd < 0)
		err("opening temporary file: %s", buf);

	const int NPAGES = 5;
	char *tmp = xmalloc(PS * NPAGES);
	int i;
	for (i = 0; i < PS * NPAGES; i++)
		tmp[i] = i;
	write(fd, tmp, PS * NPAGES);

	lseek(fd, 0, SEEK_SET);
	free(tmp);
	return fd;
}

static void dirty_anonymous(void)
{
	struct ipc *ipc;
	char *page;

	if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
		err("shmat error\n");
	ipc->test[instance].id = testid;
	page = checked_mmap(NULL, PS, PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, 0, 0);
	testmem("dirty", page, MWRITE);
	if (!failure)
		ipc->test[instance].result = TEST_PASS;
	shmdt(ipc);
}

static void dirty_anonymous_unmap(void)
{
	struct ipc *ipc;
	char *page;

	if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
		err("shmat error\n");
	ipc->test[instance].id = testid;
	page = checked_mmap(NULL, PS, PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, 0, 0);
	testmem("dirty", page, MWRITE);
	munmap_reserve(page, PS);
	if (!failure)
		ipc->test[instance].result = TEST_PASS;
	shmdt(ipc);
}

static void mlocked_anonymous(void)
{
	struct ipc *ipc;
	char *page;

	if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
		err("shmat error\n");
	ipc->test[instance].id = testid;
	page = checked_mmap(NULL, PS, PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS | MAP_LOCKED, 0, 0);
	testmem("mlocked", page, MWRITE);
	if (!failure)
		ipc->test[instance].result = TEST_PASS;
	shmdt(ipc);
}

static void do_file_clean(int flags, char *name)
{
	char *page;
	char fn[PATH_MAX];
	snprintf(fn, PATH_MAX, "%s/clean%d", tmp_dir, mypid);
	int fd = open(fn, O_RDWR | O_TRUNC | O_CREAT, 0600);
	if (fd < 0)
		err("opening temporary file: %s", fn);
	write(fd, fn, 4);
	page = checked_mmap(NULL, PS, PROT_READ | PROT_WRITE, MAP_SHARED | flags,
			    fd, 0);
	fsync(fd);
	close(fd);
	testmem(name, page, MREAD_OK);
	/* reread page from disk */
	mylog("reading %x\n", *(unsigned char *)page);
	testmem(name, page, MWRITE_OK);
}

static void file_clean(void)
{
	struct ipc *ipc;

	if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
		err("shmat error\n");
	ipc->test[instance].id = testid;
	do_file_clean(0, "file clean");
	if (!failure)
		ipc->test[instance].result = TEST_PASS;
	shmdt(ipc);
}

static void file_clean_mlocked(void)
{
	struct ipc *ipc;

	if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
		err("shmat error\n");
	ipc->test[instance].id = testid;
	do_file_clean(MAP_LOCKED, "file clean mlocked");
	if (!failure)
		ipc->test[instance].result = TEST_PASS;
	shmdt(ipc);
}

static char *ndesc(char *buf, char *name, char *add)
{
	snprintf(buf, 100, "%s %s", name, add);
	return buf;
}

static void do_file_dirty(int flags, char *name)
{
	char nbuf[100];
	char *page;
	char fn[PATH_MAX];
	fn[0] = 0;
	int fd = playfile(fn);

	page = checked_mmap(NULL, PS, PROT_READ,
			    MAP_SHARED | MAP_POPULATE | flags, fd, 0);
	testmem(ndesc(nbuf, name, "initial"), page, MREAD);
	expecterr("msync expect error", msync(page, PS, MS_SYNC) < 0);
	close(fd);
	munmap_reserve(page, PS);

	fd = open(fn, O_RDONLY);
	if (fd < 0)
		err("reopening temp file");
	page = checked_mmap(NULL, PS, PROT_READ, MAP_SHARED | MAP_POPULATE | flags,
			    fd, 0);
	recover(ndesc(nbuf, name, "populated"), page, MREAD_OK);
	close(fd);
	munmap_reserve(page, PS);

	fd = open(fn, O_RDONLY);
	if (fd < 0)
		err("reopening temp file");
	page = checked_mmap(NULL, PS, PROT_READ, MAP_SHARED | flags, fd, 0);
	recover(ndesc(nbuf, name, "fault"), page, MREAD_OK);
	close(fd);
	munmap_reserve(page, PS);

	fd = open(fn, O_RDWR);
	char buf[128];
	/* the earlier close has eaten the error */
	optionalerr("explicit read after poison", read(fd, buf, sizeof buf) < 0);
	optionalerr("explicit write after poison", write(fd, "foobar", 6) < 0);
	optionalerr("fsync expect error", fsync(fd) < 0);
	close(fd);

	/* should unlink return an error here? */
	if (unlink(fn) < 0)
		perror("unlink");
}

static void file_dirty(void)
{
	struct ipc *ipc;

	if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
		err("shmat error\n");
	ipc->test[instance].id = testid;
	do_file_dirty(0, "file dirty");
	if (!failure)
		ipc->test[instance].result = TEST_PASS;
	shmdt(ipc);
}

static void file_dirty_mlocked(void)
{
	struct ipc *ipc;

	if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
		err("shmat error\n");
	ipc->test[instance].id = testid;
	do_file_dirty(MAP_LOCKED, "file dirty mlocked");
	if (!failure)
		ipc->test[instance].result = TEST_PASS;
	shmdt(ipc);
}

static void request_sem(int id, int num)
{
	struct sembuf sb;

	sb.sem_num = num;
	sb.sem_op = -1;
	sb.sem_flg = 0;

	semop(id, &sb, 1);
}

static void waiton_sem(int id, int num)
{
	struct sembuf sb;

	sb.sem_num = num;
	sb.sem_flg = 0;

	sb.sem_op = -1;
	semop(id, &sb, 1);
	sb.sem_op = 0;
	semop(id, &sb, 1);
}

static void release_sem(int id, int num)
{
	struct sembuf sb;

	sb.sem_num = num;
	sb.sem_op = 1;
	sb.sem_flg = 0;

	semop(id, &sb, 1);
}

static void clean_anonymous(void)
{
	char *page;
	page = checked_mmap(NULL, PS, PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	testmem("clean", page, MWRITE_OK);
}

static void anon_clean(void)
{
	struct ipc *ipc;

	if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
		err("shmat error\n");
	ipc->test[instance].id = testid;
	clean_anonymous();
	if (!failure)
		ipc->test[instance].result = TEST_PASS;
	shmdt(ipc);
}

/* TBD
static void survival(void)
{
	struct ipc *ipc;
	char page;

	if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
		err("shmat error\n");
	ipc->test[instance].id = testid;
	testmem("survial", &page, MNOTHING);
	if (!failure)
		ipc->test[instance].result = TEST_PASS;
	shmdt(ipc);
}
*/

static void shm_test(void)
{
	struct ipc *ipc;

	if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
		err("shmat error\n");
	ipc->test[instance].id = testid;

	request_sem(semid_ready, 0);
	if (!ipc->shm.ready) {
		if ((ipc->shm.id = shmget(IPC_PRIVATE, shm_size * PS,
					  SHM_MODE)) < 0)
			err("shmget error\n");
		ipc->shm.ready = 1;
	}
	if ((shmptr = shmat(ipc->shm.id, 0, 0)) == (void *)-1) {
		err("shmat error\n");
	} else
		*shmptr = mypid;
	release_sem(semid_ready, 0);

	waiton_sem(semid_ready, 1);

	request_sem(semid_ready, 0);
	if (!ipc->shm.done) {
		ipc->shm.done = 1;
		testmem("shm dirty", (char *)shmptr, MWRITE);
	} else
		recover("shm dirty", (char *)shmptr, MREAD);
	release_sem(semid_ready, 0);

	if (!failure)
		ipc->test[instance].result = TEST_PASS;
	shmdt(shmptr);
	shmdt(ipc);
}

static void setup_ipc(void)
{
	int size;
	union semun sunion;
	struct ipc *ipc;

	size = sizeof(struct ipc);

	if ((ipc_entry = shmget(IPC_PRIVATE, size, SHM_MODE)) < 0)
		err("shmget error\n");
	if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
		err("shmat error\n");
	memset(ipc, 0, sizeof(struct ipc));
	ipc->shm.id = -1;
	shmdt(ipc);

	semid_ready = semget(IPC_PRIVATE, 2, SHM_R | SHM_W);
	sunion.val = 1;
	semctl(semid_ready, 0, SETVAL, sunion);
	if (t_shm != -1) {
		if (((child_num - 1) % test_types) >= t_shm)
			shm_child_num = (child_num - 1) / test_types + 1;
		else
			shm_child_num = (child_num - 1) / test_types;
	}
	if (shm_child_num) {
		sunion.val = shm_child_num;
		semctl(semid_ready, 1, SETVAL, sunion);
		mylog("there are %d shm_child\n", shm_child_num);
	}
}

static void free_ipc(void)
{
	struct ipc *ipc;

	semctl(semid_ready, 0, IPC_RMID);
	if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
		err("shmat error\n");
	if (ipc->shm.id != -1)
		shmctl(ipc->shm.id, IPC_RMID, 0);
	shmdt(ipc);
	shmctl(ipc_entry, IPC_RMID, 0);
}

static void cleanup(void)
{
	int i;
	for (i = 0; i < instance; i++)
		kill(g_pid[i], 9);	//kill the suviving child.
	free_ipc();
}

struct testcase {
	void (*f) (void);
	char *name;
	int survivable;
} cases[] = {
	{
	shm_test, "shared memory test", 0}, {
	anon_clean, "anonymous clean", 1}, {
	dirty_anonymous, "anonymous dirty", 0}, {
	dirty_anonymous_unmap, "anonymous dirty unmap", 0}, {
	mlocked_anonymous, "anonymous dirty mlocked", 0}, {
	file_clean, "file clean", 1}, {
	file_dirty, "file dirty", 0}, {
	file_clean_mlocked, "file clean mlocked", 1}, {
	file_dirty_mlocked, "file dirty mlocked", 0},
//      { survival, "survival", 0 },
	{
	NULL, NULL, 0}
};

static int run_test(int children)
{
	pid_t pid = -1;
	int i = 0, rc = 0;
	siginfo_t sig;
	struct ipc *ipc;

	for (i = 0; i < children; i++) {
		pid = fork();
		if (pid < 0) {
			err("fork %d\n", i);
			break;
		} else if (pid == 0) {
			int j = instance % test_types;
			mypid = getpid();
			testid = j;
			cases[j].f();
			exit(0);
		} else {
			g_pid[i] = pid;
			++instance;
			fflush(stdout);
		}
	}

	mylog("have spawned %d processes\n", instance);
	if (instance) {
		if ((ipc = shmat(ipc_entry, 0, 0)) == (void *)-1)
			err("shmat error\n");

		for (i = 0; i < instance; i++) {
			int t = ipc->test[i].id;

			mylog("wait for Pid %d\n", g_pid[i]);
			waitid(P_PID, g_pid[i], &sig, WEXITED);
			if (ipc->test[i].result == TEST_PASS)
				result("Ins %d: Pid %d: pass - %s\n", i,
				      g_pid[i], cases[t].name);
			else {
				result("Ins %d: Pid %d: failed - %s\n", i,
				       g_pid[i], cases[t].name);
				failure++;
			}
		}
		shmdt(ipc);
	}

	if (!failure)
		result("\t!!! Page Poisoning Test got PASS. !!!\n\n");
	else {
		result("\t!!! Page Poisoning Test is FAILED (%d failures found). !!!\n\n",
		         failure);
		rc = 1;
	}

	return rc;
}

static void setup_log(void)
{
	int rc = 0;
	if (clean_env)
		log_fd = fopen(log_file, "w");
	else
		log_fd = fopen(log_file, "a");
	if (!log_fd)
		err("cannot open log file: %s\n", log_file);

	if (clean_env)
		result_fd = fopen(result_file, "w");
	else
		result_fd = fopen(result_file, "a");
	if (!result_fd)
		err("cannot open log file: %s\n", result_file);

	if (tmp_dir[0] != '\0') {
		rc = mkdir(tmp_dir, 0777);
		if (rc && errno != EEXIST)
			err("cannot create tmp dir: %s: %s\n", tmp_dir,
			    strerror(errno));
	}
}

static void free_log(void)
{
	fclose(log_fd);
	fclose(result_fd);
}

static void main_sighandler(int sig, siginfo_t * si, void *arg)
{
	mylog("receive signal to get terminated\n");
	cleanup();
	exit(1);
}

static void setup_sig(void)
{
	struct sigaction sa = {
		.sa_sigaction = main_sighandler,
		.sa_flags = SA_SIGINFO
	};
	struct sigaction sa_bus = {
		.sa_sigaction = sighandler,
		.sa_flags = SA_SIGINFO
	};

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGKILL, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGBUS, &sa_bus, NULL);
}

static void setup_test(void)
{
	struct testcase *t;
	/* catch signals */
	for (t = cases; t->f; t++)
		if (t->f == shm_test)
			t_shm = (t - cases);
	test_types = t - cases;
}

int main(int argc, char **argv)
{
	int rc = 0, c, opt_index;

	snprintf(log_file, sizeof(log_file), "page-poisoning.log");
	snprintf(result_file, sizeof(result_file), "page-poisoning.result");
	snprintf(tmp_dir, sizeof(tmp_dir), "./tmp");
	while (1) {
		c = getopt_long(argc, argv, "Chi:l:r:s:t:", opts, &opt_index);
		if (c == -1)
			break;
		switch (c) {
		case 'C':
			clean_env = 1;
			break;
		case 'h':
			help();
			return 0;
		case 'i':
			child_num = strtol(optarg, NULL, 0);
			if (child_num > INSTANCE_NUM)
				child_num = INSTANCE_NUM;
			break;
		case 'l':
			snprintf(log_file, sizeof(log_file), "%s", optarg);
			break;
		case 'r':
			snprintf(result_file, sizeof(result_file), "%s",
				 optarg);
			break;
		case 's':
			shm_size = strtol(optarg, NULL, 0);
			if (shm_size < SHM_SIZE)
				shm_size = SHM_SIZE;
			break;
		case 't':
			snprintf(tmp_dir, sizeof(tmp_dir), "%s", optarg);
			break;
		default:
			help();
			return 0;
		}
	}

	if (!early_kill)
		system("sysctl -w vm.memory_failure_early_kill=0");
	mypid = getpid();
	setup_log();
	setup_test();
	if (!child_num) {
		mylog("end without test executed since child_num = 0\n");
		return rc;
	}

	mylog("start page-poisoning test\n");
	PS = getpagesize();
	setup_ipc();
	setup_sig();
	rc = run_test(child_num);
	free_ipc();
	mylog("page-poisoning test done!\n");
	free_log();

	return rc;
}
