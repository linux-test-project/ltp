#include <sys/ioctl.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <rpc/rpc.h>

int debug = 0;
int program = 2000333;
int version = 10;
char host_name[100];
long host_address;

struct data {
    long address;
    long request_id;
    long data_length;
    char *data;
};

void breakpoint(void);
void service_request(struct svc_req *rqstp, SVCXPRT *transp);
int xdr_receive_data(XDR *xdrs, struct data **buffer);
int xdr_send_data(XDR *xdrs, struct data *buffer);

int
main (int argc, char *argv[])
{
    SVCXPRT *transp;
    struct hostent *hp;
    int i, n;

    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "-d"))
	    debug = 1;
	if (!strcmp(argv[i], "-p")) {
	    if (++i >= argc) {
		fprintf(stderr, "%s: -p requires an argument\n", argv[0]);
		exit(1);
	    }
	    n = sscanf(argv[i], "%d", &program);
	    if (n != 1) {
		fprintf(stderr, "%s: -p requires an numeric argument\n",
			argv[0]);
		exit(1);
	    }
	}
	if (!strcmp(argv[i], "-v")) {
	    if (++i >= argc) {
		fprintf(stderr, "%s: -v requires an argument\n", argv[0]);
		exit(1);
	    }
	    n = sscanf(argv[i], "%d", &version);
	    if (n != 1) {
		fprintf(stderr, "%s: -v requires an numeric argument\n",
			argv[0]);
		exit(1);
	    }
	}
    }

    if (!debug) {
	if ((n=fork()) < 0) {
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
	dup(1);

	if ((i=open("/dev/tty", O_RDWR)) >= 0) {
	    ioctl(i, TIOCNOTTY, 0);
	    close(i);
	}
    }

    gethostname(host_name, 100);
    if (hp=gethostbyname(host_name))
	host_address = *((long *) hp->h_addr_list[0]);

    pmap_unset(program, version);
    transp = svcudp_create(RPC_ANYSOCK);
    svc_register(transp, program, version, service_request, IPPROTO_UDP);
    transp = svctcp_create(RPC_ANYSOCK, 0, 0);
    svc_register(transp, program, version, service_request, IPPROTO_TCP);
    svc_run();
    exit(1);
}

void
service_request(struct svc_req *rqstp, SVCXPRT *transp)
{
    struct data *buffer;

    switch (rqstp->rq_proc) {
    case 0:
	svc_sendreply(transp, xdr_void, (char *) 0);
	breakpoint();
	return;

    case 99:
	exit(0);

    case 1:
	svc_getargs(transp, xdr_receive_data, (unsigned char *) &buffer);
	svc_sendreply(transp, xdr_send_data, (unsigned char *) buffer);
	free(buffer->data);
	free(buffer);
	return;

    default:
	svcerr_noproc(transp);
	return;
    }
}

int
xdr_receive_data(XDR *xdrs, struct data **buffer)
{
    struct data *bp;
    int i, rc;
    char *p;

    bp = *buffer = (struct data *) malloc(sizeof(struct data));
    rc = xdr_long(xdrs, &(bp->address));
    rc = rc && xdr_long(xdrs, &bp->request_id);
    rc = rc && xdr_long(xdrs, &bp->data_length);
    p = (*buffer)->data = (char *) malloc(bp->data_length);
    for (i=0; rc && i < bp->data_length; p++, i++)
	rc = xdr_char(xdrs, p);
    return(rc);
}

int
xdr_send_data(XDR *xdrs, struct data *buffer)
{
    int i, rc;
    char *p;

    rc = xdr_long(xdrs, &buffer->address);
    rc = rc && xdr_long(xdrs, &buffer->request_id);
    rc = rc && xdr_long(xdrs, &buffer->data_length);
    for (i=0, p=buffer->data; rc && i < buffer->data_length; i++, p++)
	rc = xdr_char(xdrs, p);
    return(rc);
}

void breakpoint(void)
{
    if (debug)
	printf("breakpoint\n");
}