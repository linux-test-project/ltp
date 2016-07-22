/* host - print information about a host
 * originally written by Paul Vixie @DEC WRL, January 1989
 */

/* DECWRL Header: host.c,v 1.1 89/04/05 15:41:12 vixie Locked $ */

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

#include <stdio.h>
#include <resolv.h>
#include <netdb.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>

#ifndef LOG_PERROR
#define LOG_PERROR 0
#endif

int main(argc, argv)
int argc;
char **argv;
{
	unsigned char b_addr[IN6ADDRSZ];
	struct hostent *host;
	char **ap, **cp, *arg;
	const char *prog = "amnesia";
	int af = AF_INET;
	int size = INADDRSZ;
	int force = 0;

	if (argc < 1) {
usage:
		printf("usage:  %s [-d] [-6] [-f] (hostname|ipaddr)\n", prog);
		exit(1);
	}
	prog = *argv++;
	argc--;
#ifdef LOG_USER
	openlog(prog, LOG_PERROR, LOG_USER);
#else
	openlog(prog, LOG_PERROR);
#endif
	res_init();

	if (argc >= 1 && !strcmp(*argv, "-d")) {
		_res.options |= RES_DEBUG;
		argv++, argc--;
	}
	if (argc >= 1 && !strcmp(*argv, "-6")) {
		af = AF_INET6, size = IN6ADDRSZ;
		_res.options |= RES_USE_INET6;
		argv++, argc--;
	}
	if (argc >= 1 && !strcmp(*argv, "-f")) {
		force++;
		argv++, argc--;
	}

	if (argc < 1)
		goto usage;
	arg = *argv++;
	argc--;

	if (inet_pton(af, arg, b_addr)) {
		char p[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"];

		printf("[%s]\n", inet_ntop(af, b_addr, p, sizeof p));
		if (!(host = gethostbyaddr((char *)b_addr, size, af))) {
			herror("gethostbyaddr");
			exit(1);
		}
	} else {
		printf("{%s}\n", arg);
		if (force)
			host = gethostbyname2(arg, af);
		else
			host = gethostbyname(arg);
		if (!host) {
			herror("gethostbyname*");
			exit(1);
		}
	}
	printf("name: %s\n", host->h_name);
	if (host->h_aliases && *host->h_aliases) {
		printf("aliases:");
		for (cp = (char **)host->h_aliases; *cp; cp++)
			printf(" %s", *cp);
		printf("\n");
	}
	if (host->h_addr_list && *host->h_addr_list) {
		printf("addresses:");
		for (ap = host->h_addr_list; *ap; ap++) {
			char p[sizeof
			       "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"];

			printf(" %s", inet_ntop(host->h_addrtype,
						*ap, p, sizeof p));
		}
		printf("\n");
	}
	exit(0);
}
