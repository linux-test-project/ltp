#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/sem.h>

int main(int argc, char **argv)
{
	char ch;
	int num = 1;
	int key = 0x8888;
	int id;

	while ((ch = getopt(argc, argv, "k:-n:")) != EOF) {
		switch (ch) {
		case 'k':
			key = atoi(optarg);
			break;
		case 'n':
			num = atoi(optarg);
			break;
		}
	}

	id = semget(key, num, IPC_CREAT|0777);
	if (id == -1)
		return 1;
	printf("semget succeeded: key = %d, id = %d\n", key, id);

	return 0;
}
