
/*
 * Client for the send_file test program
 * Syntax: testsf_c <server IP addr> <port> <client_filename> <server_filename> <file-length>
 */

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include "test.h"
#include "netdefs.h"

int TST_TOTAL = 1;

#if INET6
char *TCID = "sendfile6_client";
#else
char *TCID = "sendfile_client";
#endif

int main(int argc, char *argv[])
{
	sai_t sai;
	int s, fd;
	int nbyte;
	char *serv_fname, *clnt_fname;
	char rbuf[PATH_MAX];
	int nlen, gai;
	struct addrinfo *hp;
	struct addrinfo hints;
	int port;

	if (argc != 6) {
		tst_brkm(TBROK,
			 NULL,
			 "usage: server-ip port client-file server-file file-len");
	}

	int i;
	for (i = 0; i < argc; i++)
		printf("i=%d: %s\n", i, *(argv + i));

	/* open socket to server */
	if ((s = socket(AFI, SOCK_STREAM, 0)) < 0) {
		tst_brkm(TBROK, NULL, "socket error = %d\n", errno);
	}

	clnt_fname = argv[3];	/* filename to create */
	serv_fname = argv[4];	/* filename to request */

	/* prepare to copy file from server to local machine */
	if ((fd = open(clnt_fname, O_CREAT | O_TRUNC | O_WRONLY, 0777)) < 0) {
		tst_resm(TBROK, "file open error = %d\n", errno);
		close(s);
		tst_exit();
	}

	/* initialize request info: */
	rbuf[0] = '\0';
	/* The request will be done in the form: `file-size=requested-file'. */
	snprintf(rbuf, PATH_MAX, "%s=%s", argv[5], serv_fname);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PFI;
	if ((gai = getaddrinfo(argv[1], NULL, &hints, &hp)) != 0) {
		tst_resm(TBROK, "Unknown subject address %s: %s\n",
			 argv[1], gai_strerror(gai));
	}
	if (!hp || !hp->ai_addr || hp->ai_addr->sa_family != AFI) {
		tst_brkm(TBROK, NULL, "getaddrinfo failed");
	}

	tst_resm(TINFO, "rbuf => %s\n", rbuf);
	/* initialize server info to make the connection */
	memcpy(&sai, hp->ai_addr, hp->ai_addrlen);
	port = atoi(argv[2]);

#if INET6
	sai.sin6_port = htons(port);
#else
	sai.sin_port = htons(port);
#endif

	if (connect(s, (sa_t *) & sai, sizeof(sai)) < 0) {
		tst_resm(TBROK, "connect error = %d\n", errno);
		close(s);
		exit(1);
	}

	/* send request info to server */
	if ((nbyte = write(s, rbuf, strlen(rbuf))) <= 0) {
		tst_resm(TBROK, "socket write  error = %d\n", errno);
		close(s);
		exit(1);
	}

	tst_resm(TINFO, "client write %d bytes to server with contents %s\n",
		 nbyte, rbuf);

	nlen = 0;		/* init size of info received */
	rbuf[0] = '\0';
	/* read until an EOF is encountered. */
	while ((nbyte = read(s, rbuf, PATH_MAX)) > 0) {
		nlen += nbyte;
		if (write(fd, rbuf, nbyte) != nbyte) {
			tst_brkm(TBROK, NULL,
				 "Error writing to file %s on client\n",
				 clnt_fname);
		}
	}

	tst_resm(TINFO, "Asking for remote file: %s", serv_fname);

	tst_resm(TINFO, "File %s received\n", argv[4]);

	close(s);
	close(fd);

	tst_exit();

}
