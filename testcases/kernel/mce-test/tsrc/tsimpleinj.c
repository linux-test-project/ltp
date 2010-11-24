/* 
 * Test program for Linux memory error recovery. 
 * Requires special injection support.
 * 
 * This is a early primitive version of tinjpage.c,
 * but simpler to debug in some cases.
 */
#define _GNU_SOURCE 1
#include <sys/mman.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <setjmp.h>
#include <errno.h>
#include <string.h>

#define MADV_POISON 100

#define err(x) perror(x),exit(1)

int count = 20;
int failure = 0;
int total_cases = 0;
sigjmp_buf recover;
int PS;

void sighandler(int sig, siginfo_t *si, void *arg)
{
	printf("signal %d code %d addr %p\n", sig, si->si_code, si->si_addr);

	if (--count == 0)
		exit(1);

	siglongjmp(recover, 1);
}

void testmem(char *msg, char *page, int write)
{
	printf("%s page %p\n", msg, page);
	total_cases++;
	if (sigsetjmp(recover,1) == 0) {
		if (madvise(page, PS, MADV_POISON) != 0) {
			failure++;
			perror("madvise");
		}
		if (write)
			*page = 2;
		else
			printf("%x\n", *(unsigned char *)page);	
	}
	printf("recovered\n");
}

void expecterr(char *msg, int res)
{
	if (res == 0)
		printf("no error on %s\n", msg);
	else
		perror(msg);
}

int tempfd(void)
{
	static int tmpcount;
	int fd;
	char buf[30];
	snprintf(buf,30,"/tmp/test%d.XXXXXXXX",tmpcount++);
	fd = mkstemp(buf);
	if (fd >= 0)
		unlink(buf);
	return fd;
}

#define RANDOM_FILE "/etc/profile"

int main(void)
{
	PS = getpagesize();
	char *page;

	struct sigaction sa = { 	
		.sa_sigaction = sighandler,
		.sa_flags = SA_SIGINFO
	};
	sigaction(SIGBUS, &sa, NULL);
//	sigaction(SIGSEGV, &sa, NULL);

 	page = mmap(NULL, PS, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, 0, 0);
	testmem("dirty", page, 1);

	page = mmap(NULL, PS, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_LOCKED, 0, 0);
	testmem("mlocked", page, 1);

	int fd = open(RANDOM_FILE, O_RDONLY);
	if (fd < 0) err("open " RANDOM_FILE);
	page = mmap(NULL, PS, PROT_READ, MAP_SHARED|MAP_POPULATE, fd, 0);
	if (page == (char *)-1) err("mmap");
	close(fd);
	testmem("clean file", page, 0);

	fd = tempfd();
	if (fd < 0) err("open testfile");
	char *tmp = malloc(PS);
	if (!tmp) err("no enough memory");
	memset(tmp, 0xff, PS);
	write(fd, tmp, PS);
	free(tmp);
	page = mmap(NULL, PS, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (page == (char*)-1) err("mmap");
	*page = 1;
	testmem("file dirty", page, 0);
	expecterr("msync expect error", msync(page, PS, MS_SYNC));
	expecterr("fsync expect error", fsync(fd));
	close(fd);

	/* hole case still broken in the kernel -- doesn't report error */
	fd = tempfd();
	if (fd < 0) err("open testfile");
	ftruncate(fd, PS);
	page = mmap(NULL, PS, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (page == (char*)-1) err("mmap");
	*page = 1;
	testmem("hole file dirty", page, 0);
	expecterr("hole msync expect error", msync(page, PS, MS_SYNC));
	expecterr("hole fsync expect error", fsync(fd));
	close(fd);

#if 0
	const int NPAGES = 10;
	int i;
	fd = tempfd();
	if (fd < 0) err("open rfp testfile");
	tmp = malloc(PS);
	if (!tmp) exit(ENOMEM);
	for (i = 0; i < NPAGES; i++)  {
		memset(tmp, i, PS);
		write(fd, tmp, PS);
	}
	free(tmp);
	page = mmap(NULL, PS*NPAGES, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (page == (char*)-1) err("mmap");
	int k = NPAGES - 1;
	for (i = 0; i < NPAGES; i++, k--) {
		if (remap_file_pages(page + i*PS, PS, 0, k, 0))
			perror("remap_file_pages");
	}
	*page = 1;
	testmem("rfp file dirty", page, 0);
	expecterr("rfp msync expect error", msync(page, PS, MS_SYNC));
	expecterr("rfp fsync expect error", fsync(fd));
	close(fd);
#endif

	if (failure > 0) {
		printf("FAILURE -- %d of %d cases broken!\n", failure, total_cases);
		return 1;
	}
	printf("SUCCESS\n");

	return 0;
}	


