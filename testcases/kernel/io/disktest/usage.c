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
*
* $Id: usage.c,v 1.4 2005/05/04 17:54:00 mridge Exp $
* $Log: usage.c,v $
* Revision 1.4  2005/05/04 17:54:00  mridge
* Update to version 1.2.8
*
* Revision 1.13  2005/01/08 21:18:34  yardleyb
* Update performance output and usage.  Fixed pass count check
*
* Revision 1.12  2004/11/02 20:47:13  yardleyb
* Added -F functions.
* lots of minor fixes. see README
*
* Revision 1.11  2002/03/07 03:33:11  yardleyb
* More cleanup to usage text.
*
* Revision 1.10  2001/12/04 19:22:19  yardleyb
* Removed -t option from usage
*
* Revision 1.9  2001/12/04 18:51:06  yardleyb
* Checkin of new source files and removal
* of outdated source
*
* Revision 1.8  2001/10/10 00:17:14  yardleyb
* Added Copyright and GPL license text.
* Miner bug fixes throughout text.
*
* Revision 1.7  2001/09/26 23:39:48  yardleyb
* Rearranged option of command line options.  Added -? and -v.
*
* Revision 1.6  2001/09/22 03:37:23  yardleyb
* Added new option text.
*
* Revision 1.5  2001/09/10 22:15:32  yardleyb
* Removed lots of text as it is now all in the man page.
*
* Revision 1.4  2001/09/06 21:59:23  yardleyb
* Fixed a bug in the -L/-K where it ther ewere to many childern be created.
* also more code cleanup.  Changed 'loops' to 'seeks' throughout.
*
* Revision 1.3  2001/09/06 18:23:30  yardleyb
* Added duty cycle -D.  Updated usage. Added
* make option to create .tar.gz of all files
*
* Revision 1.2  2001/09/05 22:44:42  yardleyb
* Split out some of the special functions.
* added O_DIRECT -Id.  Updated usage.  Lots
* of clean up to functions.  Added header info
* to pMsg.
*
* Revision 1.1  2001/09/04 19:28:08  yardleyb
* Split usage out. Split header out.  Added usage text.
* Made signal handler one function. code cleanup.
*
*
*/

#include <stdio.h>

void usage(void)
{
	printf("\n");
	printf("\tdisktest [OPTIONS...] filespec\n");
	printf("\t-?\t\tDisplay this help text and exit.\n");
	printf("\t-a seed\t\tsets seed for random number generation.\n");
	printf("\t-A\t\tAll threads will not die on error.\n");
	printf("\t-B lblk[:hblk]\tSet the block transfer size.\n");
	printf("\t-c\t\tUse a counting sequence as the data pattern.\n");
	printf("\t-C cycles\tRun until cycles disk access cycles are complete.\n");
	printf("\t-d\t\tDump data to standard out and exit.\n");
	printf("\t-D r%%:w%%\tDuty cycle used while reading and/or writing.\n");
	printf("\t-E cmp_len\tTurn on error checking comparing <cmp_len> bytes.\n");
	printf("\t-f byte\t\tUse a fixed data pattern up to 8 bytes.\n");
	printf("\t-h hbeat\tDisplays performance statistic every <hbeat> seconds.\n");
	printf("\t-I IO_type\tSet the data transfer type to IO_type.\n");
	printf("\t-K threads\tSet the number of test threads.\n");
	printf("\t-L seeks\tTotal number of seeks to occur.\n");
	printf("\t-m mark\t\tMark blocks with LBA and pass count\n");
	printf("\t-n\t\tUse the LBA number as the data pattern.\n");
	printf("\t-N num_secs\tSet the number of available sectors.\n");
	printf("\t-p seek_pattern\tSet the pattern of disk seeks.\n");
	printf("\t-P perf_opts\tDisplays performance statistic.\n");
	printf("\t-q\t\tSepress INFO level messages.\n");
	printf("\t-Q\t\tSepress header information on messages.\n");
	printf("\t-r\t\tRead data from disk.\n");
	printf("\t-s sLBA[:eLBA]\tSet the start [and stop] test LBA.\n");
	printf("\t-S sblk[:eblk]\tSet the start [and stop] test block.\n");
	printf("\t-T runtime\tRun until <runtime> seconds have elapsed.\n");
	printf("\t-w\t\tWrite data to disk.\n");
	printf("\t-v\t\tDisplay version information and exit.\n");
	printf("\t-z\t\tUse randomly generated data as the data pattern.\n");
}
