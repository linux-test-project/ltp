/* random-access.c (GPL)*/
/* Hironobu SUZUKI <hironobu@h2np.net> */
#include <stdio.h>
#include <errno.h>
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

void open_read_close(char *fname);

int nullfd;

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

	if ((nullfd = open("/dev/null", O_WRONLY)) < 0) {
		perror("/dev/null");
		exit(1);
	}

	/* 00/00/00/00 */
	for (i = 0; i < m; i++) {
		r = random() % m;
		sprintf(fname, "00/%2.2x/%2.2x/00%2.2x%2.2x%2.2x",
			((r >> 16) & 0xFF),
			((r >> 8) & 0xFF),
			((r >> 16) & 0xFF), ((r >> 8) & 0xFF), (r & 0xFF));
		open_read_close(fname);
	}
	close(nullfd);
	printf("Success:\t%d\nFail:\t%d\n", openlog[SUCCESS], openlog[FAIL]);
	exit(0);
}

#define BUFS 8192
void open_read_close(char *fname)
{
	int fd;
	char buf[BUFS];
	int c;

	if ((fd = open(fname, O_RDONLY)) < 0) {
		openlog[FAIL]++;
		close(fd);
		return;
	}
	openlog[SUCCESS]++;
	while ((c = read(fd, buf, BUFS)) > 0) {
		if (write(nullfd, buf, c) < 0) {
			perror("/dev/null");
			printf("Opened\t %d\nUnopend:\t%d\n", openlog[SUCCESS],
			       openlog[FAIL]);
			exit(1);
		}
	}
	if (c < 0) {
		perror(fname);
		printf("Opened\t %d\nUnopend:\t%d\n", openlog[SUCCESS],
		       openlog[FAIL]);
		exit(1);
	}
	close(fd);
}
