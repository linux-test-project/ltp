#include <selinux/selinux.h>

int main(int argc, char **argv) 
{
	int rc;

	rc = setcon(argv[1]);
	if (rc < 0) {
		perror("setcon");
		exit(1);
	}
	exit(0);
}

