/* create.c (GPL)*/
/* Hironobu SUZUKI <hironobu@h2np.net> */
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define MAXN 4096

#define MAXFSIZE 1024 * 192

char wbuf[MAXFSIZE];
static int filecount = 0;

void makedir(char *dir1);
void changedir(char *dir);
void create_file(char *filename);

extern int box_muler(int, int);

int startc = 0;
int main(int ac, char *av[])
{
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	char dir1[MAXN];
	char dir2[MAXN];
	char dir3[MAXN];
	char filename[MAXN];
	time_t t;
	int maxfiles = 0xFFFFFF;
	int createfiles = 0;

	if (ac > 1) {
		sscanf(av[1], "%x", &maxfiles);
		if (maxfiles == 0) {
			printf("maxfile argument error (0 value)\n");
			exit(1);
		}
	}
	time(&t);
	srandom((unsigned int)getpid() ^
		(((unsigned int)t << 16) | (unsigned int)t >> 16));
	printf("Create files\n");
	for (i = 0; i < 0xFF; i++) {
		sprintf(dir1, "%2.2x", i);
		makedir(dir1);
		changedir(dir1);
		for (j = 0; j < 0xFF; j++) {
			sprintf(dir2, "%2.2x", j);
			makedir(dir2);
			changedir(dir2);
			for (k = 0; k < 0xFF; k++) {
				sprintf(dir3, "%2.2x", k);
				makedir(dir3);
				changedir(dir3);
				for (l = 0; l < 0xFF; l++) {
					sprintf(filename, "%s%s%s%2.2x", dir1,
						dir2, dir3, l);
					create_file(filename);
					if (maxfiles < createfiles++) {
						goto end;
					}
				}
				changedir("../");
			}
			changedir("../");
		}
		changedir("../");
	}
end:
	fprintf(stderr, "\nTotal create files: %d\n", filecount);
	printf("Done\n");
	return 0;
}

int showchar[] = { 124, 47, 45, 92, 124, 47, 45, 92 };

void makedir(char *dir1)
{
	if (mkdir(dir1, S_IRWXU) < 0) {
		perror(dir1);
		exit(1);
	}
}

void changedir(char *dir)
{
	if (chdir(dir) < 0) {
		perror(dir);
		exit(1);
	}
}

void create_file(char *filename)
{
	int fd;
	int randomsize;
	if ((fd = creat(filename, S_IRWXU)) < 0) {
		fprintf(stderr, "\nTotal create files: %d\n", filecount);
		perror(filename);
		exit(1);
	}
	if ((randomsize = box_muler(0, MAXFSIZE)) < 0) {
		randomsize = MAXFSIZE;
	}
	if (write(fd, wbuf, randomsize) < 0) {
		fprintf(stderr, "\nTotal create files: %d\n", filecount);
		perror(filename);
		exit(1);
	}
	filecount++;
	close(fd);
}
