/*      -*- linux-c -*-
 *
 * Copyright (c) 2004 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Racing Guo <racing.guo@intel.com>
 * Changes:
 *	11.30.2004 - Kouzmich: porting to HPI-B
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "hpi_cmd.h"

int	debug_flag = 0;

int main(int argc, char **argv)
{
	int	c, eflag = 0;

	while ( (c = getopt( argc, argv,"c:ef:xn:?")) != EOF )
		switch(c)  {
			case 'c':
				setenv("OPENHPI_CONF", optarg, 1);
				break;
			case 'e':
				eflag = 1;
				break;
			case 'f':
				open_file(optarg);
				break;
			case 'x':
				debug_flag = 1;
				break;
            case 'n':
                setenv("OPENHPI_DAEMON_HOST", optarg, 1);
                break;
			default:
				printf("Usage: %s [-c <cfgfile>][-e][-f <file>][-n <hostname>]\n", argv[0]);
				printf("   -c <cfgfile> - use passed file as configuration file\n");
				printf("   -e - show short events, discover after subscribe\n");
				printf("   -f <file> - execute command file\n");
                printf("   -n <hostname>  use passed hostname as OpenHPI daemon host\n");
				return(1);
		}

	domainlist = (GSList *)NULL;
	if (open_session(eflag) == -1)
		return(1);
	cmd_shell();
	close_session();
	return 0;
}

ret_code_t ask_rpt(SaHpiResourceIdT *ret)
{
	term_def_t	*term;
	int		i, res;

	term = get_next_term();
	if (term == NULL) {
		if (read_file) return(HPI_SHELL_PARM_ERROR);
		i = show_rpt_list(Domain, SHOW_ALL_RPT, 0, SHORT_LSRES, ui_print);
		if (i == 0) {
			printf("NO rpts!\n");
			return(HPI_SHELL_CMD_ERROR);
		};
		i = get_int_param("RPT ID ==> ", &res);
		if (i == 1) *ret = (SaHpiResourceIdT)res;
		else return(HPI_SHELL_PARM_ERROR);
	} else {
		*ret = (SaHpiResourceIdT)atoi(term->term);
	};
	return(HPI_SHELL_OK);
}

ret_code_t ask_rdr(SaHpiResourceIdT rptid, SaHpiRdrTypeT type, SaHpiInstrumentIdT *ret)
{
	term_def_t	*term;
	int		i, res;
	char            buf[64];

	strncpy(buf, oh_lookup_rdrtype(type), 64);
	buf[strlen(buf)-4] = '\0';
	strncat(buf, " NUM ==> ", 64-strlen(buf));
	term = get_next_term();
	if (term == NULL) {
		if (read_file) return(HPI_SHELL_CMD_ERROR);
		i = show_rdr_list(Domain, rptid, type, ui_print);
		if (i == 0) {
			printf("No rdrs for rpt: %d\n", rptid);
			return(HPI_SHELL_CMD_ERROR);
		};
		i = get_int_param(buf, &res);
		if (i != 1) return(HPI_SHELL_PARM_ERROR);
		*ret = (SaHpiInstrumentIdT)res;
	} else {
		*ret = (SaHpiInstrumentIdT)atoi(term->term);
	};
	return(HPI_SHELL_OK);
}

ret_code_t open_file(char *path)
{
	if (add_input_file(path) != 0) {
		printf("Can not run file: %s\n", path);
		return(HPI_SHELL_PARM_ERROR);
	};
	read_file = 1;
	read_stdin = 0;
	return(HPI_SHELL_OK);
}
