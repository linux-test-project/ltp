#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

static char *prog;
static int errors;

static int join_group(int, char *, struct ip_mreq *);
static int leave_group(int, char *, struct ip_mreq *);
static void usage(void);

int main(int argc, char *argv[])
{
	int s;
	struct ip_mreq imr;

	char *group_list = NULL, *interface = NULL;
	unsigned i1, i2, i3, i4;
	struct hostent *hp, *gethostbyname();
	int c;
	int lflg = 0, jflg = 0, sflg = 0;

	prog = argv[0];
	if (argc == 1)
		usage();

	while ((c = getopt(argc, argv, "jlg:s:i:")) != EOF)
		switch (c) {
		case 'j':
			if (lflg)
				usage();
			else
				jflg++;
			break;
		case 'l':
			if (jflg)
				usage();
			else
				lflg++;
			break;
		case 'g':
			group_list = optarg;
			break;
		case 's':
			sflg = atoi(optarg);
			break;
		case 'i':
			interface = optarg;
			break;
		case '?':
			usage();
		}

	if (optind != argc)
		usage();

	if (access(group_list, R_OK) != 0) {
		printf("Unabled to read group file %s\n", group_list);
		exit(1);
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		perror("can not open socket");
		exit(1);
	}

	hp = gethostbyname(interface);
	if (hp != NULL) {
		memcpy(&imr.imr_interface.s_addr, hp->h_addr, hp->h_length);
	} else if (sscanf(interface, "%u.%u.%u.%u", &i1, &i2, &i3, &i4) != 4) {
		fprintf(stderr, "bad group address\n");
		exit(1);
	} else {
		imr.imr_interface.s_addr =
		    htonl((i1 << 24) | (i2 << 16) | (i3 << 8) | i4);
	}
	/* verify socket options */
	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF,
		       &imr.imr_interface.s_addr,
		       sizeof(imr.imr_interface.s_addr)) != 0) {
		fprintf(stderr,
			"Error: unable to set socket option IP_MULTICAST_IF\n");
		errors++;
	} else
		printf("Socket set for Multicasting on: %s\n", interface);

	if ((!jflg && !lflg) || jflg)
		join_group(s, group_list, &imr);

	sleep(sflg);

	if ((!jflg && !lflg) || lflg)
		leave_group(s, group_list, &imr);

	close(s);
	if (errors)
		exit(1);
	return 0;
}

static int join_group(int s, char *glist, struct ip_mreq *imr)
{
	char buf[40];
	unsigned g1, g2, g3, g4;
	FILE *fd;
	char group[40], itf[40];

	fd = fopen(glist, "r");
	if (fd == NULL)
		printf("Error: unable to open %s\n", glist);

	while (fgets(buf, sizeof(buf), fd) != NULL) {
		if (sscanf(buf, "%u.%u.%u.%u", &g1, &g2, &g3, &g4) != 4) {
			fprintf(stderr, "bad group address\n");
			exit(1);
		}

		imr->imr_multiaddr.s_addr =
		    htonl((g1 << 24) | (g2 << 16) | (g3 << 8) | g4);

		if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			       imr, sizeof(struct ip_mreq)) == -1) {
			fprintf(stderr, "errno is %d\n", errno);
			perror("can't join group");
			errors++;
		} else {
			strcpy(group, inet_ntoa(imr->imr_multiaddr));
			strcpy(itf, inet_ntoa(imr->imr_interface));
			printf("IPM group: %s added to interface: %s\n", group,
			       itf);
		}
	}
	return 0;
}

static int leave_group(int s, char *glist, struct ip_mreq *imr)
{
	char buf[40];
	unsigned g1, g2, g3, g4;
	FILE *fd;
	char group[40], itf[40];

	fd = fopen(glist, "r");
	if (fd == NULL)
		printf("Error: unable to open %s\n", glist);

	while (fgets(buf, sizeof(buf), fd) != NULL) {
		if (sscanf(buf, "%u.%u.%u.%u", &g1, &g2, &g3, &g4) != 4) {
			fprintf(stderr, "leave_group: bad group address\n");
			exit(1);
		}

		imr->imr_multiaddr.s_addr =
		    htonl((g1 << 24) | (g2 << 16) | (g3 << 8) | g4);

		if (setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP,
			       imr, sizeof(struct ip_mreq)) == -1) {
			perror("can't leave group");
			errors++;
		} else {
			strcpy(group, inet_ntoa(imr->imr_multiaddr));
			strcpy(itf, inet_ntoa(imr->imr_interface));
			printf("IPM group: %s dropped from interface: %s\n",
			       group, itf);
		}
	}
	return 0;
}

static void usage(void)
{
	fprintf(stderr,
		"usage: %s [ -j -l ] -g group_list [-s time_to_sleep] -i interface_name (or i.i.i.i)\n",
		prog);
	exit(1);
}
