#include <stdio.h>
#include <errno.h>
#include <sys/capability.h>
#include <sys/prctl.h>

#ifndef PR_CAPBSET_READ
#define PR_CAPBSET_READ 23
#endif

int main(int argc, char *argv[])
{
	int i, ret = 0;

	for (i=0; ret != -1; i++) {
		ret = prctl(PR_CAPBSET_READ, i);
		if (ret == -1)
			break;
	}
	printf("#define NUM_CAPS %d\n", i);
	return 0;
}
