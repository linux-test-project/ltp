#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv) 
{
	FILE *fp;
	if (argc != 3) {
		fprintf(stderr, "usage:  %s path mode", argv[0]);
		exit(1);
	}
	fp = fopen(argv[1], argv[2]);
	if (!fp) {
		perror(argv[1]);
		exit(1);
	}
	fclose(fp);
	exit(0);
}
