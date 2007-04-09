/*
 * ttutils.c
 *
 * dump tool test suite
 *
 * Copyright (C) 2005  NTT Data Corporation
 *
 * Author: Fernando Luis Vazquez Cao
 *
 * This program communicates with the DTT (Dump Tools Set) kernel module.
 *
 * 2005-01-20 Let's spin some drives
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Contact information:
 * Fernando Luis Vazquez Cao <fernando@intellilink.co.jp>
 * NTT Data Intellilink Corporation, Kayaba-cho Tower (2nd floor),
 * 1-21-2 Shinkawa, Chuo-ku, Tokyo 104-0033, Japan
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


/****************************************
 * Limits, default values and the like  *
 ***************************************/

#define NAME	"ttutils"
#define VERSION	"0.1"

#define NR_CMD_MAX	128
#define MAX_CMDLEN 50

#define MAX_CPOINT_NAME_LEN 50
#define MAX_CPOINT_NUM 300
#define MAX_CPOINT_ID MAX_CPOINT_NUM
#define CPOINT_ID_VOID -1

#define MAX_CTYPE_NAME_LEN 50

#define MAX_PASS_COUNT 100000

/* END Limits, default values and the like */


/*****************************************
 * Available commands and their handlers *
 ****************************************/

void usagefn(const int, char**);
void versionfn(const int, char**);
void listfn(const int, char**);
void addfn(const int, char**);
void rmvfn(const int, char**);
void setfn(const int, char**);
void resetfn(const int, char**);

void usage(const char *prog_name);
int add_cpoint(const char *cpoint_name, int id);
int rmv_cpoint(const char *cpoint_name);
int set_cpoint(const char *cpoint_name, const char *ctype_name, int cnt);
int reset_cpoint(const char *cpoint_name, int full);
void cpoint_list(void);

struct command {
	char *name;				/* command name */
	void (*func)(const int, char **);	/* associated function */
};

struct command cmd[NR_CMD_MAX] =
{
	[0] = {"help", &usagefn},
	[1] = {"ver", &versionfn},
	[2] = {"version", &versionfn},
	[3] = {"list", &listfn},
	[4] = {"ls", &listfn},
	[5] = {"set", &setfn},
	[6] = {"reset", &resetfn},
	[7] = {"add", &addfn},
	[8] = {"rmv", &rmvfn},
	[9] = {"", NULL},
};

/* END Available commands and their handlers */


/*****************************************
 * Error messages                        *
 *****************************************/

#define SUCCESS 0
#define EPARSE 1
#define ENOCP 2
#define ENOTYPE 3
#define EIDRANGE 4
#define ECNTRANGE 5
#define EPROC 6
#define EKERN 7

void cmdnotfound(const char *prog_name, const char *cmd_name) {
	printf("ERROR: Invalid command \"%s\"\n", cmd_name);
	usage(prog_name);
}

void print_ctrl_error(const char *cpoint_name, const char *ctype_name,
		      int cnt, int errcode) {
	switch (errcode) {
	case ENOENT:
		printf("ERROR: The crash point \"%s\" %s\n",
			cpoint_name,
			strcmp(ctype_name, "add") == 0 ?
				"was not found" : "is not registered"
			);
		break;
	case EINVAL:
		printf("ERROR: \"%s\" is not a valid crash dump trigger\n",
			ctype_name);
		break;
	case EDOM:
		printf("ERROR: \"%d\" is not a valid counter's value\n", cnt);
		break;
	case EEXIST:
		printf("ERROR: crash point's name/id pair \"%s/%d\" is a duplicate\n",
			cpoint_name, cnt);
		break;
	default:
		printf("ERROR: Unexpected error\n");
		printf("%5d: %s %s %d\n",
			errcode, cpoint_name, ctype_name, cnt);
		break;
	}
}

#define ENAMEMISSING	1
#define EIDMISSING	2
#define NOTID		3
#define NOTCNT		4

#define print_missing_name(name)\
	print_parsing_error(name, NULL, NULL, 0, ENAMEMISSING)

#define print_missing_id(name)\
	print_parsing_error(name, NULL, NULL, 0, EIDMISSING)

#define print_invalid_id() print_parsing_error(NULL, NULL, NULL, 0, NOTID)

#define print_invalid_cnt() print_parsing_error(NULL, NULL, NULL, 0, NOTCNT)

void print_parsing_error(const char *cmd, const char *cpoint_name,
			 const char *ctype_name, int cnt, int errcode) {
	switch (errcode) {
	case ENAMEMISSING:
		printf("ERROR: The command \"%s\" needs a valid -p option\n",
			cmd);
		break;
	case EIDMISSING:
		printf("ERROR: The command \"%s\" needs a valid -n option\n",
			cmd);
		break;
	case NOTID:
		printf("ERROR: Valid crash point IDs (\"-n\") are bigger than %d and smaller than %d\n", 0, MAX_CPOINT_ID);
		break;
	case NOTCNT:
		printf("ERROR: The counter (\"-c\") has to be bigger than %d and smaller than %d\n", 0, MAX_PASS_COUNT);
		break;
	}
}

#define perror_notloaded(what, cp_name)\
	printf("ERROR: Could not %s the crash point %s.\n", what, cp_name);\
	printf("Please verify that the dtt kernel module is loaded.\n")

/* END Error messages */


/****************************************
 * Main                                 *
 ****************************************/

int main(int argc, char **argv)
{
	int i;

	if (argc == 1) {
		usage(argv[0]);
		return SUCCESS;
	}

	opterr = 0;

	for (i = 0; (i < NR_CMD_MAX) && (cmd[i].func); i++) {
		if (!strcmp(cmd[i].name, argv[1])) {
			cmd[i].func(argc, argv);
			break;
		}
	}
	if (!cmd[i].func) {
		cmdnotfound(argv[0], argv[1]);
		return EPARSE;
	}

	return SUCCESS;
}

/* END Main */


/**********************************************************
 * Parsing the input and invoking the appropriate handler *
 **********************************************************/

void usage(const char *prog_name) {
	printf("Usage: %s {help|ver|version|list|ls|set|reset|add|rmv} [options]\n",
	prog_name);
}

void usagefn(const int argc, char **argv) {
	usage(argv[0]);
}

void versionfn(const int argc, char **argv) {
	printf("Version: %s\n", VERSION);
}

void addfn(const int argc, char **argv) {
	int c;
	int id = CPOINT_ID_VOID;
	char cpoint_name[MAX_CPOINT_NAME_LEN] = {0};
	char *cmds;

	cmds = argv[1];

	while ((c = getopt(argc, argv, "p:n:")) != EOF) {
		switch (c) {
		case 'p':
			strncpy(cpoint_name, optarg, MAX_CPOINT_NAME_LEN-1);
			break;
		case 'n':
			id = strtol(optarg, NULL, 0);
			if (id < 1 || id > MAX_CPOINT_NUM + 1) {
				print_invalid_id();
				exit(EIDRANGE);
			}
			break;
		}
	}
	if (cpoint_name[0] == 0) {
		print_missing_name(cmds);
		exit(EPARSE);
	}
	if (id == CPOINT_ID_VOID) {
		print_missing_id(cmds);
		exit(EPARSE);
	}

	add_cpoint(cpoint_name, id);
}

void rmvfn(const int argc, char **argv) {
	int c;
	char cpoint_name[MAX_CPOINT_NAME_LEN] = {0};
	char *cmds;

	cmds = argv[1];

	while ((c = getopt(argc, argv, "p:n:")) != EOF) {
		switch (c) {
		case 'p':
			strncpy(cpoint_name, optarg, MAX_CPOINT_NAME_LEN-1);
			break;
		}
	}
	if (cpoint_name[0] == 0) {
		print_missing_name(cmds);
		exit(EPARSE);
	}

	rmv_cpoint(cpoint_name);
}

void setfn(const int argc, char **argv) {
	int c;
	int id = CPOINT_ID_VOID;
	int cnt = -1;
	char cpoint_name[MAX_CPOINT_NAME_LEN] = {0};
	char ctype_name[MAX_CTYPE_NAME_LEN] = {0};
	char *cmds;

	cmds = argv[1];

	while ((c = getopt(argc, argv, "p:n:t:c:")) != EOF) {
		switch (c) {
		case 'p':
			strncpy(cpoint_name, optarg, MAX_CPOINT_NAME_LEN-1);
			break;
		case 'n':
			id = strtol(optarg, NULL, 0);
			if (id < 1 || id > MAX_CPOINT_NUM + 1) {
				print_invalid_id();
				exit(EIDRANGE);
			}
			break;
		case 't':
			strncpy(ctype_name, optarg, MAX_CPOINT_NAME_LEN-1);
			break;
		case 'c':
			cnt = strtol(optarg, NULL, 0);
			if (cnt < 1 || cnt > MAX_PASS_COUNT) {
				print_invalid_cnt();
				exit(ECNTRANGE);
			}
			break;
		}
	}
	if (cpoint_name[0] == 0) {
		print_missing_name(cmds);
		exit(EPARSE);
	}

	set_cpoint(cpoint_name, ctype_name, cnt);
}

void resetfn(const int argc, char **argv) {
	int c;
	int id = CPOINT_ID_VOID;
	int full = 0;
	char cpoint_name[MAX_CPOINT_NAME_LEN] = {0};
	char *cmds;
	
	cmds = argv[1];
	
	while ((c = getopt(argc, argv, "p:n:f")) != EOF) {
		switch (c) {
		case 'p':
			strncpy(cpoint_name, optarg, MAX_CPOINT_NAME_LEN-1);
			break;
		case 'n':
			id = strtol(optarg, NULL, 0);
			if (id < 1 || id > MAX_CPOINT_NUM + 1) {
				print_invalid_id();
				exit(EIDRANGE);
			}
			break;
		case 'f':
			full = 1;
			break;
		}
	}
	if (cpoint_name[0] == 0) {
		print_missing_name(cmds);
		exit(EPARSE);
	}

	reset_cpoint(cpoint_name, full);
}

void listfn(const int argc, char **argv) {
	cpoint_list();
}

/* END Parsing the input and invoking the appropriate handler */


/****************************************
 * Communicating with the kernel module *
 ****************************************/

#define CTRL_FILE "/proc/dtt/ctrl"
#define CPOINTS_FILE "/proc/dtt/cpoints"

int add_cpoint(const char *cpoint_name, int id) {
	int fd;
	int res;
	char buffer[MAX_CMDLEN];

	fd = open(CTRL_FILE, O_WRONLY);
	if (fd < 0) {
		perror_notloaded("add", cpoint_name);
		exit(EPROC);
	}

	snprintf(buffer, MAX_CMDLEN, "%s %d", cpoint_name, id);

	res = write (fd, buffer, MAX_CMDLEN);
	if (res < 0) {
		print_ctrl_error(cpoint_name, "add", id, errno);
		exit(EKERN);
	}
	
	return 0;
}

int rmv_cpoint(const char *cpoint_name) {
	int fd;
	int res;
	char buffer[MAX_CMDLEN];

	fd = open(CTRL_FILE, O_WRONLY);
	if (fd < 0) {
		perror_notloaded("remove", cpoint_name);
		exit(EPROC);
	}

	snprintf(buffer, MAX_CMDLEN, "%s 0", cpoint_name);

	res = write(fd, buffer, MAX_CMDLEN);
	if (res < 0) {
		print_ctrl_error(cpoint_name, "rmv", 0, errno);
		exit(EKERN);
	}

	return 0;
}

int set_cpoint(const char *cpoint_name, const char *ctype_name, int cnt) {
	int fd;
	int res;
	char buffer[MAX_CMDLEN];
	
	fd = open(CTRL_FILE, O_WRONLY);
	if (fd < 0) {
		perror_notloaded("configure", cpoint_name);
		exit(EPROC);
	}

	snprintf(buffer, MAX_CMDLEN, "%s %s %d",
		 cpoint_name, ctype_name[0] == 0 ? "same":ctype_name, cnt);
	
	res = write(fd, buffer, MAX_CMDLEN);
	if (res < 0) {
		print_ctrl_error(cpoint_name, ctype_name, cnt, errno);
		exit(EKERN);
	}
	
	return 0;
}

int reset_cpoint(const char *cpoint_name, int full) {
	int fd;
	int res;
	char buffer[MAX_CMDLEN];

	fd = open(CTRL_FILE, O_WRONLY);
	if (fd < 0) {
		perror_notloaded("reset", cpoint_name);
		exit(EPROC);
	}

	snprintf(buffer, MAX_CMDLEN, "%s %s 0",
			             cpoint_name, full ? "none":"same");
	
	res = write(fd, buffer, MAX_CMDLEN);
	if (res < 0) {
		print_ctrl_error(cpoint_name, "none", 0, errno);
		exit(EKERN);
	}
	
	return 0;
}

void cpoint_list(void) {
	FILE *cpoints;
	char buffer[MAX_CMDLEN];
	char id[MAX_CMDLEN], ct[MAX_CMDLEN], name[MAX_CMDLEN],
	     cnt[MAX_CMDLEN], loc[MAX_CMDLEN];

	cpoints = fopen(CPOINTS_FILE, "r");
	if (cpoints == NULL) {
		printf("ERROR: Could not get list of crash points.\n");
		printf("Please verify that the dtt kernel module is loaded.\n");
		exit(EPROC);
	}

	printf("id\tcrash type\tcrash point name\tcount\tlocation\n");
	
	while (fgets(buffer, MAX_CMDLEN, cpoints) != NULL) {
		sscanf(buffer, "%s %s %s %s %s", id, ct, name, cnt, loc);
		printf("%s\t%-15s\t%-20s\t%-5s\t%s\n", id, ct, name, cnt, loc);
	}
	
	return;
}

/* END Communicating with the kernel module */
