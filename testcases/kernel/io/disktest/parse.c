/*
* Disktest
* Copyright (c) International Business Machines Corp., 2001
*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Please send e-mail to yardleyb@us.ibm.com if you have
*  questions or comments.
*
*  Project Website:  TBD
*
* $Id: parse.c,v 1.8 2009/02/26 12:02:23 subrata_modak Exp $
*
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "globals.h"
#include "threading.h"
#include "main.h"
#include "usage.h"
#include "sfunc.h"
#include "parse.h"

int fill_cld_args(int argc, char **argv, child_args_t * args)
{
	extern char *optarg;
	extern int optind;
	extern unsigned long glb_flags;

	signed char c;
	char *leftovers;

	while ((c =
		getopt(argc, argv,
		       "?a:A:B:cC:dD:E:f:Fh:I:K:L:m:M:nN:o:p:P:qQrR:s:S:t:T:wvV:z"))
	       != -1) {
		switch (c) {
		case ':':
			pMsg(WARN, args, "Missing argument for perameter.\n");
			usage();
			return (-1);
		case 'V':
#ifdef _DEBUG
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				exit(1);
			}
			if (!isdigit(optarg[0])) {
				pMsg(WARN, args,
				     "-%c argument is non numeric.\n", c);
				exit(1);
			}
			gbl_dbg_lvl = atoi(optarg);
#else
			pMsg(ERR, args,
			     "Debug code not compiled in, recompile with _DEBUG directive.\n",
			     c);
			exit(1);
#endif
			break;
		case 'd':
			glb_flags |= GLB_FLG_QUIET;
			args->flags |= CLD_FLG_DUMP;
			break;
		case 'a':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (!isdigit(optarg[0])) {
				pMsg(WARN, args,
				     "-%c arguments is non numeric.\n", c);
				return (-1);
			}
			args->seed = (unsigned int)strtol(optarg, NULL, 0);
			break;
		case 'A':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				exit(1);
			}
			if (strchr(optarg, 'g')) {
				glb_flags |= GLB_FLG_KILL;
			}
			if (strchr(optarg, 'c')) {
				args->flags &= ~CLD_FLG_ALLDIE;
			}
			if (strchr(optarg, 'm')) {
				args->flags |= CLD_FLG_ERR_MARK;
			}
			if (strchr(optarg, 'r')) {
				args->flags &= ~CLD_FLG_ERR_REREAD;
			}
			if (strchr(optarg, 's')) {
				args->flags &= ~CLD_FLG_LBA_SYNC;
			}
			if (strchr(optarg, 'S')) {
				args->flags |= CLD_FLG_IO_SERIAL;
			}
			if (strchr(optarg, 'w')) {
				args->flags |= CLD_FLG_WRITE_ONCE;
			}
			if (strchr(optarg, 'W')) {
				args->flags |= CLD_FLG_UNIQ_WRT;
			}
			if (strchr(optarg, 't')) {
				args->flags |= CLD_FLG_TMO_ERROR;
			}
			break;
		case 'q':
			glb_flags |= GLB_FLG_QUIET;
			break;
		case 'Q':
			glb_flags |= GLB_FLG_SUPRESS;
			break;
		case 'v':
			pMsg(INFO, args, "Version %s\n", VER_STR);
			exit(0);
		case 'p':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (args->flags & (CLD_FLG_LINEAR | CLD_FLG_RANDOM)) {
				pMsg(WARN, args,
				     "Only one seek type, -p, can be specified.\n");
				return (-1);
			}
			/* seek pattern type */
			if (strchr(optarg, 'L'))
				args->flags |= CLD_FLG_LINEAR;
			else if (strchr(optarg, 'l'))
				args->flags |=
				    (CLD_FLG_LINEAR | CLD_FLG_NTRLVD);
			else if (strchr(optarg, 'R'))
				args->flags |= CLD_FLG_RANDOM;
			else if (strchr(optarg, 'r'))
				args->flags |=
				    (CLD_FLG_RANDOM | CLD_FLG_NTRLVD);
			else {
				pMsg(WARN, args, "Unknown Seek pattern\n");
				usage();
				return (-1);
			}
			if (strchr(optarg, 'U') || strchr(optarg, 'u'))
				if ((args->flags & (CLD_FLG_LINEAR)) &&
				    !(args->flags & CLD_FLG_LUND))
					args->flags |= CLD_FLG_LUNU;
			if (strchr(optarg, 'D') || strchr(optarg, 'd'))
				if ((args->flags & (CLD_FLG_LINEAR)) &&
				    !(args->flags & CLD_FLG_LUNU))
					args->flags |= CLD_FLG_LUND;
			break;
		case 'B':
			if (!isdigit(optarg[0])) {
				pMsg(WARN, args,
				     "-%c arguments is non numeric.\n", c);
				return (-1);
			}
			if (strchr(optarg, ':') != NULL) {	/* we are given a range of transfer sizes */
				args->flags |= CLD_FLG_RTRSIZ;
				args->ltrsiz = strtoul(optarg, &leftovers, 10);
				if (leftovers == strchr(leftovers, 'k')) {	/* first value had a 'k' */
					args->ltrsiz *= 2;
					leftovers++;
				} else if (leftovers == strchr(leftovers, 'm')) {	/* first value had a 'm' */
					args->ltrsiz *= (2 * 1024);
					leftovers++;
				} else {
					if (args->ltrsiz > 256)
						args->ltrsiz /= BLK_SIZE;
				}
				if (!isdigit(leftovers[1])) {
					pMsg(WARN, args,
					     "-%c arguments is non numeric.\n",
					     c);
					return (-1);
				}
				args->htrsiz =
				    atol((char *)strchr(leftovers, ':') + 1);
				if ((strchr(leftovers, 'k')) != NULL) {	/* second value had a 'k' */
					args->htrsiz *= 2;
				} else if ((strchr(leftovers, 'm')) != NULL) {	/* second value had a 'm' */
					args->htrsiz *= (2 * 1024);
				} else {
					if (args->htrsiz > 256)
						args->htrsiz /= BLK_SIZE;
				}
			} else {	/* only a single value given for transfer size */
				args->ltrsiz = atoi(optarg);
				if (strchr(optarg, 'k')) {
					args->ltrsiz *= 2;
				} else if (strchr(optarg, 'm')) {
					args->ltrsiz *= (2 * 1024);
				} else {
					if (args->ltrsiz > 256)
						args->ltrsiz /= BLK_SIZE;
				}
				args->htrsiz = args->ltrsiz;
			}
#ifdef _DEBUG
			PDBG5(DBUG, args, "Parsed Transfer size: %ld\n",
			      args->htrsiz);
#endif
			break;
		case 'c':
			if (args->flags & CLD_FLG_PTYPS) {
				pMsg(WARN, args,
				     "Please specify only one pattern type\n");
				usage();
				return (-1);
			}
			args->flags |= CLD_FLG_CPTYPE;
			break;
		case 'n':
			if (args->flags & CLD_FLG_PTYPS) {
				pMsg(WARN, args,
				     "Please specify only one pattern type\n");
				usage();
				return (-1);
			}
			args->flags |= CLD_FLG_LPTYPE;
			break;
		case 'f':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (args->flags & CLD_FLG_PTYPS) {
				pMsg(WARN, args,
				     "Please specify only one pattern type\n");
				usage();
				return (-1);
			}
			args->pattern = my_strtofft(optarg);
			args->flags |= CLD_FLG_FPTYPE;
			break;
		case 'F':
			/* the filespec is a list of filespecs in a file */
			args->flags |= CLD_FLG_FSLIST;
			break;
		case 'z':
			if (args->flags & CLD_FLG_PTYPS) {
				pMsg(WARN, args,
				     "Please specify only one pattern type\n");
				usage();
				return (-1);
			}
			args->flags |= CLD_FLG_RPTYPE;
			break;
		case 'h':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (!isdigit(optarg[0])) {
				pMsg(WARN, args,
				     "-%c arguments is non numeric.\n", c);
				usage();
				return (-1);
			}
			args->flags |= CLD_FLG_HBEAT;
			args->hbeat = atoi(optarg);
			if (strchr(optarg, 'm')) {	/* multiply by sec */
				args->hbeat *= 60;
			} else if (strchr(optarg, 'h')) {	/* multiply sec*min */
				args->hbeat *= (time_t) (60 * 60);
			} else if (strchr(optarg, 'd')) {	/* multiply by sec*min*hours */
				args->hbeat *= (time_t) (60 * 60 * 24);
			}
			break;
		case 'D':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (!isdigit(optarg[0])) {
				pMsg(WARN, args,
				     "-%c arguments is non numeric.\n", c);
				usage();
				return (-1);
			}
			args->rperc = atoi(optarg);
			args->wperc = atoi((char *)(strchr(optarg, ':') + 1));
			args->flags |= CLD_FLG_DUTY;
			break;
		case 'r':
			args->flags |= CLD_FLG_R;
			break;
		case 'w':
			args->flags |= CLD_FLG_W;
			break;
		case 'o':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			args->offset = atol(optarg);
			args->flags |= CLD_FLG_OFFSET;
			break;
		case 'R':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (strchr(optarg, ':') != NULL) {	/* we are given a retry delay */
				args->retries = strtol(optarg, &leftovers, 10);
				args->retry_delay =
				    (time_t) atol((char *)strchr(leftovers, ':')
						  + 1);
			} else {	/* only a retry count given */
				args->retries = atoi(optarg);
			}
			break;
		case 'M':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			args->flags |= CLD_FLG_ALT_MARK;
			args->alt_mark = my_strtofft(optarg);
			break;
		case 'm':
			args->flags |= CLD_FLG_MBLK;
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (strchr(optarg, 'l')) {	/* returns NULL if char is not found */
				args->flags |= CLD_FLG_MRK_LBA;
			}
			if (strchr(optarg, 'p')) {
				args->flags |= CLD_FLG_MRK_PASS;
			}
			if (strchr(optarg, 't')) {
				args->flags |= CLD_FLG_MRK_TIME;
			}
			if (strchr(optarg, 's')) {
				args->flags |= CLD_FLG_MRK_SEED;
			}
			if (strchr(optarg, 'h')) {
				args->flags |= CLD_FLG_MRK_HOST;
			}
			if (strchr(optarg, 'f')) {
				args->flags |= CLD_FLG_MRK_TARGET;
			}
			if (strchr(optarg, 'a')) {
				args->flags |= CLD_FLG_MRK_ALL;
			}
			if (!strchr(optarg, 'l') &&
			    !strchr(optarg, 'p') &&
			    !strchr(optarg, 't') &&
			    !strchr(optarg, 's') &&
			    !strchr(optarg, 'h') &&
			    !strchr(optarg, 'f') && !strchr(optarg, 'a')) {
				pMsg(WARN, args,
				     "Unknown header mark option\n");
				return (-1);
			}
			break;
		case 'E':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (!isdigit(optarg[0])) {
				pMsg(WARN, args,
				     "-%c arguments are non numeric.\n", c);
				usage();
				return (-1);
			}
			args->flags |= CLD_FLG_CMPR;
			args->cmp_lng = strtol(optarg, NULL, 0);
			if (strchr(optarg, 'k')) {	/* multiply by 2^10 */
				args->cmp_lng <<= 10;
			} else if (strchr(optarg, 'K')) {	/* multiply 10^3 */
				args->cmp_lng *= 1000;
			} else if (strchr(optarg, 'm')) {	/* multiply by 2^20 */
				args->cmp_lng <<= 20;
			} else if (strchr(optarg, 'M')) {	/* multiply by 10^6 */
				args->cmp_lng *= 1000000;
			}
			break;
		case 'N':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (!isdigit(optarg[0])) {
				pMsg(WARN, args,
				     "-%c arguments are non numeric.\n", c);
				return (-1);
			}
			args->flags |= CLD_FLG_VSIZ;
			args->vsiz = my_strtofft(optarg);
			if (strchr(optarg, 'k')) {	/* multiply by 2^10 */
				args->vsiz <<= 10;
			} else if (strchr(optarg, 'K')) {	/* multiply 10^3 */
				args->vsiz *= 1000;
			} else if (strchr(optarg, 'm')) {	/* multiply by 2^20 */
				args->vsiz <<= 20;
			} else if (strchr(optarg, 'M')) {	/* multiply by 10^6 */
				args->vsiz *= 1000000;
			} else if (strchr(optarg, 'g')) {	/* multiply by 2^30 */
				args->vsiz <<= 30;
			} else if (strchr(optarg, 'G')) {	/* multiply by 10^9 */
				args->vsiz *= 1000000000;
			}
			break;
		case 'I':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (strchr(optarg, 'R') || strchr(optarg, 'r')) {
				if (!(args->flags & CLD_FLG_BLK) &&
				    !(args->flags & CLD_FLG_FILE)) {
					args->flags |= CLD_FLG_RAW;
				} else {
					pMsg(WARN, args,
					     "Can only specify one IO type\n");
					return (-1);
				}
			}
			if (strchr(optarg, 'B') || strchr(optarg, 'b')) {
				if (!(args->flags & CLD_FLG_RAW) &&
				    !(args->flags & CLD_FLG_FILE)) {
					args->flags |= CLD_FLG_BLK;
				} else {
					pMsg(WARN, args,
					     "Can only specify one IO type\n");
					return (-1);
				}
			}
			if (strchr(optarg, 'F') || strchr(optarg, 'f')) {
				if (!(args->flags & CLD_FLG_RAW) &&
				    !(args->flags & CLD_FLG_BLK)) {
					args->flags |= CLD_FLG_FILE;
				} else {
					pMsg(WARN, args,
					     "Can only specify one IO type\n");
					return (-1);
				}
			}
			if (strchr(optarg, 'D') || strchr(optarg, 'd')) {
				args->flags |= CLD_FLG_DIRECT;
			}
			if (strchr(optarg, 's')) {
				args->sync_interval =
				    strtoul((char *)strchr(optarg, 's') + 1,
					    NULL, 10);
#ifdef _DEBUG
				PDBG3(DBUG, args, "Parsed sync interval: %ld\n",
				      args->sync_interval);
#endif
				if ((args->flags & CLD_FLG_DIRECT)) {
					pMsg(ERR, args,
					     "Can't specify sync with Direct IO\n");
					return (-1);
				}
				args->flags |= CLD_FLG_WFSYNC;
			}
			break;
		case 't':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}

			if (strchr(optarg, ':') != NULL) {	/* we are given a option for delay & timeout */
				args->delayTimeMin =
				    strtoul(optarg, &leftovers, 10);
				/* check to see if we have one or more then one ':' */
				if ((char *)strchr(optarg, ':') ==
				    (char *)strrchr(optarg, ':')) {
					/* only one ':', assume no random delayTime, and ioTimeout */
					args->delayTimeMax = args->delayTimeMin;
					args->ioTimeout =
					    (time_t) atol((char *)
							  strchr(leftovers,
								 ':') + 1);
				} else {
					/* more then one ':', assume random delayTime, and ioTimeout */
					args->delayTimeMax =
					    strtoul(leftovers + 1, &leftovers,
						    10);
					args->ioTimeout =
					    (time_t) atol((char *)
							  strchr(leftovers,
								 ':') + 1);
				}
				if (strchr(leftovers, 'm')) {	/* multiply by sec */
					args->ioTimeout *= 60;
				} else if (strchr(leftovers, 'h')) {	/* multiply sec*min */
					args->ioTimeout *= (time_t) (60 * 60);
				} else if (strchr(leftovers, 'd')) {	/* multiply by sec*min*hours */
					args->ioTimeout *=
					    (time_t) (60 * 60 * 24);
				}
			} else {
				args->delayTimeMin =
				    strtoul(optarg, NULL, 10);
				args->delayTimeMax = args->delayTimeMin;
			}
			break;
		case 'T':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			args->run_time = atoi(optarg);
			args->flags |= CLD_FLG_TMD;
			if (strchr(optarg, 'm')) {	/* multiply by sec */
				args->run_time *= 60;
			} else if (strchr(optarg, 'h')) {	/* multiply sec*min */
				args->run_time *= (time_t) (60 * 60);
			} else if (strchr(optarg, 'd')) {	/* multiply by sec*min*hours */
				args->run_time *= (time_t) (60 * 60 * 24);
			}
			break;
		case 'L':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			args->seeks = atoi(optarg);
			args->flags |= CLD_FLG_SKS;
			if (strchr(optarg, 'k')) {	/* multiply by 2^10 */
				args->seeks <<= 10;
			} else if (strchr(optarg, 'K')) {	/* multiply 10^3 */
				args->seeks *= 1000;
			} else if (strchr(optarg, 'm')) {	/* multiply by 2^20 */
				args->seeks <<= 20;
			} else if (strchr(optarg, 'M')) {	/* multiply by 10^6 */
				args->seeks *= 1000000;
			} else if (strchr(optarg, 'g')) {	/* multiply by 2^30 */
				args->seeks <<= 30;
			} else if (strchr(optarg, 'G')) {	/* multiply by 10^9 */
				args->seeks *= 1000000000;
			}
			break;
		case 'C':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (!isdigit(optarg[0])) {
				pMsg(WARN, args,
				     "-%c arguments is non numeric.\n", c);
				usage();
				return (-1);
			}
			args->flags |= CLD_FLG_CYC;
			args->cycles = atol(optarg);
			break;
		case 'K':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (!isdigit(optarg[0])) {
				pMsg(WARN, args,
				     "-%c arguments is non numeric.\n", c);
				usage();
				return (-1);
			}
			if (atoi(optarg) > MAX_THREADS) {
				pMsg(WARN, args,
				     "%u exceeds max of %u threads.\n",
				     atoi(optarg), MAX_THREADS);
				return (-1);
			}
			args->t_kids = atoi(optarg);
			break;
		case 'P':
			if (optarg == NULL) {
				pMsg(WARN, args,
				     "-%c option requires an argument.\n", c);
				return (-1);
			}
			if (strchr(optarg, 'X')) {	/* returns NULL if char is not found */
				args->flags |= CLD_FLG_XFERS;
			}
			if (strchr(optarg, 'T')) {
				args->flags |= CLD_FLG_TPUTS;
			}
			if (strchr(optarg, 'P')) {
				glb_flags |= GLB_FLG_PERFP;
			}
			if (strchr(optarg, 'R')) {
				args->flags |= CLD_FLG_RUNT;
			}
			if (strchr(optarg, 'C')) {
				args->flags |= CLD_FLG_PCYC;
			}
			if (strchr(optarg, 'A')) {
				args->flags |= CLD_FLG_PRFTYPS;
			}
			if (!strchr(optarg, 'P') &&
			    !strchr(optarg, 'A') &&
			    !strchr(optarg, 'X') &&
			    !strchr(optarg, 'R') &&
			    !strchr(optarg, 'C') && !strchr(optarg, 'T')) {
				pMsg(WARN, args,
				     "Unknown performance option\n");
				return (-1);
			}
			break;
		case 'S':
			if (!isdigit((int)optarg[0])) {
				pMsg(WARN, args,
				     "-%c arguments is non numeric.\n", c);
				return (-1);
			}
			args->flags |= CLD_FLG_BLK_RNG;
			if (strchr(optarg, ':') != NULL) {	/* we are given a range */
				args->start_blk =
				    (OFF_T) strtoul(optarg, &leftovers, 0);
				if (leftovers == strchr(leftovers, 'k')) {	/* multiply by 2^10 */
					args->start_blk <<= 10;
					leftovers++;	/* at the ':' */
				} else if (leftovers == strchr(leftovers, 'K')) {	/* multiply 10^3 */
					args->start_blk *= 1000;
					leftovers++;	/* at the ':' */
				} else if (leftovers == strchr(leftovers, 'm')) {	/* multiply by 2^20 */
					args->start_blk <<= 20;
					leftovers++;	/* at the ':' */
				} else if (leftovers == strchr(leftovers, 'M')) {	/* multiply by 10^6 */
					args->start_blk *= 1000000;
					leftovers++;	/* at the ':' */
				} else if (leftovers == strchr(leftovers, 'g')) {	/* multiply by 2^30 */
					args->start_blk <<= 30;
					leftovers++;	/* at the ':' */
				} else if (leftovers == strchr(leftovers, 'G')) {	/* multiply by 10^9 */
					args->start_blk *= 1000000000;
					leftovers++;	/* at the ':' */
				}
				leftovers++;	/* should be at the next value */
				if (!isdigit((int)leftovers[0])) {
					pMsg(WARN, args,
					     "-%c arguments is non numeric.\n",
					     c);
					return (-1);
				}
				args->stop_blk =
				    (OFF_T) strtoul(leftovers, &leftovers, 0);
				if (leftovers == strchr(leftovers, 'k')) {	/* multiply by 2^10 */
					args->stop_blk <<= 10;
				} else if (leftovers == strchr(leftovers, 'K')) {	/* multiply 10^3 */
					args->stop_blk *= 1000;
				} else if (leftovers == strchr(leftovers, 'm')) {	/* multiply by 2^20 */
					args->stop_blk <<= 20;
				} else if (leftovers == strchr(leftovers, 'M')) {	/* multiply by 10^6 */
					args->stop_blk *= 1000000;
				} else if (leftovers == strchr(leftovers, 'g')) {	/* multiply by 2^30 */
					args->stop_blk <<= 30;
				} else if (leftovers == strchr(leftovers, 'G')) {	/* multiply by 10^9 */
					args->stop_blk *= 1000000000;
				}
			} else {	/* only a single value given */
				args->start_blk =
				    (OFF_T) strtoul(optarg, &leftovers, 0);
				if (leftovers == strchr(leftovers, 'k')) {	/* multiply by 2^10 */
					args->start_blk <<= 10;
				} else if (leftovers == strchr(leftovers, 'K')) {	/* multiply 10^3 */
					args->start_blk *= 1000;
				} else if (leftovers == strchr(leftovers, 'm')) {	/* multiply by 2^20 */
					args->start_blk <<= 20;
				} else if (leftovers == strchr(leftovers, 'M')) {	/* multiply by 10^6 */
					args->start_blk *= 1000000;
				} else if (leftovers == strchr(leftovers, 'g')) {	/* multiply by 2^30 */
					args->start_blk <<= 30;
				} else if (leftovers == strchr(leftovers, 'G')) {	/* multiply by 10^9 */
					args->start_blk *= 1000000000;
				}
			}
			break;
		case 's':
			if (!isdigit((int)optarg[0])) {
				pMsg(WARN, args,
				     "-%c argument is non numeric.\n", c);
				return (-1);
			}
			args->flags |= CLD_FLG_LBA_RNG;
			if (strchr(optarg, ':') != NULL) {	/* we are given a range */
				args->start_lba =
				    (OFF_T) strtoul(optarg, &leftovers, 0);
				if (leftovers == strchr(leftovers, 'k')) {	/* multiply by 2^10 */
					args->start_lba <<= 10;
					leftovers++;	/* at the ':' */
				} else if (leftovers == strchr(leftovers, 'K')) {	/* multiply 10^3 */
					args->start_lba *= 1000;
					leftovers++;	/* at the ':' */
				} else if (leftovers == strchr(leftovers, 'm')) {	/* multiply by 2^20 */
					args->start_lba <<= 20;
					leftovers++;	/* at the ':' */
				} else if (leftovers == strchr(leftovers, 'M')) {	/* multiply by 10^6 */
					args->start_lba *= 1000000;
					leftovers++;	/* at the ':' */
				} else if (leftovers == strchr(leftovers, 'g')) {	/* multiply by 2^30 */
					args->start_lba <<= 30;
					leftovers++;	/* at the ':' */
				} else if (leftovers == strchr(leftovers, 'G')) {	/* multiply by 10^9 */
					args->start_lba *= 1000000000;
					leftovers++;	/* at the ':' */
				}
				leftovers++;	/* should be at the next value */
				if (!isdigit((int)leftovers[0])) {
					pMsg(WARN, args,
					     "-%c second argument is non numeric.\n",
					     c);
					return (-1);
				}
				args->stop_lba =
				    (OFF_T) strtoul(leftovers, &leftovers, 0);
				if (leftovers == strchr(leftovers, 'k')) {	/* multiply by 2^10 */
					args->stop_lba <<= 10;
				} else if (leftovers == strchr(leftovers, 'K')) {	/* multiply 10^3 */
					args->stop_lba *= 1000;
				} else if (leftovers == strchr(leftovers, 'm')) {	/* multiply by 2^20 */
					args->stop_lba <<= 20;
				} else if (leftovers == strchr(leftovers, 'M')) {	/* multiply by 10^6 */
					args->stop_lba *= 1000000;
				} else if (leftovers == strchr(leftovers, 'g')) {	/* multiply by 2^30 */
					args->stop_lba <<= 30;
				} else if (leftovers == strchr(leftovers, 'G')) {	/* multiply by 10^9 */
					args->stop_lba *= 1000000000;
				}
			} else {	/* only a single value given */
				args->start_lba =
				    (OFF_T) strtoul(optarg, &leftovers, 0);
				if (leftovers == strchr(leftovers, 'k')) {	/* multiply by 2^10 */
					args->start_lba <<= 10;
				} else if (leftovers == strchr(leftovers, 'K')) {	/* multiply 10^3 */
					args->start_lba *= 1000;
				} else if (leftovers == strchr(leftovers, 'm')) {	/* multiply by 2^20 */
					args->start_lba <<= 20;
				} else if (leftovers == strchr(leftovers, 'M')) {	/* multiply by 10^6 */
					args->start_lba *= 1000000;
				} else if (leftovers == strchr(leftovers, 'g')) {	/* multiply by 2^30 */
					args->start_lba <<= 30;
				} else if (leftovers == strchr(leftovers, 'G')) {	/* multiply by 10^9 */
					args->start_lba *= 1000000000;
				}
			}
			break;
		case '?':
		default:
			usage();
			return (-1);
		}
	}
	if (argv[optind] == NULL) {
		pMsg(WARN, args, "Unspecified target.\n");
		return (-1);
	}
	strncpy(args->device, argv[optind], (DEV_NAME_LEN - 1));
	return 0;
}

int make_assumptions(child_args_t * args)
{
	char TmpStr[80];
	struct stat stat_buf;
	int rv;

	if (!(args->flags & CLD_FLG_IOTYPS)) {
		/* use stat to get file properties, and use to set -I */
		rv = stat(args->device, &stat_buf);
		if (0 == rv) {
			if (IS_FILE(stat_buf.st_mode)) {
				strncat(args->argstr, "(-I f) ",
					(MAX_ARG_LEN - 1) -
					strlen(args->argstr));
				args->flags |= CLD_FLG_FILE;
			} else if (IS_BLK(stat_buf.st_mode)) {
				strncat(args->argstr, "(-I b) ",
					(MAX_ARG_LEN - 1) -
					strlen(args->argstr));
				args->flags |= CLD_FLG_BLK;
#ifndef WINDOWS
			} else if (S_ISCHR(stat_buf.st_mode)) {
				strncat(args->argstr, "(-I r) ",
					(MAX_ARG_LEN - 1) -
					strlen(args->argstr));
				args->flags |= CLD_FLG_RAW;
#endif
			}
		} else {
			pMsg(WARN, args,
			     "Can't get status on %s, defaulting to file, errno = %d\n",
			     args->device, GETLASTERROR());
			strncat(args->argstr, "(-I f) ",
				(MAX_ARG_LEN - 1) - strlen(args->argstr));
			args->flags |= CLD_FLG_FILE;
		}
	}
	if ((args->flags & CLD_FLG_WFSYNC) && (0 == args->sync_interval)) {
		pMsg(INFO, args,
		     "Sync interval set to zero, assuming interval of 1.\n");
		args->sync_interval = 1;
	}

	if (args->ltrsiz <= 0) {
		sprintf(TmpStr, "(-B %d) ", TRSIZ * BLK_SIZE);
		strncat(args->argstr, TmpStr,
			(MAX_ARG_LEN - 1) - strlen(args->argstr));
		args->ltrsiz = TRSIZ;
		args->htrsiz = TRSIZ;
	}
	if (args->flags & CLD_FLG_LBA_RNG) {
		args->start_blk = args->start_lba / args->htrsiz;
		if (!(args->stop_lba < 0))
			args->stop_blk = args->stop_lba / args->htrsiz;
	}
	if (args->flags & CLD_FLG_BLK_RNG) {
		args->start_lba = args->start_blk * args->htrsiz;
		if (!(args->stop_blk < 0))
			args->stop_lba =
			    (args->stop_blk * args->htrsiz) + (args->htrsiz -
							       1);
	}
	/* if vsiz is still not set, try and get it from the file */
	if ((args->vsiz <= 0) && (args->flags & CLD_FLG_FILE)) {
		if (0 != get_file_size(args->device)) {	/* file size retrieved */
			args->vsiz = get_file_size(args->device);
		}
	}
	/* if vsiz is still not set, try and get it from the device */
	if ((args->vsiz <= 0) && !(args->flags & CLD_FLG_FILE)) {
		args->vsiz = get_vsiz(args->device);
	}
	/* if vsiz is still not set, set based on given range */
	if ((args->vsiz <= 0)
	    && (args->flags & (CLD_FLG_LBA_RNG | CLD_FLG_BLK_RNG))) {
		if (!(args->stop_lba < 0))
			args->vsiz = args->stop_lba + 1;
		else
			args->vsiz = args->start_lba + 1;
	}
	/* if vsiz is still not set, then set it to the default size */
	if (args->vsiz <= 0) {
		args->vsiz = VSIZ;
	}
	if (!(args->flags & CLD_FLG_VSIZ)) {
		sprintf(TmpStr, N_ASSUME, args->vsiz);
		strncat(args->argstr, TmpStr,
			(MAX_ARG_LEN - 1) - strlen(args->argstr));
	}

	if (args->stop_lba == -1) {
		args->stop_lba = args->vsiz - 1;
	}
	if (args->stop_blk == -1) {
		args->stop_blk = (args->stop_lba / (OFF_T) args->htrsiz);
	}
	if (args->t_kids == 0) {
		sprintf(TmpStr, "(-K %d) ", KIDS);
		strncat(args->argstr, TmpStr,
			(MAX_ARG_LEN - 1) - strlen(args->argstr));
		args->t_kids = KIDS;
	}
	if ((args->flags & (CLD_FLG_W | CLD_FLG_R)) == 0) {
		if (args->flags & CLD_FLG_DUTY) {	/* no read/write but duty cycle specified */
			if (args->rperc > 0) {
				args->flags |= CLD_FLG_R;
				strncat(args->argstr, "(-r) ",
					(MAX_ARG_LEN - 1) -
					strlen(args->argstr));
			}
			if (args->wperc > 0) {
				args->flags |= CLD_FLG_W;
				strncat(args->argstr, "(-w) ",
					(MAX_ARG_LEN - 1) -
					strlen(args->argstr));
			}
		} else {
			strncat(args->argstr, "(-r) ",
				(MAX_ARG_LEN - 1) - strlen(args->argstr));
			args->flags |= CLD_FLG_R;
		}
	}
	if (!(args->flags & CLD_FLG_PTYPS)) {
		strncat(args->argstr, "(-c) ",
			(MAX_ARG_LEN - 1) - strlen(args->argstr));
		args->flags |= CLD_FLG_CPTYPE;
	}
	if (!(args->flags & CLD_FLG_SKTYPS)) {
		strncat(args->argstr, "(-p R) ",
			(MAX_ARG_LEN - 1) - strlen(args->argstr));
		args->flags |= CLD_FLG_RANDOM;
	}
	if (!(args->flags & CLD_FLG_SKS)) {
		if (args->start_blk == args->stop_blk) {	/* diskcache test, w/ no seek count set */
			args->seeks = SEEKS;
		} else if (args->flags & (CLD_FLG_BLK_RNG | CLD_FLG_LBA_RNG)) {	/* range set, w/ no seek count */
			args->seeks = args->stop_blk - args->start_blk + 1;
		} else {
			/* if vsiz is available, calculated seeks are in terms of the largest transfer size */
			args->seeks =
			    (args->vsiz >
			     0) ? (args->vsiz / args->htrsiz) : SEEKS;
		}
		if ((args->flags & CLD_FLG_LINEAR) && (args->flags & CLD_FLG_R)
		    && (args->flags & CLD_FLG_W)) {
			args->seeks *= 2;
		}

		if (!(args->flags & CLD_FLG_TMD)) {
			sprintf(TmpStr, L_ASSUME, args->seeks);
			strncat(args->argstr, TmpStr,
				(MAX_ARG_LEN - 1) - strlen(args->argstr));
		}
	}
	if (!(args->flags & (CLD_FLG_SKS | CLD_FLG_TMD))
	    || ((args->flags & CLD_FLG_CYC)
		&& !(args->flags & (CLD_FLG_SKS | CLD_FLG_TMD)))) {
		args->flags |= CLD_FLG_SKS;
	}
	if (args->flags & (CLD_FLG_LINEAR)) {
		if (!(args->flags & (CLD_FLG_LUNU | CLD_FLG_LUND))) {
			strncat(args->argstr, "(-p u) ",
				(MAX_ARG_LEN - 1) - strlen(args->argstr));
			args->flags |= CLD_FLG_LUNU;
		}
	}
	normalize_percs(args);
	if (!(args->flags & CLD_FLG_DUTY) && (args->flags & CLD_FLG_RANDOM)
	    && !(args->flags & CLD_FLG_NTRLVD)) {
		sprintf(TmpStr, "(-D %d:%d) ", args->rperc, args->wperc);
		strncat(args->argstr, TmpStr,
			(MAX_ARG_LEN - 1) - strlen(args->argstr));
		args->flags |= CLD_FLG_DUTY;
	}
	if ((args->delayTimeMin == 0) && (args->delayTimeMax == 0)
	    && (args->ioTimeout == DEFAULT_IO_TIMEOUT)) {
		strncat(args->argstr, "(-t 0:2m) ",
			(MAX_ARG_LEN - 1) - strlen(args->argstr));
	}
	if (!(args->flags & CLD_FLG_OFFSET)) {
		strncat(args->argstr, "(-o 0) ",
			(MAX_ARG_LEN - 1) - strlen(args->argstr));
	}

	return 0;
}

/*
 * checks validity of data after parsing
 * args and make assumtions. returns 0 on
 * success and -1 on failure.
 */
int check_conclusions(child_args_t * args)
{
	extern unsigned long glb_flags;
	struct stat stat_buf;
	int rv;

	if ((args->flags & CLD_FLG_DUTY)
	    && ((args->flags & CLD_FLG_LINEAR)
		|| (args->flags & CLD_FLG_NTRLVD))) {
		pMsg(WARN, args,
		     "Duty cycle testing is supported for random (-pR) tests only.\n");
		return (-1);
	}
	if ((args->flags & CLD_FLG_BLK_RNG) && (args->flags & CLD_FLG_RTRSIZ)) {
		pMsg(WARN, args,
		     "Can't have unfixed block sizes and specify seek range in terms of blocks.\n");
		return (-1);
	}
	if ((args->vsiz < 0) || (args->ltrsiz < 1) || (args->htrsiz < 1)) {
		pMsg(WARN, args,
		     "Bounds exceeded for transfer size and/or volume size.\n");
		pMsg(WARN, args, MAXTRSIZ, (args->htrsiz * BLK_SIZE),
		     args->vsiz);
		return (-1);
	}
	if (args->htrsiz < args->ltrsiz) {
		pMsg(ERR, args,
		     "Min transfer size, %lu, greater then Max transfer size, %lu.\n",
		     args->ltrsiz, args->htrsiz);
		return (-1);
	}
	if (args->vsiz < (args->stop_lba - args->start_lba + 1)) {
		pMsg(ERR, args, "Volume stop block/lba exceeds volume size.\n");
		return (-1);
	}
	if (args->vsiz < args->htrsiz) {
		pMsg(WARN, args, VSIZETS, args->vsiz, args->htrsiz);
		return (-1);
	}
	if ((args->flags & CLD_FLG_TMD) == 0 && (args->seeks <= 0)) {
		pMsg(WARN, args, TSEEK, args->seeks);
		return (-1);
	}
	if ((args->flags & CLD_FLG_SKS) && (args->t_kids > args->seeks)) {
		pMsg(WARN, args,
		     "Can't have more children then max number of seeks, use -K/-L to adjust.\n");
		return (-1);
	}
	if ((args->start_blk > args->vsiz)
	    && !(args->flags & (CLD_FLG_BLK_RNG | CLD_FLG_LBA_RNG))) {
		pMsg(WARN, args, STBGTTLBA, args->start_blk,
		     (args->vsiz / args->htrsiz));
		return (-1);
	}
	if ((args->stop_blk > args->vsiz)
	    && !(args->flags & (CLD_FLG_BLK_RNG | CLD_FLG_LBA_RNG))) {
		pMsg(WARN, args, SBGTTLBA, args->stop_blk,
		     (args->vsiz / args->htrsiz));
		return (-1);
	}
	if ((args->start_lba > args->vsiz)
	    && !(args->flags & (CLD_FLG_BLK_RNG | CLD_FLG_LBA_RNG))) {
		pMsg(WARN, args, STLBAGTLBA, args->start_lba, args->vsiz);
		return (-1);
	}
	if ((args->stop_lba > args->vsiz)
	    && !(args->flags & (CLD_FLG_BLK_RNG | CLD_FLG_LBA_RNG))) {
		pMsg(WARN, args, SLBAGTLBA, args->stop_lba, args->vsiz);
		return (-1);
	}
	if (args->start_blk > args->stop_blk) {
		pMsg(WARN, args, SBRSB, args->stop_blk, args->start_blk);
		return (-1);
	}
	if (args->start_lba > args->stop_lba) {
		pMsg(ERR, args, SLBARSLBA, args->stop_lba, args->start_lba);
		return (-1);
	}
	if ((args->flags & CLD_FLG_LBA_RNG) && (args->flags & CLD_FLG_BLK_RNG)) {
		pMsg(ERR, args,
		     "Can't specify range in both block and LBA, use -s or -S.\n");
		return (-1);
	}

	/* use stat to get file properties, and test then agains specified -I */
	rv = stat(args->device, &stat_buf);
	if (0 == rv) {		/* no error on call to stat, compare against -I option */
		/* files are usually file type */
		if ((args->flags & CLD_FLG_FILE) && !IS_FILE(stat_buf.st_mode)) {
			pMsg(ERR, args,
			     "Can't open non-file filespec with file device type, -If.\n");
			return (-1);
		}
		/* block devices, are usually block type */
		if ((args->flags & CLD_FLG_BLK) && !IS_BLK(stat_buf.st_mode)) {
			pMsg(ERR, args,
			     "Can't open non-block filespec with block device type, -Ib.\n");
			return (-1);
		}
#ifndef WINDOWS
		/* raw devices, are usually character type */
		if ((args->flags & CLD_FLG_RAW) && !S_ISCHR(stat_buf.st_mode)) {
			pMsg(ERR, args,
			     "Can't open non-raw filespec with raw device type, -Ir.\n");
			return (-1);
		}
#else
		if (args->flags & CLD_FLG_RAW) {
			pMsg(ERR, args,
			     "RAW IO type not supported in Windows, use direct IO instead.\n");
			return (-1);
		}
#endif
#ifdef _DEBUG
	} else {
		PDBG1(DBUG, args,
		      "Can't get status on %s, assuming a new file, errno = %d\n",
		      args->device, GETLASTERROR());
#endif
	}

	if ((args->hbeat > 0) && (args->flags & CLD_FLG_TMD)
	    && (args->hbeat > args->run_time)) {
		pMsg(ERR, args,
		     "Heartbeat should be at least equal to runtime, use -h/-T to adjust.\n");
		return (-1);
	}
	if ((args->hbeat > 0) && !(args->flags & CLD_FLG_PRFTYPS)) {
		pMsg(ERR, args,
		     "At least one performance option, -P, must be specified when using -h.\n");
		return (-1);
	}
	if ((args->flags & CLD_FLG_W) && !(args->flags & CLD_FLG_R)
	    && (args->flags & CLD_FLG_CMPR)) {
		pMsg(ERR, args, "Write only, ignoring option -E.\n");
	}
	if ((args->flags & CLD_FLG_TMD) && (args->flags & CLD_FLG_SKS)) {
		pMsg(ERR, args,
		     "Can't specify both -L and -T they are mutually exclusive.\n");
		return (-1);
	}
	if (((args->flags & CLD_FLG_R) && !(args->flags & CLD_FLG_W))
	    && (args->flags & CLD_FLG_ERR_MARK)) {
		pMsg(ERR, args,
		     "Can't specify mark on error, -Am, in read only mode.\n");
		return (-1);
	}
	if (!(args->flags & CLD_FLG_ALLDIE) && (args->flags & CLD_FLG_ERR_MARK)) {
		pMsg(ERR, args,
		     "Can't specify mark on error, -Am, when continue on error is set.\n");
		return (-1);
	}
	if ((glb_flags & GLB_FLG_KILL) && !(args->flags & CLD_FLG_ALLDIE)) {
		pMsg(ERR, args,
		     "Can't specify global kill, -Ag, when continue on error is set, -Ac.\n");
		return (-1);
	}
	if ((args->flags & CLD_FLG_LINEAR) && !(args->flags & CLD_FLG_NTRLVD)
	    && (args->flags & CLD_FLG_TMD)) {
		pMsg(ERR, args, "Linear read / write test can not be timed.\n");
		return (-1);
	}
	if ((args->flags & CLD_FLG_CMPR)
	    && (args->cmp_lng > (args->ltrsiz * BLK_SIZE))) {
		pMsg(ERR, args,
		     "Compare length, %lu, is greater then transfer size, %lu\n",
		     args->cmp_lng, args->ltrsiz * BLK_SIZE);
		return (-1);
	}
	if ((args->flags & CLD_FLG_OFFSET) && (args->offset > args->stop_lba)) {
		pMsg(ERR, args, LBAOFFGSLBA, args->offset, args->stop_lba);
		return (-1);
	}
	if ((args->flags & CLD_FLG_OFFSET)
	    && ((args->offset + args->ltrsiz - 1) > args->stop_lba)) {
		pMsg(ERR, args, LBAOTSGSLBA, args->offset, args->ltrsiz,
		     args->stop_lba);
		return (-1);
	}
	return 0;
}
