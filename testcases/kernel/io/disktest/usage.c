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
* $Id: usage.c,v 1.5 2008/02/14 08:22:24 subrata_modak Exp $
*
*/

#include <stdio.h>

void usage(void)
{
	printf("\n");
	printf("\tdisktest [OPTIONS...] filespec\n");
	printf("\t-?\t\tDisplay this help text and exit.\n");
	printf("\t-a seed\t\tSets seed for random number generation.\n");
	printf("\t-A action\tSpecifies modified actions during runtime.\n");
	printf("\t-B lblk[:hblk]\tSet the block transfer size.\n");
	printf("\t-c\t\tUse a counting sequence as the data pattern.\n");
	printf
	    ("\t-C cycles\tRun until cycles disk access cycles are complete.\n");
	printf("\t-d\t\tDump data to standard out and exit.\n");
	printf("\t-D r%%:w%%\tDuty cycle used while reading and/or writing.\n");
	printf
	    ("\t-E cmp_len\tTurn on error checking comparing <cmp_len> bytes.\n");
	printf("\t-f byte\t\tUse a fixed data pattern up to 8 bytes.\n");
	printf("\t-F \t\tfilespec is a file describing a list of targets\n");
	printf
	    ("\t-h hbeat\tDisplays performance statistic every <hbeat> seconds.\n");
	printf("\t-I IO_type\tSet the data transfer type to IO_type.\n");
	printf("\t-K threads\tSet the number of test threads.\n");
	printf("\t-L seeks\tTotal number of seeks to occur.\n");
	printf("\t-m\t\tMark each LBA with header information.\n");
	printf("\t-M marker\tSpecify an alternate marker then start time.\n");
	printf("\t-n\t\tUse the LBA number as the data pattern.\n");
	printf("\t-N num_secs\tSet the number of available sectors.\n");
	printf("\t-o offset\tSet lba alignment offset.\n");
	printf("\t-p seek_pattern\tSet the pattern of disk seeks.\n");
	printf("\t-P perf_opts\tDisplays performance statistic.\n");
	printf("\t-q\t\tSuppress INFO level messages.\n");
	printf("\t-Q\t\tSuppress header information on messages.\n");
	printf("\t-r\t\tRead data from disk.\n");
	printf
	    ("\t-R rty[:dly]\tNumber of retries / retry delay after failure.\n");
	printf("\t-s sLBA[:eLBA]\tSet the start [and stop] test LBA.\n");
	printf("\t-S sblk[:eblk]\tSet the start [and stop] test block.\n");
	printf("\t-t dMin[:dMax][:ioTMO] set IO timing /timeout operations.\n");
	printf("\t-T runtime\tRun until <runtime> seconds have elapsed.\n");
	printf("\t-w\t\tWrite data to disk.\n");
	printf("\t-v\t\tDisplay version information and exit.\n");
	printf("\t-z\t\tUse randomly generated data as the data pattern.\n");
}
