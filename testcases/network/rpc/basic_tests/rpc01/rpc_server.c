#include <sys/ioctl.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rpc.h"
#include "librpc01.h"

int debug = 0;
int program = 2000333;
int version = 10;

void breakpoint(void);
void service_request(struct svc_req *rqstp, SVCXPRT * transp);

int main(int argc, char *argv[])
{
	SVCXPRT *transp;
	int i, n;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-d"))
			debug = 1;
		if (!strcmp(argv[i], "-p")) {
			if (++i >= argc) {
				fprintf(stderr, "%s: -p requires an argument\n",
					argv[0]);
				exit(1);
			}
			n = sscanf(argv[i], "%d", &program);
			if (n != 1) {
				fprintf(stderr,
					"%s: -p requires an numeric argument\n",
					argv[0]);
				exit(1);
			}
		}
		if (!strcmp(argv[i], "-v")) {
			if (++i >= argc) {
				fprintf(stderr, "%s: -v requires an argument\n",
					argv[0]);
				exit(1);
			}
			n = sscanf(argv[i], "%d", &version);
			if (n != 1) {
				fprintf(stderr,
					"%s: -v requires an numeric argument\n",
					argv[0]);
				exit(1);
			}
		}
	}

	if (!debug) {
		if ((n = fork()) < 0) {
			fprintf(stderr, "%s: Can't fork\n", argv[0]);
			exit(1);
		}

		if (n > 0)
			exit(0);

		i = 50;
		while (--i >= 0)
			close(i);
		open("/dev/null", O_RDONLY);
		open("/dev/null", O_WRONLY);
		i = dup(1);

		if ((i = open("/dev/tty", O_RDWR)) >= 0) {
			ioctl(i, TIOCNOTTY, 0);
			close(i);
		}
	}

	pmap_unset(program, version);
	transp = svcudp_create(RPC_ANYSOCK);
	svc_register(transp, program, version, service_request, IPPROTO_UDP);
	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	svc_register(transp, program, version, service_request, IPPROTO_TCP);
	svc_run();
	exit(1);
}

void service_request(struct svc_req *rqstp, SVCXPRT * transp)
{
	struct data *buffer;

	switch (rqstp->rq_proc) {
	case 0:
		svc_sendreply(transp, (xdrproc_t)xdr_void, NULL);
		breakpoint();
		return;

	case 99:
		exit(0);

	case 1:
		svc_getargs(transp, (xdrproc_t)xdr_receive_data,
				(char *)&buffer);
		svc_sendreply(transp, (xdrproc_t)xdr_send_data,
				(char *)buffer);
		free(buffer->data);
		free(buffer);
		return;

	default:
		svcerr_noproc(transp);
		return;
	}
}

void breakpoint(void)
{
	if (debug)
		printf("breakpoint\n");
}
