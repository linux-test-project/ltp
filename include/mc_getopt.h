
/* $Header: /cvsroot/ltp/ltp/include/Attic/mc_getopt.h,v 1.1 2000/07/27 17:13:18 alaffin Exp $ */

/*
 *	(C) COPYRIGHT CRAY RESEARCH, INC.
 *	UNPUBLISHED PROPRIETARY INFORMATION.
 *	ALL RIGHTS RESERVED.
 */

#ifndef __MC_GETOPT_H__
#define __MC_GETOPT_H__ 1

/*
 * Possible return value of mc_getopt
 */

#define MC_UNKNOWN_OPTION       "UNKNOWN_OPTION"
#define MC_AMBIGUOUS_OPTION     "AMBIGUOUS_OPTION"
#define MC_MISSING_OPTARG       "MISSING_OPTARG"
#define MC_DONE                 NULL

/*
 * Mc_getopt flags argument bits
 */
#define MC_FULL_TOKEN_MATCH     01
#define MC_CASE_INSENSITIVE     02

extern int   mc_optind;
extern char *mc_optarg;
extern char *mc_optopt;

/*
 * Prototypes
 */

extern char *mc_getopt(int argc, char * const argv[], int flags, const char *optstring);
extern char *mc_getoptv(int argc, char * const argv[], int flags, int nopts, char * const opt_arr[]);

#endif  /* __MC_GETOPT_H__ */

