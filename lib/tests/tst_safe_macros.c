#include "test.h"
#include "safe_macros.h"

char *TCID = "test_safe_macros";
int TST_TOTAL = 1;

int fd = -1;

void cleanup(void)
{
	SAFE_CLOSE(NULL, fd);
	SAFE_UNLINK(NULL, __FILE__ "~");
	tst_resm(TINFO, "got here");
}

int main(int argc LTP_ATTRIBUTE_UNUSED, char **argv)
{
	char buf[10];
	int fds[2];

	buf[9] = '\0';

	if (system("cp " __FILE__ " " __FILE__ "~")) {
		fprintf(stderr, "error: could not cp file\n");
		return 1;
	}
	printf("%s\n", SAFE_BASENAME(NULL, *argv));
	printf("%s\n", SAFE_DIRNAME(NULL, *argv));
	fd = SAFE_OPEN(cleanup, __FILE__ "~", O_RDWR);
	SAFE_READ(cleanup, 0, fd, buf, 9);
	printf("buf: %s\n", buf);
	SAFE_READ(cleanup, 1, fd, buf, 9);
	printf("buf: %s\n", buf);
	SAFE_WRITE(cleanup, 0, -1, buf, 9);
	SAFE_WRITE(NULL, 0, fd, buf, 9);
	SAFE_WRITE(NULL, 1, fd, buf, 9);
	SAFE_PIPE(NULL, fds);

	return 0;
}
