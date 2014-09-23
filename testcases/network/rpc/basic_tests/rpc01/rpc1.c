#include <stdio.h>
#include <rpc/rpc.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "librpc01.h"

int program = 2000333;
int version = 10;
char *server = NULL;
char *file_name = NULL;
char host_name[100];
long host_address;

void do_compare(int, char *, struct data *, char *);
void usage_error(char *program_name);

int main(int argc, char *argv[])
{
	struct hostent *hp;
	struct data buffer, *return_buffer;
	int i, n, rc;
	FILE *fp;
	struct stat stat_buffer;
	char *p;
	CLIENT *clnt;
	struct sockaddr_in server_sin;
	int sock;
	struct timeval timeout;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-s")) {
			if (++i >= argc) {
				fprintf(stderr, "-s requires a host name\n");
				usage_error(argv[0]);
			}
			server = argv[i];
			continue;
		}

		if (!strcmp(argv[i], "-f")) {
			if (++i >= argc) {
				fprintf(stderr, "-h requires a file name\n");
				usage_error(argv[0]);
			}
			file_name = argv[i];
			continue;
		}

		if (!strcmp(argv[i], "-p")) {
			if (++i >= argc) {
				fprintf(stderr, "-p requires an argument\n");
				usage_error(argv[0]);
			}
			n = sscanf(argv[i], "%d", &program);
			if (n != 1) {
				fprintf(stderr,
					"-p requires an numeric argument\n");
				usage_error(argv[0]);
			}
			continue;
		}

		if (!strcmp(argv[i], "-v")) {
			if (++i >= argc) {
				fprintf(stderr, "-v requires an argument\n");
				usage_error(argv[0]);
			}
			n = sscanf(argv[i], "%d", &version);
			if (n != 1) {
				fprintf(stderr,
					"-v requires an numeric argument\n");
				usage_error(argv[0]);
			}
			continue;
		}
	}

	if (!server) {
		fprintf(stderr, "server not given\n");
		usage_error(argv[0]);
	}
	hp = gethostbyname(server);
	if (hp == NULL) {
		fprintf(stderr, "server %s unknown\n", server);
		usage_error(argv[0]);
	}
	memset(&server_sin, 0x00, sizeof(server_sin));
	server_sin.sin_family = AF_INET;
	memcpy(&server_sin.sin_addr, hp->h_addr, sizeof(hp->h_addr));

	if (!file_name) {
		fprintf(stderr, "file name not given\n");
		usage_error(argv[0]);
	}

	gethostname(host_name, 100);
	if ((hp = gethostbyname(host_name)) != NULL)
		host_address = (long)*((int *)hp->h_addr_list[0]);
	buffer.address = host_address;
	buffer.request_id = getpid();

	fp = fopen(file_name, "r");
	if (!fp) {
		fprintf(stderr, "Can not open %s\n", file_name);
		usage_error(argv[0]);
	}
	if (stat(file_name, &stat_buffer))
		fprintf(stderr, "stat() failed for %s, errno %d\n", file_name,
			errno);
	buffer.data_length = stat_buffer.st_size;
	if (buffer.data_length > 2187) {
		fprintf(stderr, "file too large (2187 max)\n");
		usage_error(argv[0]);
	}

	buffer.data = malloc(buffer.data_length);
	for (i = 0, p = buffer.data; i < buffer.data_length; i++, p++)
		*p = getc(fp);
	fclose(fp);

	rc = callrpc(server, program, version, 1, (xdrproc_t)xdr_send_data,
			(char *)&buffer, (xdrproc_t)xdr_receive_data,
			(char *)&return_buffer);
	do_compare(rc, "callrpc", &buffer, return_buffer->data);

	server_sin.sin_port = 0;
	sock = RPC_ANYSOCK;
	timeout.tv_usec = 0;
	timeout.tv_sec = 10;
	clnt = clntudp_create(&server_sin, program, version, timeout, &sock);
	if (clnt == NULL) {
		fprintf(stderr, "clntudp_create failed\n");
		exit(1);
	}
	timeout.tv_usec = 0;
	timeout.tv_sec = 30;
	rc = (int)clnt_call(clnt, 1, (xdrproc_t)xdr_send_data,
				(char *)&buffer, (xdrproc_t)xdr_receive_data,
				(char *)&return_buffer, timeout);
	clnt_destroy(clnt);
	do_compare(rc, "udp transport", &buffer, return_buffer->data);

	server_sin.sin_port = 0;
	sock = RPC_ANYSOCK;
	clnt = clnttcp_create(&server_sin, program, version, &sock, 0, 0);
	if (clnt == NULL) {
		fprintf(stderr, "clntudp_create failed\n");
		exit(1);
	}
	timeout.tv_usec = 0;
	timeout.tv_sec = 30;
	rc = (int)clnt_call(clnt, 1, (xdrproc_t)xdr_send_data,
				(char *)&buffer, (xdrproc_t)xdr_receive_data,
				(char *)&return_buffer, timeout);
	clnt_destroy(clnt);
	do_compare(rc, "tcp transport", &buffer, return_buffer->data);

	exit(0);
}

void do_compare(int rpc_rc, char *msg, struct data *buffer, char *ret_data)
{
	int rc;

	if (rpc_rc) {
		printf("RPC call with %s returned %d: ", msg, rpc_rc);
		clnt_perrno(rpc_rc);
		printf("\n");
		exit(1);
	}
	rc = memcmp(buffer->data, ret_data, buffer->data_length);
	if (rc) {
		printf("Data compare for %s returned %d\n", msg, rc);
		exit(1);
	}
}

void usage_error(char *program_name)
{
	fprintf(stderr,
		"Usage: %s -s server -f file [-p program-number] [-v version]\n",
		program_name);
	exit(2);
}
