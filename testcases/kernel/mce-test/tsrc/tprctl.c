// test prctl per process setting
#define _GNU_SOURCE 1
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <setjmp.h>
#include <signal.h>

#define err(x) perror("FAILURE: " x), exit(1)
#define fail(x) printf("FAILURE: " x "\n"), exit(1)
#define mb() asm volatile("" ::: "memory")

#define MADV_POISON 100

/*
 * Set early/late kill mode for hwpoison memory corruption.
 * This influences when the process gets killed on a memory corruption.
 */
#define PR_MCE_KILL     33
# define PR_MCE_KILL_CLEAR   0
# define PR_MCE_KILL_SET     1

# define PR_MCE_KILL_LATE    0
# define PR_MCE_KILL_EARLY   1
# define PR_MCE_KILL_DEFAULT 2

#define PR_MCE_KILL_GET 34

sigjmp_buf recover_ctx;
volatile int seq;

void handler(int sig)
{
	siglongjmp(recover_ctx, 1);
}

void test(int early)
{
	int PS = getpagesize();
	char *ptr = mmap(NULL, PS, PROT_READ|PROT_WRITE,
			 MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, 0,0);
	if (ptr == (char *)-1L)
		err("mmap");
	signal(SIGBUS, handler);
	printf("ptr = %p\n", ptr);
	if (sigsetjmp(recover_ctx, 1) == 0) { 
		seq = 0;
		printf("injection\n");
		if (madvise(ptr, PS, MADV_POISON) < 0)
			err("MADV_POISON");
		/* early kill should kill here */
		seq++;
		mb();
		printf("faulting\n");
		/* late kill should kill here */
		*ptr = 1;
		printf("finished\n");
	} else {
		printf("recovered\n");
		if (seq == 1 && early)
			fail("early mode set, but no early kill");
		if (seq == 0 && !early)
			fail("late mode set, but early kill");
	}
}

int main(void)
{
	int err;
	err = prctl(PR_MCE_KILL_GET, 0, 0, 0, 0, 0);
	if (err < 0)
		err("PR_MCE_KILL_GET");
	if (err != PR_MCE_KILL_DEFAULT)
		fail("starting policy not default");
	if (prctl(PR_MCE_KILL, PR_MCE_KILL_SET, PR_MCE_KILL_LATE, 0, 0, 0) < 0)
		err("PR_MCE_KILL_SET late");
	test(0);	
	if (prctl(PR_MCE_KILL, PR_MCE_KILL_SET, PR_MCE_KILL_EARLY, 0, 0, 0) < 0)
		err("PR_MCE_KILL_SET early");
	test(1);
	err = prctl(PR_MCE_KILL_GET, 0, 0, 0,0,0);
	if (err < 0)
		err("PR_MCE_KILL_GET");
	if (err != PR_MCE_KILL_EARLY)
		fail("get mode not early after setting");
	if (prctl(PR_MCE_KILL, PR_MCE_KILL_CLEAR, 0, 0,0,0) < 0)
		err("PR_MCE_KILL_CLEAR");
	err = prctl(PR_MCE_KILL_GET, 0, 0, 0, 0, 0);
	if (err < 0)
		err("PR_MCE_KILL_GET");
	if (err != PR_MCE_KILL_DEFAULT)
		fail("ending policy not default");
	return 0;
}
