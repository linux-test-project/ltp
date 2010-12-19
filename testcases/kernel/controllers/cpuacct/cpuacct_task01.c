#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	FILE *fp;

	while (1) {
		fp = fopen("txt.x", "w");
		if (fp == NULL) {
			perror("fopen failed");
			exit(1);
		}
		fclose(fp);
	}
	return 0;
}