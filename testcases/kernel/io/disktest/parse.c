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
* $Id: parse.c,v 1.5 2005/05/04 17:54:00 mridge Exp $
* $Log: parse.c,v $
* Revision 1.5  2005/05/04 17:54:00  mridge
* Update to version 1.2.8
*
* Revision 1.18  2005/04/28 21:25:18  yardleyb
* Fixed up some issues with AIX compilation due to the change made
* in endian support in Linux.
*
* Revision 1.17  2005/01/08 21:18:34  yardleyb
* Update performance output and usage.  Fixed pass count check
*
* Revision 1.16  2004/12/18 06:13:03  yardleyb
* Updated timer schema to more accurately use the time options.  Added
* fsync on write option to -If.
*
* Revision 1.15  2004/11/20 05:05:58  yardleyb
* added more command line checking
*
* Revision 1.14  2004/11/20 04:43:42  yardleyb
* Minor code fixes.  Checking for alloc errors.
*
* Revision 1.13  2004/11/02 20:47:13  yardleyb
* Added -F functions.
* lots of minor fixes. see README
*
* Revision 1.12  2003/09/12 21:47:22  yardleyb
* Removed a debug message
*
* Revision 1.11  2003/09/12 18:10:09  yardleyb
* Updated to version 1.11
* Code added to fix compile
* time warnings
*
* Revision 1.10  2003/04/08 17:21:19  yardleyb
* Added get volume size code for any AIX volume type
*
* Revision 1.9  2002/05/31 18:47:59  yardleyb
* Updates to -pl -pL options.
* Fixed test status to fail on
* failure to open filespec.
* Version set to 1.1.9
*
* Revision 1.8  2002/04/24 01:45:31  yardleyb
* Minor Fixes:
* Read/write time could exceeds overall time
* Heartbeat options sometimes only displayed once
* Cleanup time for large number of threads was very long (windows)
* If heartbeat specified, now checks for performance option also
* No IO was performed when -S0:0 and -pr specified
*
* Revision 1.7  2002/03/30 01:32:14  yardleyb
* Major Changes:
*
* Added Dumping routines for
* data miscompares,
*
* Updated performance output
* based on command line.  Gave
* one decimal in MB/s output.
*
* Rewrote -pL IO routine to show
* correct stats.  Now show pass count
* when using -C.
*
* Minor Changes:
*
* Code cleanup to remove the plethera
* if #ifdef for windows/unix functional
* differences.
*
* Revision 1.6  2002/03/07 03:38:52  yardleyb
* Added dump function from command
* line.  Created formatted dump output
* for Data miscomare and command line.
* Can now leave off filespec the full
* path header as it will be added based
* on -I.
*
* Revision 1.5  2002/02/26 19:35:59  yardleyb
* Updates to parsing routines for user
* input.  Added multipliers for -S and
* -s command line arguments. Forced
* default seeks to default if performing
* a diskcache test.
*
* Revision 1.4  2002/02/19 02:46:37  yardleyb
* Added changes to compile for AIX.
* Update getvsiz so it returns a -1
* if the ioctl fails and we handle
* that fact correctly.  Added check
* to force vsiz to always be greater
* then stop_lba.
*
* Revision 1.3  2002/02/04 20:35:38  yardleyb
* Changed max. number of threads to 64k.
* Check for max threads in parsing.
* Fixed windows getopt to return correctly
* when a bad option is given.
* Update time output to be in the format:
*   YEAR/MONTH/DAY-HOUR:MIN:SEC
* instead of epoch time.
*
* Revision 1.2  2001/12/04 19:25:47  yardleyb
* Finished removeal of -t option
*
* Revision 1.1  2001/12/04 18:51:06  yardleyb
* Checkin of new source files and removal
* of outdated source
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

#include "globals.h"
#include "threading.h"
#include "main.h"
#include "usage.h"
#include "sfunc.h"
#include "parse.h"

int fill_cld_args(int argc, char **argv, child_args_t *args)
{
	extern char *optarg;
	extern int optind;
	extern unsigned long glb_flags;

	signed char c;
	char *leftovers;

	while((c = getopt(argc, argv, "?a:AB:cC:dD:E:f:Fh:I:K:L:m:nN:op:P:qQrs:S:t:T:wvV:z")) != -1) {
		switch(c) {
			case ':' :
				pMsg(WARN, args, "Missing argument for perameter.\n");
				usage();
				return(-1);
			case 'V' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, args, "-%c argument is non numeric.\n", c);
					exit(1);
				}
				gbl_dbg_lvl = atoi(optarg);
				break;
			case 'd' :
				glb_flags |= GLB_FLG_QUIET;
				args->flags |= CLD_FLG_DUMP;
				break;
			case 'a' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, args, "-%c arguments is non numeric.\n", c);
					return(-1);
				}
				args->seed = (unsigned int) atoi(optarg);
				break;
			case 'A' :
				args->flags &= ~CLD_FLG_ALLDIE;
				break;
			case 'q' :
				glb_flags |= GLB_FLG_QUIET;
				break;
			case 'Q' :
				glb_flags |= GLB_FLG_SUPRESS;
				break;
			case 'v' :
				pMsg(INFO, args, "Version %s\n", VER_STR);
				exit(0);
			case 'p' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				if(args->flags & (CLD_FLG_LINEAR|CLD_FLG_RANDOM)) {
					pMsg(WARN, args, "Only one seek type, -p, can be specified.\n");
					return(-1);
				}
				/* seek pattern type */
				if (strchr(optarg,'L'))
					args->flags |= CLD_FLG_LINEAR;
				else if (strchr(optarg,'l'))
					args->flags |= (CLD_FLG_LINEAR|CLD_FLG_NTRLVD);
				else if (strchr(optarg,'R') || strchr(optarg,'r'))
					args->flags |= CLD_FLG_RANDOM;
				else {
					pMsg(WARN, args, "Unknown Seek pattern\n");
					usage();
					return(-1);
				}
				if (strchr(optarg,'U') || strchr(optarg,'u'))
					if((args->flags & (CLD_FLG_LINEAR)) &&
							!(args->flags & CLD_FLG_LUND))
						args->flags |= CLD_FLG_LUNU;
				if (strchr(optarg,'D') || strchr(optarg,'d'))
					if((args->flags & (CLD_FLG_LINEAR)) &&
							!(args->flags & CLD_FLG_LUNU))
						args->flags |= CLD_FLG_LUND;
				break;
			case 'B' :
				if(!isdigit(optarg[0])) {
					pMsg(WARN, args, "-%c arguments is non numeric.\n", c);
					return(-1);
				}
				if(strchr(optarg,':') != NULL) { /* we are given a range of transfer sizes */
					args->flags |= CLD_FLG_RTRSIZ;
					args->ltrsiz = strtoul(optarg, &leftovers, 10);
					if(leftovers == strchr(leftovers,'k')) { /* first value had a 'k' */
						args->ltrsiz *= 2;
						leftovers++;
					} else {
						if (args->ltrsiz > 256)
							args->ltrsiz /= BLK_SIZE;
					}
					if(!isdigit(leftovers[1])) {
						pMsg(WARN, args, "-%c arguments is non numeric.\n", c);
						return(-1);
					}
					args->htrsiz = atol((char *)strchr(leftovers,':')+1);
					if((strchr(leftovers,'k')) != NULL) {/* second value had a 'k' */
						args->htrsiz *= 2;
					} else {
						if (args->htrsiz > 256)
							   args->htrsiz /= BLK_SIZE;
					}
				} else { /* only a single value given for transfer size */
					if(strchr(optarg,'k')) {
						args->ltrsiz = atoi(optarg);
						args->ltrsiz *= 2;
					} else {
						args->ltrsiz = atoi(optarg);
						if (args->ltrsiz > 256)
							   args->ltrsiz /= BLK_SIZE;
					}
					args->htrsiz = args->ltrsiz;
				}
				PDBG5(DEBUG, args, "Parsed Transfer size: %ld\n", args->htrsiz);
				break;
			case 'c' :
				if(args->flags & CLD_FLG_PTYPS) {
					pMsg(WARN, args, "Please specify only one pattern type\n");
					usage();
					return(-1);
				}
				args->flags |= CLD_FLG_CPTYPE;
				break;
			case 'n' :
				if(args->flags & CLD_FLG_PTYPS) {
					pMsg(WARN, args, "Please specify only one pattern type\n");
					usage();
					return(-1);
				}
				args->flags |= CLD_FLG_LPTYPE;
				break;
			case 'f' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				if(args->flags & CLD_FLG_PTYPS) {
					pMsg(WARN, args, "Please specify only one pattern type\n");
					usage();
					return(-1);
				}
				args->pattern = my_strtofft(optarg);
				args->flags |= CLD_FLG_FPTYPE;
				break;
			case 'F' :
				/* the filespec is a list of filespecs in a file */
				args->flags |= CLD_FLG_FSLIST;
				break;
			case 'z' :
				if(args->flags & CLD_FLG_PTYPS) {
					pMsg(WARN, args, "Please specify only one pattern type\n");
					usage();
					return(-1);
				}
				args->flags |= CLD_FLG_RPTYPE;
				break;
			case 'h' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, args, "-%c arguments is non numeric.\n", c);
					usage();
					return(-1);
				}
				args->flags |= CLD_FLG_HBEAT;
				args->hbeat = atoi(optarg);
				if(strchr(optarg,'m')) {	/* multiply by sec */
					args->hbeat *= 60;
				} else if(strchr(optarg,'h')) { /* multiply sec*min */
					args->hbeat *= (time_t) (60*60);
				} else if(strchr(optarg,'d')) { /* multiply by sec*min*hours */
					args->hbeat *= (time_t) (60*60*24);
				}
				break;
			case 'D' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, args, "-%c arguments is non numeric.\n", c);
					usage();
					return(-1);
				}
				args->rperc = atoi(optarg);
				args->wperc = atoi((char *)(strchr(optarg,':')+1));
				args->flags |= CLD_FLG_DUTY;
				break;
			case 'r' : 
				args->flags |= CLD_FLG_R;
				break;
			case 'w' : 
				args->flags |= CLD_FLG_W;
				break;
			case 'o' : { args->flags |= CLD_FLG_SQNCE; break; }
			case 't' : 
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				args->retries = atoi(optarg);
				pMsg(ERR, args, "-%c not implemented...\n", c);
				return(-1);
				break;
			case 'm' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				if (strchr(optarg,'L') || strchr(optarg,'l'))
					args->mrk_flag |= MARK_LAST;
				else if (strchr(optarg,'F') || strchr(optarg,'f'))
					args->mrk_flag |= MARK_FIRST;
				else if (strchr(optarg,'A') || strchr(optarg,'a'))
					args->mrk_flag |= MARK_ALL;
				else {
					pMsg(WARN, args, "Unknown mark type.\n");
					return(-1);
				}
				args->flags |= CLD_FLG_MBLK;
				break;
			case 'E' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, args, "-%c arguments are non numeric.\n", c);
					usage();
					return(-1);
				}
				args->flags |= CLD_FLG_CMPR;
				args->cmp_lng = strtol(optarg,NULL,0);
				break;
			case 'N' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, args, "-%c arguments are non numeric.\n", c);
					return(-1);
				}
				args->flags |= CLD_FLG_VSIZ;
				args->vsiz = my_strtofft(optarg);
				if(strchr(optarg,'k')) {	/* multiply by 2^10 */
					args->vsiz <<= 10;
				} else if(strchr(optarg,'K')) { /* multiply 10^3 */
					args->vsiz *= 1000;
				} else if(strchr(optarg,'m')) { /* multiply by 2^20 */
					args->vsiz <<= 20;
				} else if(strchr(optarg,'M')) { /* multiply by 10^6 */
					args->vsiz *= 1000000;
				} else if(strchr(optarg,'g')) { /* multiply by 2^30 */
					args->vsiz <<= 30;
				} else if(strchr(optarg,'G')) { /* multiply by 10^9 */
					args->vsiz *= 1000000000;
				}
				break;
			case 'I' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				if (strchr(optarg,'R') || strchr(optarg,'r')) {
					if (!(args->flags & CLD_FLG_BLK) &&
					    !(args->flags & CLD_FLG_FILE)) {
						args->flags |= CLD_FLG_RAW;
					} else {
						pMsg(WARN, args, "Can only specify one IO type\n");
						return(-1);
					}
				}
				if (strchr(optarg,'B') || strchr(optarg,'b')) {
					if (!(args->flags & CLD_FLG_RAW) &&
					    !(args->flags & CLD_FLG_FILE)) {
						args->flags |= CLD_FLG_BLK;
					} else {
						pMsg(WARN, args, "Can only specify one IO type\n");
						return(-1);
					}
				}
				if (strchr(optarg,'F') || strchr(optarg,'f')) {
					if (!(args->flags & CLD_FLG_RAW) &&
					    !(args->flags & CLD_FLG_BLK)) {
						args->flags |= CLD_FLG_FILE;
					} else {
						pMsg(WARN, args, "Can only specify one IO type\n");
						return(-1);
					}
				}
				if (strchr(optarg,'D') || strchr(optarg,'d')) {
					args->flags |= CLD_FLG_DIRECT;
				}
				if (strchr(optarg,'S') || strchr(optarg,'s')) {
					if(!(args->flags & CLD_FLG_FILE)) {
						pMsg(WARN, args, "Can only specify fsync with file IO\n");
						return(-1);
					}
					args->flags |= CLD_FLG_WFSYNC;
				}
				break;
			case 'T' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				args->run_time = atoi(optarg);
				args->flags |= CLD_FLG_TMD;
				if(strchr(optarg,'m')) {	/* multiply by sec */
					args->run_time *= 60;
				} else if(strchr(optarg,'h')) { /* multiply sec*min */
					args->run_time *= (time_t) (60*60);
				} else if(strchr(optarg,'d')) { /* multiply by sec*min*hours */
					args->run_time *= (time_t) (60*60*24);
				}
				break;
			case 'L' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				args->seeks = atoi(optarg);
				args->flags |= CLD_FLG_SKS;
				if(strchr(optarg,'k')) {	/* multiply by 2^10 */
					args->seeks <<= 10;
				} else if(strchr(optarg,'K')) { /* multiply 10^3 */
					args->seeks *= 1000;
				} else if(strchr(optarg,'m')) { /* multiply by 2^20 */
					args->seeks <<= 20;
				} else if(strchr(optarg,'M')) { /* multiply by 10^6 */
					args->seeks *= 1000000;
				} else if(strchr(optarg,'g')) { /* multiply by 2^30 */
					args->seeks <<= 30;
				} else if(strchr(optarg,'G')) { /* multiply by 10^9 */
					args->seeks *= 1000000000;
				}
				break;
			case 'C' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, args, "-%c arguments is non numeric.\n", c);
					usage();
					return(-1);
				}
				args->flags |= CLD_FLG_CYC;
				args->cycles = atol(optarg);
				break;
			case 'K' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, args, "-%c arguments is non numeric.\n", c);
					usage();
					return(-1);
				}
				if(atoi(optarg) > MAX_THREADS) {
					pMsg(WARN, args, "%u exceeds max of %u threads.\n", atoi(optarg), MAX_THREADS);
					return(-1);
				}
				args->t_kids = atoi(optarg);
				break;
			case 'P' :
				if(optarg == NULL) {
					pMsg(WARN, args, "-%c option requires an argument.\n", c);
					return(-1);
				}
				if (strchr(optarg,'X')) { /* returns NULL if char is not found */
					args->flags |= CLD_FLG_XFERS;
				}
				if (strchr(optarg,'T')) {
					args->flags |= CLD_FLG_TPUTS;
				}
				if (strchr(optarg,'P')) {
					glb_flags |= GLB_FLG_PERFP;
				}
				if (strchr(optarg,'R')) {
					args->flags |= CLD_FLG_RUNT;
				}
				if (strchr(optarg,'C')) {
					args->flags |= CLD_FLG_PCYC;
				}
				if (strchr(optarg,'A')) {
					args->flags |= CLD_FLG_PRFTYPS;
				}
				if (!strchr(optarg,'P') &&
						!strchr(optarg,'A') &&
						!strchr(optarg,'X') &&
						!strchr(optarg,'R') &&
						!strchr(optarg,'C') &&
						!strchr(optarg,'T')) {
					pMsg(WARN, args, "Unknown performance option\n");
					return(-1);
				}
				break;
			case 'S' :
				if(!isdigit((int)optarg[0])) {
					pMsg(WARN, args, "-%c arguments is non numeric.\n", c);
					return(-1);
				}
				args->flags |= CLD_FLG_BLK_RNG;
				if(strchr(optarg,':') != NULL) { /* we are given a range */
					args->start_blk = (OFF_T) strtoul(optarg, &leftovers, 10);
					if(leftovers == strchr(leftovers,'k')) { /* multiply by 2^10 */
						args->start_blk <<= 10;
						leftovers++;  /* at the ':' */
					} else if(leftovers == strchr(leftovers,'K')) { /* multiply 10^3 */
						args->start_blk *= 1000;
						leftovers++;  /* at the ':' */
					} else if(leftovers == strchr(leftovers,'m')) { /* multiply by 2^20 */
						args->start_blk <<= 20;
						leftovers++;  /* at the ':' */
					} else if(leftovers == strchr(leftovers,'M')) { /* multiply by 10^6 */
						args->start_blk *= 1000000;
						leftovers++;  /* at the ':' */
					}
					leftovers++;  /* should be at the next value */
					if(!isdigit((int) leftovers[0])) {
						pMsg(WARN, args, "-%c arguments is non numeric.\n", c);
						return(-1);
					}
					args->stop_blk = (OFF_T) strtoul(leftovers, &leftovers, 10);
					if(leftovers == strchr(leftovers,'k')) { /* multiply by 2^10 */
						args->stop_blk <<= 10;
					} else if(leftovers == strchr(leftovers,'K')) { /* multiply 10^3 */
						args->stop_blk *= 1000;
					} else if(leftovers == strchr(leftovers,'m')) { /* multiply by 2^20 */
						args->stop_blk <<= 20;
					} else if(leftovers == strchr(leftovers,'M')) { /* multiply by 10^6 */
						args->stop_blk *= 1000000;
					}
				} else { /* only a single value given */
					args->start_blk = (OFF_T) strtoul(optarg, &leftovers, 10);
					if(leftovers == strchr(leftovers,'k')) { /* multiply by 2^10 */
						args->start_blk <<= 10;
					} else if(leftovers == strchr(leftovers,'K')) { /* multiply 10^3 */
						args->start_blk *= 1000;
					} else if(leftovers == strchr(leftovers,'m')) { /* multiply by 2^20 */
						args->start_blk <<= 20;
					} else if(leftovers == strchr(leftovers,'M')) { /* multiply by 10^6 */
						args->start_blk *= 1000000;
					}
				}
				break;
			case 's' :
				if(!isdigit((int)optarg[0])) {
					pMsg(WARN, args, "-%c argument is non numeric.\n", c);
					return(-1);
				}
				args->flags |= CLD_FLG_LBA_RNG;
				if(strchr(optarg,':') != NULL) { /* we are given a range */
					args->start_lba = (OFF_T) strtoul(optarg, &leftovers, 10);
					if(leftovers == strchr(leftovers,'k')) { /* multiply by 2^10 */
						args->start_lba <<= 10;
						leftovers++;  /* at the ':' */
					} else if(leftovers == strchr(leftovers,'K')) { /* multiply 10^3 */
						args->start_lba *= 1000;
						leftovers++;  /* at the ':' */
					} else if(leftovers == strchr(leftovers,'m')) { /* multiply by 2^20 */
						args->start_lba <<= 20;
						leftovers++;  /* at the ':' */
					} else if(leftovers == strchr(leftovers,'M')) { /* multiply by 10^6 */
						args->start_lba *= 1000000;
						leftovers++;  /* at the ':' */
					}
					leftovers++;  /* should be at the next value */
					if(!isdigit((int) leftovers[0])) {
						pMsg(WARN, args, "-%c second argument is non numeric.\n", c);
						return(-1);
					}
					args->stop_lba = (OFF_T) strtoul(leftovers, &leftovers, 10);
					if(leftovers == strchr(leftovers,'k')) { /* multiply by 2^10 */
						args->stop_lba <<= 10;
					} else if(leftovers == strchr(leftovers,'K')) { /* multiply 10^3 */
						args->stop_lba *= 1000;
					} else if(leftovers == strchr(leftovers,'m')) { /* multiply by 2^20 */
						args->stop_lba <<= 20;
					} else if(leftovers == strchr(leftovers,'M')) { /* multiply by 10^6 */
						args->stop_lba *= 1000000;
					}
				} else { /* only a single value given */
					args->start_lba = (OFF_T) strtoul(optarg, &leftovers, 10);
					if(leftovers == strchr(leftovers,'k')) { /* multiply by 2^10 */
						args->start_lba <<= 10;
					} else if(leftovers == strchr(leftovers,'K')) { /* multiply 10^3 */
						args->start_lba *= 1000;
					} else if(leftovers == strchr(leftovers,'m')) { /* multiply by 2^20 */
						args->start_lba <<= 20;
					} else if(leftovers == strchr(leftovers,'M')) { /* multiply by 10^6 */
						args->start_lba *= 1000000;
					}
				}
				break;
			case '?' :
			default  :
				usage();
				return(-1);
		}
	}
	if(argv[optind] == NULL) {
		pMsg(WARN, args, "Unspecified target.\n");
		return(-1);
	}
	strncpy(args->device, argv[optind], (DEV_NAME_LEN-1));
	return(0);
}

int make_assumptions(child_args_t *args)
{
	char TmpStr[80];
	char *device;

	if(!(args->flags & CLD_FLG_IOTYPS)) {
		if((device = (char *) ALLOC(strlen(args->device)+1)) == NULL) {
			pMsg(ERR, args, "%d : Could not create device string map.\n");
			return 1;
		}

		memset(device, 0, strlen(args->device)+1);
		strncpy(device, args->device, strlen(args->device));
#ifndef WINDOWS
		if(strstr(strlwr(device), RAWDEVICESTR) != NULL) {
			strncat(args->argstr, "(-I r) ", (MAX_ARG_LEN-1)-strlen(args->argstr));
			args->flags |= CLD_FLG_RAW;
		} else
#endif
		if(strstr(strupr(device), BLKDEVICESTR) != NULL) {
			strncat(args->argstr, "(-I b) ", (MAX_ARG_LEN-1)-strlen(args->argstr));
			args->flags |= CLD_FLG_BLK;
		} else {
			strncat(args->argstr, "(-I f) ", (MAX_ARG_LEN-1)-strlen(args->argstr));
			args->flags |= CLD_FLG_FILE;
		}
		FREE(device);
	}

	if(args->ltrsiz <= 0) {
		sprintf(TmpStr, "(-B %d) ", TRSIZ*BLK_SIZE);
		strncat(args->argstr, TmpStr, (MAX_ARG_LEN-1)-strlen(args->argstr));
		args->ltrsiz=TRSIZ;
		args->htrsiz=TRSIZ;
	}
	if(args->flags & CLD_FLG_LBA_RNG) {
		args->start_blk = args->start_lba / args->htrsiz;
		if(!(args->stop_lba < 0))
			args->stop_blk = args->stop_lba / args->htrsiz;
	}
	if(args->flags & CLD_FLG_BLK_RNG) {
		args->start_lba = args->start_blk * args->htrsiz;
		if(!(args->stop_blk < 0))
			args->stop_lba = args->stop_blk * args->htrsiz;
	}
	/* override volume size in cases were range is given */
	if(args->flags & CLD_FLG_LBA_RNG) {
		if(!(args->stop_lba < 0))
			args->vsiz=args->stop_lba-args->start_lba+1;
	}
	if(args->flags & CLD_FLG_BLK_RNG) {
		if(!(args->stop_lba < 0))
			args->vsiz=args->stop_lba-args->start_lba+args->htrsiz;
	}
	if((args->vsiz <=0) && (args->flags & CLD_FLG_FILE)) {
		args->vsiz=get_file_size(args->device);
	}
#ifdef AIX
	if(args->vsiz <= 0) { args->vsiz=get_vsiz(args->device); }
#else
	if((args->vsiz <= 0) && (args->flags & CLD_FLG_BLK)) {
		args->vsiz=get_vsiz(args->device);
	}
#endif
	if(args->vsiz <= 0) {
		args->vsiz=VSIZ;
	}
	if(!(args->flags & CLD_FLG_VSIZ)) {
#ifdef WINDOWS
		sprintf(TmpStr, "(-N %I64d) ", args->vsiz);
#else
		sprintf(TmpStr, "(-N %lld) ", args->vsiz);
#endif
		strncat(args->argstr, TmpStr, (MAX_ARG_LEN-1)-strlen(args->argstr));
	}

	if(args->stop_lba == -1) {
		args->stop_lba=args->vsiz-1;
	}
	if(args->stop_blk == -1) {
		args->stop_blk=(args->vsiz / (OFF_T) args->htrsiz) - 1;
	}
	if(args->t_kids == 0) {
		sprintf(TmpStr, "(-K %d) ", KIDS);
		strncat(args->argstr, TmpStr, (MAX_ARG_LEN-1)-strlen(args->argstr));
		args->t_kids=KIDS;
	}
	if((args->flags & (CLD_FLG_W|CLD_FLG_R)) == 0) {
		if(args->flags & CLD_FLG_DUTY) { /* no read/write but duty cycle specified */
			if(args->rperc > 0) {
				args->flags |= CLD_FLG_R;
				strncat(args->argstr, "(-r) ", (MAX_ARG_LEN-1)-strlen(args->argstr));
			}
			if(args->wperc > 0) {
				args->flags |= CLD_FLG_W;
				strncat(args->argstr, "(-w) ", (MAX_ARG_LEN-1)-strlen(args->argstr));
			}
		} else {
			strncat(args->argstr, "(-r) ", (MAX_ARG_LEN-1)-strlen(args->argstr));
			args->flags |= CLD_FLG_R;
		}
	}
	if(!(args->flags & CLD_FLG_PTYPS)) {
		strncat(args->argstr, "(-c) ", (MAX_ARG_LEN-1)-strlen(args->argstr));
		args->flags |= CLD_FLG_CPTYPE;
	}
	if(!(args->flags & CLD_FLG_SKTYPS)) {
		strncat(args->argstr, "(-p r) ", (MAX_ARG_LEN-1)-strlen(args->argstr));
		args->flags |= CLD_FLG_RANDOM;
	}
	if(!(args->flags & CLD_FLG_SKS)) {
		if(args->vsiz == args->htrsiz) { /* diskcache test */
			args->seeks = SEEKS;
		} else {
			args->seeks = (args->vsiz > 0) ? args->vsiz : SEEKS;
			args->seeks /= args->htrsiz;	/* calculated seeks are in terms of the largest transfer size */
		}
		if((args->flags & CLD_FLG_LINEAR) && (args->flags & CLD_FLG_R) && (args->flags & CLD_FLG_W)) {
			args->seeks *= 2;
		}

		if(!(args->flags & CLD_FLG_TMD)) {
#ifdef WINDOWS
			sprintf(TmpStr, "(-L %I64d) ", args->seeks);
#else
			sprintf(TmpStr, "(-L %lld) ", args->seeks);
#endif
			strncat(args->argstr, TmpStr, (MAX_ARG_LEN-1)-strlen(args->argstr));
		}
	}
	if(!(args->flags & (CLD_FLG_SKS|CLD_FLG_TMD)) || (args->flags & CLD_FLG_CYC)) {
		args->flags |= CLD_FLG_SKS;
	}
	if(args->flags & (CLD_FLG_LINEAR)) {
		if(!(args->flags & (CLD_FLG_LUNU|CLD_FLG_LUND))) {
			strncat(args->argstr, "(-p u) ", (MAX_ARG_LEN-1)-strlen(args->argstr));
			args->flags |= CLD_FLG_LUNU;
		}
	}
	normalize_percs(args);
	if(!(args->flags & CLD_FLG_DUTY) && (args->flags & CLD_FLG_RANDOM)) {
		sprintf(TmpStr, "(-D %d:%d)", args->rperc, args->wperc);
		strncat(args->argstr, TmpStr, (MAX_ARG_LEN-1)-strlen(args->argstr));
		args->flags |= CLD_FLG_DUTY;
	}

	return(0);
}

/*
 * checks validity of data after parsing
 * args and make assumtions. returns 0 on
 * success and -1 on failure.
 */
int check_conclusions(child_args_t *args)
{
	if((args->flags & CLD_FLG_DUTY) && (args->flags & CLD_FLG_LINEAR)) {
		pMsg(WARN, args, "Duty cycle testing is supported for random (-pr) tests only.\n");
		return(-1);
	}
	if((args->flags & CLD_FLG_BLK_RNG) && (args->flags & CLD_FLG_RTRSIZ)) {
		pMsg(WARN, args, "Can't have unfixed block sizes and specify seek range in terms of blocks.\n");
		return(-1);
	}
	/* Commented out to not limit the block transfer size
	if(args->htrsiz > M_BLK_SIZE) {
		pMsg(WARN, args, "Transfer size exceeds max transfer size of %luk, use -B to adjust.\n", M_BLK_SIZE/2);
		return(-1);
	}
	*/
	if((args->vsiz < 0) || (args->ltrsiz < 1) || (args->htrsiz < 1)) {
		pMsg(WARN, args, "Bounds exceeded for transfer size and/or volume size.\n");
#ifdef WINDOWS
		pMsg(WARN, args, "Max transfer size is %lu and Volume size is %I64d\n",(args->htrsiz*BLK_SIZE),args->vsiz);
#else
		pMsg(WARN, args, "Max transfer size is %lu and Volume size is %lld\n",(args->htrsiz*BLK_SIZE),args->vsiz);
#endif
		return(-1);
	}
	if(args->vsiz < (args->stop_lba-args->start_lba+1)) {
		pMsg(ERR, args, "Volume stop block/lba exceeds volume size.\n");
		return(-1);
	}
	if(args->vsiz < args->htrsiz) {
		pMsg(WARN, args, "Volume size is to small for transfer size.\n");
		return(-1);
	}
	if((!args->flags & CLD_FLG_TMD) && (args->seeks <= 0)) {
#ifdef WINDOWS
		pMsg(WARN,args, "Total seeks of %I64d, is invalid.\n", args->seeks);
#else
		pMsg(WARN,args, "Total seeks of %lld, is invalid.\n", args->seeks);
#endif
		return(-1);
	}
	if((args->flags & CLD_FLG_SKS) && (args->t_kids > args->seeks)) {
		pMsg(WARN, args, "Can't have more children then max number of seeks, use -K/-L to adjust.\n");
		return(-1);
	}
	if(args->start_blk > args->stop_blk) {
		pMsg(WARN, args, "Stop Block of range must be greater the Start Block.\n");
		return(-1);
	}
	if(args->start_lba > args->stop_lba) {
		pMsg(WARN, args, "Stop LBA of range must be greater the Start LBA.\n");
		return(-1);
	}
	if((args->flags & CLD_FLG_LBA_RNG) && (args->flags & CLD_FLG_BLK_RNG)) {
		pMsg(WARN, args, "Can't specify range in both block and LBA, use -s or -S.\n");
		return(-1);
	}
	if((args->flags & CLD_FLG_FILE) && (strstr(args->device,BLKDEVICESTR) != NULL)) {
		pMsg(WARN, args, "Can't open block device with file I/O type.\n");
		return(-1);
	}
#ifndef WINDOWS
	if((args->flags & CLD_FLG_BLK) && (strstr(args->device,RAWDEVICESTR) != NULL)) {
		pMsg(WARN, args, "Raw device filespec for Block I/O type.\n");
		return(-1);
	}
	if((args->flags & CLD_FLG_RAW) && (strstr(args->device,RAWDEVICESTR) == NULL)) {
		pMsg(WARN, args, "Device not a raw device.\n");
		return(-1);
	}
#else
	if(args->flags & CLD_FLG_RAW) {
		pMsg(WARN, args, "RAW IO type not supported in WINDOWS environments.\n");
		return(-1);
	}
#endif
	if((args->hbeat > 0) && (args->flags & CLD_FLG_TMD) && (args->hbeat > args->run_time)) {
		pMsg(WARN, args, "Heartbeat should be at least equal to runtime, use -h/-T to adjust.\n");
		return(-1);
	}
	if((args->hbeat > 0) && !(args->flags & CLD_FLG_PRFTYPS)) {
		pMsg(WARN, args, "At least one performance option, -P, must be specified when using -h.\n");
		return(-1);
	}
	if((args->flags & CLD_FLG_W) && !(args->flags & CLD_FLG_R) && (args->flags & CLD_FLG_CMPR)) {
		pMsg(WARN, args, "Write only, ignoring option -E.\n");
	}
	return(0);
}
