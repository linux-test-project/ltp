#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/msg.h>

int main(int argc, char **argv)
{
	char ch;
	int key = 0x8888;
	int id;
	int perms = 0444;

	while ((ch = getopt(argc, argv, "k:p:")) != EOF) {
		switch (ch) {
		case 'k':
			key = atoi(optarg);
			break;
		case 'p':
			perms = atoi(optarg);
			break;
		}

	}

	id = msgget(key, IPC_CREAT|perms);
	if (id == -1)
	  return 1;
	printf("msgget succeeded: key = %d, id = %d\n", key, id);

	return 0;
}
