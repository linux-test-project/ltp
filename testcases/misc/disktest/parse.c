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
* $Id: parse.c,v 1.1 2002/02/19 21:29:26 robbiew Exp $
* $Log: parse.c,v $
* Revision 1.1  2002/02/19 21:29:26  robbiew
* Added disktest.
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
#include "main.h"
#include "usage.h"
#include "sfunc.h"
#include "parse.h"

void fill_cld_args(int argc, char **argv, child_args_t *args)
{
	char c;
	char *leftovers;
	extern char *optarg;
	extern int optind;
	extern unsigned long glb_flags;
	extern size_t seed;

	while((c = getopt(argc, argv, "?dqD:L:a:zC:I:s:S:P:f:T:K:cnrh:wot:E:N:m:B:p:v")) != -1) {
		switch(c) {
			case ':' :
				pMsg(WARN, "Missing argument for perameter.\n");
				usage();
				exit(1);
			case 'a' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, "-%c arguments is non numeric.\n", c);
					exit(1);
				}
				seed = strtol(optarg,NULL,0);
				break;
			case 'd' :
				args->flags &= ~CLD_FLG_ALLDIE;
				break;
			case 'q' :
				glb_flags |= GLB_FLG_QUIET;
				break;
			case 'v' :
				pMsg(INFO, "%s\n", VER_STR);
				exit(0);
			case 'p' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				/* seek pattern type */
				if (strchr(optarg,'L'))
					args->flags |= CLD_FLG_LINEAR;
				else if (strchr(optarg,'l'))
					args->flags |= (CLD_FLG_LINEAR|CLD_FLG_NTRLVD);
				else if (strchr(optarg,'R') || strchr(optarg,'r'))
					args->flags |= CLD_FLG_RANDOM;
				else {
					pMsg(WARN, "Unknown Seek pattern\n");
					usage();
					exit(1);
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
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, "-%c arguments is non numeric.\n", c);
					exit(1);
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
						pMsg(WARN, "-%c arguments is non numeric.\n", c);
						exit(1);
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
				break;
			case 'c' :
				if(args->flags & CLD_FLG_PTYPS) {
					pMsg(WARN, "Please specify only one pattern type\n");
					usage();
					exit(1);
				}
				args->flags |= CLD_FLG_CPTYPE;
				break;
			case 'n' :
				if(args->flags & CLD_FLG_PTYPS) {
					pMsg(WARN, "Please specify only one pattern type\n");
					usage();
					exit(1);
				}
				args->flags |= CLD_FLG_LPTYPE;
				break;
			case 'f' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if(args->flags & CLD_FLG_PTYPS) {
					pMsg(WARN, "Please specify only one pattern type\n");
					usage();
					exit(1);
				}
				args->pattern = my_strtofft(optarg);
				args->flags |= CLD_FLG_FPTYPE;
				break;
			case 'z' :
				if(args->flags & CLD_FLG_PTYPS) {
					pMsg(WARN, "Please specify only one pattern type\n");
					usage();
					exit(1);
				}
				args->flags |= CLD_FLG_RPTYPE;
				break;
			case 'h' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, "-%c arguments is non numeric.\n", c);
					usage();
					exit(1);
				}
				args->hbeat = atoi(optarg);
				break;
			case 'D' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, "-%c arguments is non numeric.\n", c);
					usage();
					exit(1);
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
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				args->retries = atoi(optarg);
				pMsg(ERR, "-%c not implemented...\n", c);
				exit(1);
				break;
			case 'm' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if (strchr(optarg,'L') || strchr(optarg,'l'))
					args->mrk_flag |= MARK_LAST;
				else if (strchr(optarg,'F') || strchr(optarg,'f'))
					args->mrk_flag |= MARK_FIRST;
				else if (strchr(optarg,'A') || strchr(optarg,'a'))
					args->mrk_flag |= MARK_ALL;
				else {
					pMsg(WARN, "Unknown mark type.\n");
					exit(1);
				}
				args->flags |= CLD_FLG_MBLK;
				break;
			case 'E' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, "-%c arguments are non numeric.\n", c);
					usage();
					exit(1);
				}
				args->flags |= CLD_FLG_CMPR;
				args->cmp_lng = strtol(optarg,NULL,0);
				break;
			case 'N' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, "-%c arguments are non numeric.\n", c);
					exit(1);
				}
				args->vsiz = my_strtofft(optarg);
				if(strchr(optarg,'k')) {	/* multiply by 2^10 */
					args->vsiz <<= 10;
				} else if(strchr(optarg,'K')) { /* multiply 10^3 */
					args->vsiz *= 1000;
				} else if(strchr(optarg,'m')) { /* multiply by 2^20 */
					args->vsiz <<= 20;
				} else if(strchr(optarg,'M')) { /* multiply by 10^6 */
					args->vsiz *= 1000000;
				}
				break;
			case 'I' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if (strchr(optarg,'R') || strchr(optarg,'r')) {
					if (!(args->flags & CLD_FLG_BLK) &&
					    !(args->flags & CLD_FLG_FILE)) {
						args->flags |= CLD_FLG_RAW;
					} else {
						pMsg(WARN, "Can only specify one IO type\n");
						exit(1);
					}
				}
				if (strchr(optarg,'B') || strchr(optarg,'b')) {
					if (!(args->flags & CLD_FLG_RAW) &&
					    !(args->flags & CLD_FLG_FILE)) {
						args->flags |= CLD_FLG_BLK;
					} else {
						pMsg(WARN, "Can only specify one IO type\n");
						exit(1);
					}
				}
				if (strchr(optarg,'F') || strchr(optarg,'f')) {
					if (!(args->flags & CLD_FLG_RAW) &&
					    !(args->flags & CLD_FLG_BLK)) {
						args->flags |= CLD_FLG_FILE;
					} else {
						pMsg(WARN, "Can only specify one IO type\n");
						exit(1);
					}
				}
				if (strchr(optarg,'D') || strchr(optarg,'d'))
					args->flags |= CLD_FLG_DIRECT;
				break;
			case 'T' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				args->run_time = atoi(optarg);
				args->flags |= CLD_FLG_TMD;
				break;
			case 'L' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				args->seeks = atoi(optarg);
				args->flags |= CLD_FLG_SKS;
				break;
			case 'C' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, "-%c arguments is non numeric.\n", c);
					usage();
					exit(1);
				}
				args->flags |= CLD_FLG_CYC;
				args->cycles = atol(optarg);
				break;
			case 'K' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, "-%c arguments is non numeric.\n", c);
					usage();
					exit(1);
				}
				args->t_kids = atoi(optarg);
				break;
			case 'P' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
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
				if (strchr(optarg,'A')) {
					args->flags |= CLD_FLG_XFERS;
					args->flags |= CLD_FLG_TPUTS;
					args->flags |= CLD_FLG_RUNT;
				}
				if (!strchr(optarg,'P') &&
						!strchr(optarg,'A') &&
						!strchr(optarg,'X') &&
						!strchr(optarg,'R') &&
						!strchr(optarg,'T')) {
					pMsg(WARN, "Unknown performance option\n");
					exit(1);
				}
				break;
			case 'S' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, "-%c arguments is non numeric.\n", c);
					exit(1);
				}
				/* start and stop are in terms of trsiz */
				args->start_blk = my_strtofft(optarg);
				if(strchr(optarg,':') != NULL)
					args->stop_blk =
						my_strtofft((char *)(strchr(optarg,':')+1));
				args->flags |= CLD_FLG_BLK_RNG;
				break;
			case 's' :
				if(optarg == NULL) {
					pMsg(WARN, "-%c option requires an argument.\n", c);
					exit(1);
				}
				if(!isdigit(optarg[0])) {
					pMsg(WARN, "-%c arguments are non numeric.\n", c);
					exit(1);
				}
				/* start and stop are in terms of trsiz */
				args->start_lba = my_strtofft(optarg);
				if(strchr(optarg,':') != NULL) {
					args->stop_lba = my_strtofft((char *)(strchr(optarg,':')+1));
					args->flags |= CLD_FLG_LBA_RNG;
					if(strchr((char *)(strchr(optarg,':')+1),'k')) {	/* multiply by 2^10 */
						args->stop_lba <<= 10;
					} else if(strchr((char *)(strchr(optarg,':')+1),'K')) { /* multiply 10^3 */
						args->stop_lba *= 1000;
					} else if(strchr((char *)(strchr(optarg,':')+1),'m')) { /* multiply by 2^20 */
						args->stop_lba <<= 20;
					} else if(strchr((char *)(strchr(optarg,':')+1),'M')) { /* multiply by 10^6 */
						args->stop_lba *= 1000000;
					}
				}
				break;
			case '?' :
				usage();
				exit(1);
			default  :
				pMsg(WARN, "Unknown option.\n");
				usage();
				exit(1);
		}
	}
	if(argv[optind] == NULL) {
		pMsg(WARN, "Unspecified target.\n");
		usage();
		exit(1);
	}
	args->device = argv[optind];
}

void make_assumptions(child_args_t *args)
{
	char *device = (char *) malloc(strlen(args->device)+1);

	if(!(args->flags & CLD_FLG_IOTYPS)) {
		strncpy(device, args->device, strlen(args->device));
		if(strstr(strlwr(device), "raw") != NULL) {
			pMsg(INFO,"Assuming raw IO, use -I to specify.\n");
			args->flags |= CLD_FLG_RAW;
		} else if(strstr(strupr(device), BLKDEVICESTR) != NULL) {
			pMsg(INFO,"Assuming block IO, use -I to specify.\n");
			args->flags |= CLD_FLG_BLK;
		} else {
			pMsg(INFO,"Assuming file IO, use -I to specify.\n");
			args->flags |= CLD_FLG_FILE;
		}
	}
	free(device);

	if((args->flags & CLD_FLG_BLK_RNG) && (args->flags & CLD_FLG_RTRSIZ)) {
		pMsg(WARN, "Can't have unfixed block sizes and specify seek range in terms of blocks.\n");
		exit(1);
	}
	if(args->ltrsiz <= 0) {
		pMsg(INFO,"Assuming transfer size of %ld, use -B to specify.\n", TRSIZ*BLK_SIZE);
		args->ltrsiz=TRSIZ;
		args->htrsiz=TRSIZ;
	}
	if(args->flags & CLD_FLG_BLK_RNG) {
		args->start_lba += args->start_blk * args->ltrsiz;
		args->stop_lba += args->stop_blk * args->ltrsiz;
		if(!(args->flags & CLD_FLG_LBA_RNG)) {
			args->stop_lba++; /* we init stop_lba to -1 so remove that here */
		}
	}
	if(args->vsiz <= 0) {
		if(args->flags & CLD_FLG_BLK) {
			args->vsiz=get_vsiz(args->device);
		} else if((args->flags & CLD_FLG_LBA_RNG) || (args->flags & CLD_FLG_BLK_RNG)) {
			args->vsiz=args->stop_lba-args->start_lba+1;
		} else {
			args->vsiz=VSIZ;
		}
		pMsg(INFO,"Assuming volume size of %ld sectors, use -N to specify.\n", args->vsiz);
	}
	if(args->stop_lba == -1) {
		args->stop_lba=args->vsiz;
	}
	if(args->t_kids == 0) {
		pMsg(INFO,"Assuming %d childern, use -K to specify.\n", KIDS);
		args->t_kids=KIDS;
	}
	if((args->flags & (CLD_FLG_W|CLD_FLG_R)) == 0) {
		pMsg(INFO,"Assuming read, use -r, -w to specify.\n");
		args->flags |= CLD_FLG_R;
	}
	if(!(args->flags & CLD_FLG_PTYPS)) {
		pMsg(INFO,"Assuming counting pattern, use -z, -n, -c, or -f to specify.\n");
		args->flags |= CLD_FLG_CPTYPE;
	}
	if(!(args->flags & CLD_FLG_SKTYPS)) {
		pMsg(INFO,"Assuming random seek, use -p to specify.\n");
		args->flags |= CLD_FLG_RANDOM;
	}
	if(!(args->flags & CLD_FLG_SKS)) {
		args->seeks = (args->stop_lba - args->start_lba) ?
			(args->stop_lba - args->start_lba) : SEEKS;
		args->seeks /= args->htrsiz;  /* calculated seeks are in terms of the largest transfer size */
		if((args->flags & CLD_FLG_LINEAR) && (args->flags & CLD_FLG_R) && (args->flags & CLD_FLG_W)) {
			pMsg(INFO, "Doubling assumed seeks, use -L to specify\n");
			args->seeks *= 2;
		}
		pMsg(INFO,"Assuming %d seeks, use -C, -L, or -T to specify.\n", args->seeks);
	}
	if(!(args->flags & (CLD_FLG_CYC|CLD_FLG_SKS|CLD_FLG_TMD))) {
		args->flags |= CLD_FLG_SKS;
	}
	if(args->flags & (CLD_FLG_LINEAR)) {
		if(!(args->flags & (CLD_FLG_LUNU|CLD_FLG_LUND))) {
			pMsg(INFO,"Assuming start/end then start/end seeking, use -p to specify\n");
			args->flags |= CLD_FLG_LUNU;
		}
	}
	if(!(args->flags & CLD_FLG_DUTY)) {
		args->flags |= CLD_FLG_DUTY;
		if((args->flags & CLD_FLG_W) & !(args->flags & CLD_FLG_R)) /* only writes */
			args->wperc = 100;
		else if(!(args->flags & CLD_FLG_W) & (args->flags & CLD_FLG_R)) /* only reads */
			args->rperc = 100;
	}
}

/*
 * checks validity of data after parsing
 * args and make assumtions. returns 0 on
 * success and -1 on failure.
 */
int check_conclusions(child_args_t *args)
{
	if((args->vsiz < 0) || (args->ltrsiz < 1) || (args->htrsiz < 1)) {
		pMsg(WARN, "Bounds exceeded for transfer size and/or volume size.\n");
		pMsg(WARN, "Max transfer size is %lu and Volume size is (%ld)\n",(args->htrsiz*BLK_SIZE),args->vsiz);
		return(-1);
	}
	if((args->flags & CLD_FLG_SKS) && (args->t_kids > args->seeks)) {
		pMsg(WARN, "Can't have more children then max number of seeks, use -K/-L to adjust.\n");
		return(-1);
	}
	if(args->start_blk > args->stop_blk) {
		pMsg(WARN, "Stop Block of range must be greater the Start Block\n");
		return(-1);
	}
	if(args->start_lba > args->stop_lba) {
		pMsg(WARN, "Stop LBA of range must be greater the Start LBA\n");
		return(-1);
	}
	if((args->flags & CLD_FLG_DIRECT) && ((args->htrsiz%2) != 0)) {
		pMsg(WARN, "Transfer size not compatable with O_DIRECT\n");
		return(-1);
	}
	if((args->flags & CLD_FLG_FILE) && (strstr(args->device,"dev") != NULL)) {
		pMsg(WARN, "Can't open block device with file I/O type\n");
		return(-1);
	}
	if((args->flags & CLD_FLG_BLK) && (strstr(args->device,"raw") != NULL)) {
		pMsg(WARN, "Raw device filespec for Block I/O type\n");
		return(-1);
	}
	if((args->flags & CLD_FLG_RAW) && (strstr(args->device,"raw") == NULL)) {
		pMsg(WARN, "Device not a raw device or not in /dev/raw\n");
		return(-1);
	}
	if((args->hbeat > 0) && (args->flags & CLD_FLG_TMD) && (args->hbeat > args->run_time)) {
		pMsg(WARN, "Heartbeat should be at least equal to runtime, use -h/-T to adjust\n");
		return(-1);
	}
	return(0);
}
