/*
 * Test program for Linux poison memory error recovery.
 * This injects poison into various mapping cases and triggers the poison
 * handling.  Requires special injection support in the kernel.
 * Author: Andi Kleen
 */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <setjmp.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#ifndef MADV_POISON
#define MADV_POISON 100
#endif

#define TMPDIR "./"
#define PATHBUFLEN 100

#define err(x) perror(x),exit(1)
#define Perror(x) failure++, perror(x)
#define PAIR(x) x, sizeof(x)-1
#define mb() asm volatile("" ::: "memory")
#if defined(__i386__) || defined(__x86_64__)
#define cpu_relax() asm volatile("rep ; nop" ::: "memory")
#else
#define cpu_relax() mb()
#endif

int PS;
int failure;
int unexpected;
int early_kill;

void *checked_mmap(void *start, size_t length, int prot, int flags,
                   int fd, off_t offset)
{
	void *map = mmap(start, length, prot, flags, fd, offset);
	if (map == (void*)-1L)
		err("mmap");
	return map;
}

void munmap_reserve(void *page, int size)
{
	munmap(page, size);
	mmap(page, size, PROT_NONE, MAP_PRIVATE|MAP_FIXED, 0, 0);
}

void *xmalloc(size_t s)
{
	void *p = malloc(s);
	if (!p)
		exit(ENOMEM);
	return p;
}

int recovercount;
sigjmp_buf recover_ctx;
sigjmp_buf early_recover_ctx;
void *expected_addr;

void sighandler(int sig, siginfo_t *si, void *arg)
{
	if (si->si_addr != expected_addr) {
		printf("XXX: Unexpected address in signal %p (expected %p)\n", si->si_addr,
			expected_addr);
		failure++;
	}

	printf("signal %d code %d addr %p\n", sig, si->si_code, si->si_addr);

	if (--recovercount == 0) {
		write(1, PAIR("I seem to be in a signal loop. bailing out.\n"));
		exit(1);
	}

	if (si->si_code == 4)
		siglongjmp(recover_ctx, 1);
	else
		siglongjmp(early_recover_ctx, 1);
}

enum rmode {
	MREAD = 0,
	MWRITE = 1,
	MREAD_OK = 2,
	MWRITE_OK = 3,
	MNOTHING = -1,
};

void poison(char *msg, char *page, enum rmode mode)
{
	expected_addr = page;
	recovercount = 5;

	if (sigsetjmp(early_recover_ctx, 1) == 0) {

		if (madvise(page, PS, MADV_POISON) != 0) {
			if (errno == EINVAL) {
				printf("Kernel doesn't support poison injection\n");
				exit(0);
			}
			Perror("madvise");
			return;
		}

		if (early_kill && (mode == MWRITE || mode == MREAD)) {
			printf("XXX: %s: process is not early killed\n", msg);
			failure++;
		}

		return;
	}

	if (early_kill) {
		if (mode == MREAD_OK || mode == MWRITE_OK) {
			printf("XXX: %s: killed\n", msg);
			failure++;
		} else
			printf("recovered\n");
	}
}

void recover(char *msg, char *page, enum rmode mode)
{
	expected_addr = page;
	recovercount = 5;

	if (sigsetjmp(recover_ctx, 1) == 0) {
		switch (mode) {
		case MWRITE:
			printf("writing 2\n");
			*page = 2;
			break;
		case MWRITE_OK:
			printf("writing 4\n");
			*page = 4;
			return;
		case MREAD:
			printf("%x\n", *(unsigned char *)page);
			break;
		case MREAD_OK:
			printf("%x\n", *(unsigned char *)page);
			return;
		case MNOTHING:
			return;
		}
		/* signal or kill should have happened */
		printf("XXX: %s: page not poisoned after injection\n", msg);
		failure++;
		return;
	}
	if (mode == MREAD_OK || mode == MWRITE_OK) {
		printf("XXX: %s: killed\n", msg);
		failure++;
	} else
		printf("recovered\n");
}

void testmem(char *msg, char *page, enum rmode mode)
{
	printf("%s page %p\n", msg, page);
	poison(msg, page, mode);
	recover(msg, page, mode);
}

void expecterr(char *msg, int err)
{
	if (err) {
		printf("expected error %d on %s\n", errno, msg);
	} else {
		failure++;
		printf("XXX: unexpected no error on %s\n", msg);
	}
}

void optionalerr(char *msg, int err)
{
	if (err) {
		printf("expected optional error %d on %s\n", errno, msg);
	} else {
		unexpected++;
		printf("XXX: expected likely incorrect no error on %s\n", msg);
	}
}

static int tmpcount;
int tempfd(void)
{
	int fd;
	char buf[PATHBUFLEN];
	snprintf(buf, sizeof buf, TMPDIR "poison%d",tmpcount++);
	fd = open(buf, O_CREAT|O_RDWR, 0600);
	if (fd >= 0)
		unlink(buf);
	if (fd < 0)
		err("opening temporary file in " TMPDIR);
	return fd;
}

int playfile(char *buf)
{
	int fd;
	if (buf[0] == 0)
		snprintf(buf, PATHBUFLEN, TMPDIR "poison%d", tmpcount++);
	fd = open(buf, O_CREAT|O_RDWR|O_TRUNC, 0600);
	if (fd < 0)
		err("opening temporary file in " TMPDIR);

	const int NPAGES = 5;
	char *tmp = xmalloc(PS * NPAGES);
	int i;
	for (i = 0; i < PS*NPAGES; i++)
		tmp[i] = i;
	write(fd, tmp, PS*NPAGES);

	lseek(fd, 0, SEEK_SET);
	return fd;
}

static void dirty_anonymous(void)
{
	char *page;
	page = checked_mmap(NULL, PS, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, 0, 0);
	testmem("dirty", page, MWRITE);
}

static void dirty_anonymous_unmap(void)
{
	char *page;
	page = checked_mmap(NULL, PS, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, 0, 0);
	testmem("dirty", page, MWRITE);
	munmap_reserve(page, PS);
}

static void mlocked_anonymous(void)
{
	char *page;
	page = checked_mmap(NULL, PS, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_LOCKED, 0, 0);
	testmem("mlocked", page, MWRITE);
}

static void file_clean(void)
{
	char *page;
	char fn[30];
	snprintf(fn, 30, TMPDIR "test%d", tmpcount++);
	int fd = open(fn, O_RDWR|O_TRUNC|O_CREAT);
	if (fd < 0)
		err("open temp file");
	write(fd, fn, 4);
	fsync(fd);
	page = checked_mmap(NULL, PS, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	testmem("file clean", page, MREAD_OK);
	printf("%x\n", *(unsigned char *)page); /* reread page from disk */
	testmem("file clean", page, MWRITE_OK);
}

static void file_dirty(void)
{
	char *page;
	char fn[PATHBUFLEN];
	fn[0] = 0;
	int fd = playfile(fn);

	page = checked_mmap(NULL, PS, PROT_READ, MAP_SHARED|MAP_POPULATE, fd, 0);
	testmem("dirty file initial", page, MREAD);
	expecterr("msync expect error", msync(page, PS, MS_SYNC) < 0);
	close(fd);
	munmap_reserve(page, PS);

	fd = open(fn, O_RDONLY);
	if (fd < 0) err("reopening temp file");
	page = checked_mmap(NULL, PS, PROT_READ, MAP_SHARED|MAP_POPULATE, fd, 0);
	recover("dirty file populated", page, MREAD_OK);
	close(fd);
	munmap_reserve(page, PS);

	fd = open(fn, O_RDONLY);
	if (fd < 0) err("reopening temp file");
	page = checked_mmap(NULL, PS, PROT_READ, MAP_SHARED, fd, 0);
	recover("dirty file fault", page, MREAD_OK);
	close(fd);
	munmap_reserve(page, PS);

	fd = open(fn, O_RDWR);
	char buf[128];
	expecterr("explicit read after poison", read(fd, buf, sizeof buf) < 0);
	expecterr("explicit write after poison", write(fd, "foobar", 6) < 0);
	optionalerr("fsync expect error", fsync(fd) < 0);
	close(fd);

	/* should unlink return an error here? */
	if (unlink(fn) < 0)
		perror("unlink");
}

/* TBD */
static void file_hole(void)
{
	int fd = tempfd();
	char *page;

	ftruncate(fd, PS);
	page = checked_mmap(NULL, PS, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	*page = 1;
	testmem("hole file dirty", page, MREAD);
	expecterr("hole fsync expect error", fsync(fd) < 0);
	optionalerr("hole msync expect error", msync(page, PS, MS_SYNC) < 0);
	close(fd);
}

static void nonlinear(void)
{
	int fd;
	const int NPAGES = 10;
	int i;
	char *page;
	char *tmp;

	fd = tempfd();
	tmp = xmalloc(PS);
	for (i = 0; i < NPAGES; i++)  {
		memset(tmp, i, PS);
		write(fd, tmp, PS);
	}
	free(tmp);
	page = checked_mmap(NULL, PS*NPAGES, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	int k = NPAGES - 1;
	for (i = 0; i < NPAGES; i++, k--) {
		if (remap_file_pages(page + i*PS, PS, 0, k, 0))
			perror("remap_file_pages");
	}
	*page = 1;
	testmem("rfp file dirty", page, MREAD);
	expecterr("rfp fsync expect error", fsync(fd) < 0);
	optionalerr("rfp msync expect error", msync(page, PS, MS_SYNC) < 0);
	close(fd);
}

/*
 * This is quite timing dependent. The sniper might hit the page
 * before it is dirtied. If that happens tweak the delay
 * (should auto tune)
 */
enum {
	DELAY_NS = 30,
};

volatile enum sstate { START, WAITING, SNIPE } sstate;

void waitfor (enum sstate w, enum sstate s)
{
	sstate = w;
	mb();
	while (sstate != s)
		cpu_relax();
}

struct poison_arg {
	char *msg;
	char *page;
	enum rmode mode;
};

void *sniper(void *p)
{
	struct poison_arg *arg = p;

	waitfor (START, WAITING);
	nanosleep(&((struct timespec) { .tv_nsec = DELAY_NS }), NULL);
	poison(arg->msg, arg->page, arg->mode);
	return NULL;
}

int setup_sniper(struct poison_arg *arg)
{
	if (sysconf(_SC_NPROCESSORS_ONLN) < 2)  {
		printf("%s: Need at least two CPUs. Not tested\n", arg->msg);
		return -1;
	}
	sstate = START;
	mb();
	pthread_t thr;
	if (pthread_create(&thr, NULL, sniper, arg) < 0)
		err("pthread_create");
	pthread_detach(thr);
	return 0;
}

static void under_io_dirty(void)
{
	struct poison_arg arg;
	int fd = tempfd();
	char *page;

	page = checked_mmap(NULL, PS, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_POPULATE, fd, 0);

	arg.page = page;
	arg.msg  = "under io dirty";
	arg.mode = MWRITE;
	if (setup_sniper(&arg) < 0)
		return;

	write(fd, "xyz", 3);
	waitfor (WAITING, WAITING);
	expecterr("write under io", fsync(fd) < 0);
	close(fd);
}

static void under_io_clean(void)
{
	struct poison_arg arg;
	char fn[PATHBUFLEN];
	int fd;
	char *page;
	char buf[10];

 	fd = playfile(fn);
	page = checked_mmap(NULL, PS, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_POPULATE, fd, 0);
	madvise(page, PS, MADV_DONTNEED);

	arg.page = page;
	arg.msg  = "under io clean";
	arg.mode = MREAD_OK;
	if (setup_sniper(&arg) < 0)
		return;

	waitfor (WAITING, WAITING);
	// what is correct here?
	if (pread(fd, buf, 10, 0) != 0)
		perror("pread under io clean");
	close(fd);
}

struct testcase {
	void (*f)(void);
	char *name;
	int survivable;
} cases[] = {
	{ dirty_anonymous, "dirty anonymous" },
	{ dirty_anonymous_unmap, "dirty anonymous unmap" },
	{ mlocked_anonymous, "mlocked anonymous" },
	{ file_clean, "file clean", 1 },
	{ file_dirty, "file dirty" },
	{ file_hole, "file hole" },
	{ nonlinear, "nonlinear" },
	/* { under_io_dirty, "under io dirty" }, */
	/* { under_io_clean, "under io clean" }, */
	{}
};

int main(void)
{
	PS = getpagesize();

	/* don't kill me at poison time, but possibly at page fault time */
	early_kill = 0;
	system("sysctl -w vm.memory_failure_early_kill=0");

	struct sigaction sa = {
		.sa_sigaction = sighandler,
		.sa_flags = SA_SIGINFO
	};

	struct testcase *t;
	/* catch signals */
	sigaction(SIGBUS, &sa, NULL);
	for (t = cases; t->f; t++)
		t->f();

	/* suicide version */
	for (t = cases; t->f; t++) {
		pid_t child = fork();
		if (child == 0) {
			signal(SIGBUS, SIG_DFL);
			t->f();
			if (t->survivable)
				_exit(2);
			write(1, t->name, strlen(t->name));
			write(1, PAIR(" didn't kill itself?\n"));
			_exit(1);
		} else {
			siginfo_t sig;
			if (waitid(P_PID, child, &sig, WEXITED) < 0)
				perror("waitid");
			else {
				if (t->survivable) {
					if (sig.si_code != CLD_EXITED) {
						printf("XXX: %s: child not survived\n", t->name);
						failure++;
					}
				} else {
					if (sig.si_code != CLD_KILLED || sig.si_status != SIGBUS) {
						printf("XXX: %s: child not killed by SIGBUS\n", t->name);
						failure++;
					}
				}
			}
		}
	}

	/* early kill version */
	early_kill = 1;
	system("sysctl -w vm.memory_failure_early_kill=1");

	sigaction(SIGBUS, &sa, NULL);
	for (t = cases; t->f; t++)
		t->f();

	if (failure > 0) {
		printf("FAILURE -- %d cases broken!\n", failure);
		return 1;
	}
	printf("SUCCESS\n");
	return 0;
}
