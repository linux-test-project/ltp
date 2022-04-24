/* random-del-create.c (GPL)*/
/* Hironobu SUZUKI <hironobu@h2np.net> */

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#define FAIL 0
#define SUCCESS 1

static int openlog[2] = { 0, 0 };

#define MAXNUM 0x100000

#define  MAXERROR 1024

extern int box_muler(int, int);
extern void create_or_delete(char *);

int cfilecount = 0;
int dfilecount = 0;
int errorcount = 0;

int main(int ac, char **av)
{
	int r;
	char fname[1024];
	time_t t;
	int i;
	int m;

	if (ac != 2) {
		printf("%s hex-style-filename \n", av[0]);
		printf("ex) %s 00022300\n", av[0]);
		exit(1);
	}
	sscanf(av[1], "%x", &m);
	if (m < 1 || m > MAXNUM) {
		printf("out of size %d\n", m);
		exit(1);
	}

	time(&t);
	srandom((unsigned int)getpid() ^
		(((unsigned int)t << 16) | (unsigned int)t >> 16));

	/* 00/00/00/00 */
	for (i = 0; i < m; i++) {
		r = random() % m;
		sprintf(fname, "00/%2.2x/%2.2x/00%2.2x%2.2x%2.2x",
			((r >> 16) & 0xFF),
			((r >> 8) & 0xFF),
			((r >> 16) & 0xFF), ((r >> 8) & 0xFF), (r & 0xFF));
		create_or_delete(fname);
	}
	fprintf(stderr, "Total create files: %d\n", cfilecount);
	fprintf(stderr, "Total delete files: %d\n", dfilecount);
	fprintf(stderr, "Total error       : %d\n", errorcount);
	exit(0);
}

#define MAXFSIZE (192*1024)
#define AVEFSIZE (MAXFSIZE/2)
#define POOLDISKSPACE (AVEFSIZE*128)

static int disk_space_pool = 0;
void create_or_delete(char *fname)
{
	int r;
	int fsize;

	r = (random() & 1);
	if (r && disk_space_pool > POOLDISKSPACE) {
		/* create */
		create_file(fname);
	} else {
		delete_file(fname);
	}
	if ((errorcount > dfilecount || errorcount > cfilecount)
	    && (errorcount > MAXERROR)) {
		fprintf(stderr, "too much error -- stop\n");
		fprintf(stderr, "Total create files: %d\n", cfilecount);
		fprintf(stderr, "Total delete files: %d\n", dfilecount);
		fprintf(stderr, "Total error       : %d\n", errorcount);
		exit(1);
	}
}

int create_file(char *filename)
{
	int fd;
	int randomsize;
	char wbuf[MAXFSIZE];
	if ((fd = creat(filename, S_IRWXU)) < 0) {
		errorcount++;
		return (-1);
	}
	if ((randomsize = box_muler(0, MAXFSIZE)) < 0) {
		randomsize = MAXFSIZE;
	}
	if (write(fd, wbuf, randomsize) < 0) {
		errorcount++;
		close(fd);
		return (-1);
	}
	cfilecount++;
	disk_space_pool -= randomsize;
	close(fd);
	return 0;
}

#include <sys/stat.h>
#include <unistd.h>

int delete_file(char *filename)
{
	struct stat buf;
	int st;
	st = stat(filename, &buf);
	if (st < 0) {
		errorcount++;
		return (-1);
	}
	disk_space_pool += buf.st_size;
	if (unlink(filename) < 0) {
		errorcount++;
		return (-1);
	}
	dfilecount++;
	return 0;
}
