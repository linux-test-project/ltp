/*
 * Server for the sendfile test program
 * Syntax: testsf_s <own IP addr>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "test.h"
#include "netdefs.h"

int TST_TOTAL = 1;

#if INET6
char *TCID = "sendfile6_server";
#else
char *TCID = "sendfile_server";
#endif

int main(int argc, char *argv[])
{
	sai_t sa, *ap;
	sa_t from;
	struct addrinfo *hp;
	struct addrinfo hints;
	int as, fd, gai, rc, s;
	char *lp;
	char *number;
	int pid, nbytes, flen, count;
	char rbuf[PATH_MAX];
	int chunks = 0;
	off_t *offset;
	char nbuf[PATH_MAX];
	int port;

	if (argc != 3) {
		tst_brkm(TBROK, NULL, "usage: listen-address listen-port");
	}

	/* open socket */
	if ((s = socket(AFI, SOCK_STREAM, 0)) < 0) {
		tst_brkm(TBROK, NULL, "socket error = %d\n", errno);
	}

	signal(SIGCHLD, SIG_IGN);	/* ignore signals from children */

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PFI;
	if ((gai = getaddrinfo(argv[1], NULL, &hints, &hp)) != 0) {
		tst_brkm(TBROK, NULL, "getaddrinfo failed");
	}
	if (!hp || !hp->ai_addr || hp->ai_addr->sa_family != AFI) {
		tst_brkm(TBROK, NULL, "getaddrinfo failed");
	}

	/* server IP and port */
	memcpy(&sa, hp->ai_addr, hp->ai_addrlen);
	port = atoi(argv[2]);
#if INET6
	sa.sin6_port = htons(port);
#else
	sa.sin_port = htons(port);
#endif

	/* bind IP and port to socket */
	if (bind(s, (sa_t *) & sa, sizeof(sa)) < 0) {
		tst_resm(TBROK, "bind error = %d\n", errno);
		close(s);
		tst_exit();
	}

	/* start to listen socket */
	if (listen(s, LISTEN_BACKLOG) < 0) {
		tst_resm(TBROK, "listen error = %d\n", errno);
		close(s);
		tst_exit();
	}

	socklen_t fromlen = sizeof(from);

	/* process connections */
	while (1) {

		/* accept a connection from a client */
		if ((as = accept(s, &from, &fromlen)) < 0) {
			tst_resm(TBROK, "accept error = %d\n", errno);
			if (errno == EINTR)
				continue;
			close(s);
			tst_exit();
		}

		ap = (sai_t *) & from;

		/* create a process to manage the connection */
		if ((pid = fork()) < 0) {
			tst_resm(TBROK, "fork error = %d\n", errno);
			close(as);
			tst_exit();
		}
		if (pid > 0) {	/* parent, go back to accept */
			close(as);
			continue;
		}

		/* child process to manage a connection */

		close(s);	/* close service socket */

		/* get client request information */
		if ((nbytes = read(as, rbuf, PATH_MAX)) <= 0) {
			tst_resm(TBROK, "socket read error = %d\n", errno);
			close(as);
			tst_exit();
		}
		rbuf[nbytes] = '\0';	/* null terminate the info */
		lp = &rbuf[0];

		/* start with file length, '=' will start the filename */
		count = flen = 0;
		number = &nbuf[0];
		while (*lp != '=') {	/* convert ascii to integer */
			nbuf[count] = *lp;
			count++;
			lp++;
		}
		nbuf[count] = '\0';
		flen = strtol(number, NULL, 10);

		/* the file name */
		lp++;

		tst_resm(TINFO, "The file to send is %s\n", lp);
		/* open requested file to send */
		if ((fd = open(lp, O_RDONLY)) < 0) {
			tst_resm(TBROK, "file open error = %d\n", errno);
			close(as);
			tst_exit();
		}
		offset = NULL;
		errno = 0;
		do {		/* send file parts until EOF */
			if ((rc = sendfile(as, fd, offset, flen)) != flen) {
				if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
					tst_resm(TBROK,
						 "sendfile error = %d, rc = %d\n",
						 errno, rc);
					close(as);
					close(fd);
					tst_exit();
				}
			}
			chunks++;
		} while (rc != 0);
		tst_resm(TINFO, "File %s sent in %d parts\n", lp, chunks);

		close(as);	/* close connection */
		close(fd);	/* close requested file */

		exit(0);

	}

	close(s);		/* close parent socket (never reached because of the while (1)) */

	tst_exit();

}
