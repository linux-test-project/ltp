/*
@! # TEST TYPE(S): Concurrency, Load stress
@! # TESTCASE DESCRIPTION:
@! # 	Purpose: to send packets from the file to echo protocol on remote
@! #		 machine and read the echoing packets back and compare them
@! # 	Design: Connect to echo protocol on the remote machine
@! #		read from the file and send the file to remote machine
@! #		read the echoing  packets and store them in a file
@! #		repeat until file exhausted.
@! #		compare result
@! #
@! # SPEC. EXEC. REQS: May require multiple of this test to run
@! #		       to target machines from multiple machine in order
@! #		       to create stress condition
@! # 			echoes <REMOTE HOST> <echofile> <number of processes>
*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "test.h"
#include "netdefs.h"

#if INET6
char *TCID = "echoes6";
#else
char *TCID = "echoes";
#endif

int TST_TOTAL = 1;

void echofile(struct servent *, struct addrinfo *, char *, char *);
int checkfile(char *, char *);
void cleanup(int);

int main(int argc, char *argv[], char *env[])
{

	unsigned int finish, i, j, k;
	int gai, wait_stat;
	pid_t pid;
	struct addrinfo hints, *hp;
	struct servent *sp;
	struct {
		char resultfile[FILENAME_MAX + 1];
		pid_t pid;
	} echo_struc[200];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PFI;
	hints.ai_socktype = SOCK_STREAM;

	if (argc != 4)
		tst_brkm(TBROK, NULL, "usage: remote-addr file num-procs");

	if ((sp = getservbyname("echo", "tcp")) == NULL)
		tst_brkm(TBROK | TERRNO, NULL, "getservbyname failed");

	if ((gai = getaddrinfo(argv[1], NULL, &hints, &hp)) != 0)
		tst_brkm(TBROK, NULL, "unknown subject address %s: %s\n",
			 argv[1], gai_strerror(gai));

	if (!hp || !hp->ai_addr || hp->ai_addr->sa_family != AFI)
		tst_brkm(TBROK, NULL, "getaddrinfo failed");

	i = (unsigned int)strtol(argv[3], NULL, 10);
	j = 0;
	while (i-- > 0) {
		switch (pid = fork()) {
		case 0:
			snprintf(echo_struc[j].resultfile,
				 FILENAME_MAX, "%s%u", argv[2], j);
			echofile(sp, hp, echo_struc[j].resultfile, argv[2]);
			break;
		case -1:
			tst_resm(TBROK | TERRNO, "fork failed");
			break;
		default:
			echo_struc[j].pid = pid;
			j++;
			break;
		}
	}
	finish = (unsigned int)strtol(argv[3], NULL, 10);
	i = finish;
	/* Consume all operating threads until we're done... */
	while (finish != 0) {

		if ((pid = wait(&wait_stat)) == -1)
			tst_resm(TFAIL | TERRNO, "wait failed");
		if (wait_stat == 0) {
			for (j = 0; j < i; j++) {
				if (echo_struc[j].pid == pid) {
					finish--;
					j = i;
				}
			}
		} else {

			tst_resm(TFAIL, "wait(2) status was non-zero");

			if (WIFEXITED(wait_stat)) {
				tst_resm(TINFO, "exit status: %d",
					 WEXITSTATUS(wait_stat));
			} else if (WIFSIGNALED(wait_stat)) {
				tst_resm(TINFO, "signaled: %d",
					 WTERMSIG(wait_stat));
			}

			for (k = 0; k < i; k++) {
				if (kill(echo_struc[k].pid, 0) == 0) {
					kill(echo_struc[k].pid, 9);
				}
			}
			break;
		}

	}

	tst_exit();
}

/* XXX (garrcoop): This shouldn't use libltp as it's a forked process. */
void
echofile(struct servent *sp, struct addrinfo *ai, char *resultfile,
	 char *srcfile)
{
	int n;
	int port;
	char wr_buffer[BUFSIZ];
	char rd_buffer[BUFSIZ];
	sai_t sa;
#ifdef DEBUG
	sa_t address;
	socklen_t addrlen;
#endif
	int s;
	int finish;
	int fdw, fdr;
	int nread, nwrite;
	int count;
	pid_t pid;

#ifdef 	DEBUG
	printf("Creating socket .....\n");
#endif

	pid = getpid();
	if ((s = socket(AFI, SOCK_STREAM, 0)) < 0) {
		tst_resm(TBROK, "Failed to create listener socket (pid=%d)",
			 pid);
		cleanup(s);
		tst_exit();
	}
	port = sp->s_port;

	/*
	 * TODO: Old code did something of the form:
	 *
	 * struct hostent *hp;
	 *
	 * hp = gethostbyname(argv[1]);
	 *
	 * ...
	 *
	 * struct       in_addr hostaddr;
	 *
	 * memcpy(&hostaddr,hp->h_addr_list[0],sizeof(struct in_addr));
	 *
	 * This is all fine and dandy, but gethostbyname has been deprecated
	 * for some time, and doesn't work too well with IPV6 (from what I've
	 * read), so I have to push it over to getaddrinfo. getaddrinfo isn't
	 * a 1:1 mapping though, so I have to do some work to shoehorn the old
	 * code to fit the new code.
	 *
	 * Some notes (from a test app)...
	 *
	 * (gdb) set args 127.0.0.1
	 * (gdb) list
	 * 33              for (int i = 1; i < argc; i++) {
	 * 34
	 * 35                      gai = getaddrinfo(argv[i], NULL, &hints, &ai);
	 * 36                      hp = gethostbyname(argv[i]);
	 * 37
	 * 38                      if (gai != 0) {
	 * 39                              printf("Error: %s\n", gai_strerror(gai));
	 * 40                              error = 2;
	 * 41                      } else {
	 * 42                              printf("Host IP: 0x%x\n", ai->ai_addr);
	 * (gdb) p *hp
	 * $16 = {h_name = 0x1a60198 "127.0.0.1", h_aliases = 0x1a60190, h_addrtype = 2,
	 *   h_length = 4, h_addr_list = 0x1a60180}
	 * (gdb) p *hp->h_addr_list
	 * $14 = 0x1a60170 "\177"
	 * (gdb) p *ai
	 * $15 = {ai_flags = 0, ai_family = 2, ai_socktype = 1, ai_protocol = 6,
	 *   ai_addrlen = 16, ai_addr = 0x1a600b0, ai_canonname = 0x0,
	 *     ai_next = 0x1a600d0}
	 *
	 * If one continues down this path, SIGPIPE will get tossed at the first
	 * write(2), as opposed to Connection refused (the old code). So I'm not
	 * passing in the correct info to connect(2).
	 *
	 * That and using -DDEBUG with the getpeername(3) call below always fails
	 * (that alone should be a sufficient to note that my sockaddr* data is
	 * skewed).
	 *
	 * For now let's just mark it broken.
	 *
	 */
	//tst_resm(TBROK, "FIX ME GARRETT!");
	//tst_exit();

	memset((char *)&sa, 0, sizeof(sa));
	memcpy(&sa, ai->ai_addr, ai->ai_addrlen);

#if INET6
	sa.sin6_port = port;
#else
	sa.sin_port = port;
#endif

	if (connect(s, (sa_t *) & sa, sizeof(sa)) == -1) {
		tst_resm(TBROK | TERRNO,
			 "failed to create connector socket (pid=%d)", pid);
		cleanup(s);
		tst_exit();
	}
#ifdef DEBUG
	addrlen = sizeof(struct sockaddr);
	/* printf("addrlen=%d\n", addrlen); */
	/* printf("ai->ai_addr=%s\n", inet_ntoa(ai->ai_addr)); */
	if (getsockname(s, &address, &addrlen) == -1) {
		tst_resm(TBROK | TERRNO, "getsockname call failed (pid=%d)",
			 pid);
		cleanup(s);
		tst_exit();
	}

	printf("local port is: %d\n", port);

	if (getpeername(s, &address, &addrlen) == -1) {
		tst_resm(TBROK | TERRNO, "getpeername call failed (pid=%d)",
			 pid);
		cleanup(s);
		tst_exit();
	}

	tst_resm(TINFO, "The remote port is: %d\n", port);
#endif
	if ((fdr = open(srcfile, O_RDONLY)) < 0) {
		tst_resm(TBROK | TERRNO,
			 "failed to open input file (pid=%d)", pid);
		cleanup(s);
		tst_exit();
	}

	if ((fdw = creat(resultfile, 0644)) < 0) {
		tst_resm(TBROK | TERRNO,
			 "failed to create a temporary file (pid=%d)", pid);
		cleanup(s);
		tst_exit();
	}
#if DEBUG
	tst_resm(TINFO, "creat(resultfile,...) done.");
#endif
	finish = FALSE;
	count = 0;
	while (finish == FALSE) {

		if ((nwrite = read(fdr, wr_buffer, BUFSIZ)) == -1) {
			tst_resm(TFAIL | TERRNO,
				 "failed to read from file (pid=%d)", pid);
			cleanup(s);
		}
#if DEBUG
		tst_resm(TINFO, "Read %d bytes from file", nwrite);
#endif
		if (nwrite == 0)
			finish = TRUE;
		else {
			count++;
			if ((n = write(s, wr_buffer, nwrite)) != nwrite) {
				tst_resm(TFAIL | TERRNO,
					 "failed to write to socket (pid=%d)",
					 pid);
				cleanup(s);
			}
#ifdef 	DEBUG
			tst_resm(TINFO, "Writing %d bytes to remote socket",
				 count);
#endif
			while (nwrite != 0) {

				nread = read(s, rd_buffer, BUFSIZ);
				if (nread == -1) {
					printf("read size: %d\n", n);
					tst_resm(TFAIL | TERRNO,
						 "failed to read from socket [2nd "
						 "time] (pid=%d)", pid);
					cleanup(s);
				}
#ifdef 	DEBUG
				printf("Reading ....... %d\n", count);
#endif
				n = write(fdw, rd_buffer, nread);
				if (n != nread) {
					tst_resm(TFAIL | TERRNO,
						 "ERROR during write to result "
						 "file (pid=%d); read amount: %d",
						 pid, n);
					cleanup(s);
				}

				nwrite -= nread;

			}

		}		/* end of else */

	}			/* end of while */

	if ((n = close(s)) == -1) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "failed to cleanly close socket (pid=%d)", pid);
	}
	if ((n = close(fdr)) == -1) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "failed to cleanly close input file (pid=%d)", pid);
	}
	if ((n = close(fdw)) == -1) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "failed to cleanly close temp file (pid=%d)", pid);
	}
	if (checkfile(srcfile, resultfile) != TRUE) {
		tst_brkm(TFAIL, NULL,
			 "Input file and output file are not equal (pid=%d)",
			 pid);
	}
	tst_resm(TINFO, "Finish .... (pid=%d)", pid);
	tst_exit();
}

int checkfile(char *file1, char *file2)
{
	off_t n;
	struct stat buffer;
	stat(file1, &buffer);
	n = buffer.st_size;
#ifdef 	DEBUG
	printf("%s size=%lu\n", file1, n);
#endif
	stat(file2, &buffer);
#ifdef 	DEBUG
	printf("%s size=%lu\n", file2, buffer.st_size);
#endif
	if (n != buffer.st_size)
		return FALSE;
	else
		return TRUE;
}

void cleanup(int s)
{
	close(s);
}
