#include<stdio.h>

int main(void)
{
	FILE *fp;

	while (1) {
		fp = fopen("txt.x", "w");
		fclose(fp);
	}
	return 0;
}


