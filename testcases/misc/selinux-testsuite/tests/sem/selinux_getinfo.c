#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/sem.h>

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
	int val;                    /* value for SETVAL */
	struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
	unsigned short int *array;  /* array for GETALL, SETALL */
	struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif

int main(int argc, char **argv)
{
	char ch;
	int num = 1;
	int key = 0x8888;
	int id;
	int error;
	union semun arg;
	struct seminfo seminfo;

	arg.array = (ushort *)  &seminfo;
	error = semctl(0, 0, SEM_INFO, arg);
	printf ("semctl: SEM_INFO result = %d\n", error);

	return (error < 0);
}
