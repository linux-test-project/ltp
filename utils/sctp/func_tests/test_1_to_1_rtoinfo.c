/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test the getsockopt () and sectsockopt () with
 * SCTP_RTOINFO option on 1-1 style socket
 *
 * This program first gets the default values using getsockopt(). It also sets
 * the value using setsockopt() and gets the set value using getsockopt().
 * A comparison between set values and get values are performed.
 *
 * The SCTP implementation is free software;
 * you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * The SCTP implementation is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 ************************
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU CC; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Please send any bug reports or fixes you make to the
 * email address(es):
 *    lksctp developers <lksctp-developers@lists.sourceforge.net>
 *
 * Or submit a bug report through the following website:
 *    http://www.sf.net/projects/lksctp
 *
 * Any bugs reported given to us we will try to fix... any fixes shared will
 * be incorporated into the next SCTP release
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/in.h>         /* for sockaddr_in */
#include <linux/in6.h>         /* for sockaddr_in6 */
#include <sys/errno.h>
#include <sys/uio.h>
#include <netinet/sctp.h>
#include <sctputil.h>

char *TCID = __FILE__;
int TST_TOTAL = 3;
int TST_CNT = 0;

int 
main(void) 
{
	
	int sd, ret;
	socklen_t len;
	struct sctp_rtoinfo srtoinfo; /*setting the variables*/
	struct sctp_rtoinfo grtoinfo; /*Getting the variables*/

	sd = test_socket (PF_INET, SOCK_STREAM, IPPROTO_SCTP);

	len = sizeof(struct sctp_rtoinfo);
	
	/*TEST1 Getting the default values using getsockopt()*/
	ret = getsockopt(sd, IPPROTO_SCTP, SCTP_RTOINFO, &grtoinfo, &len);
	if (ret < 0)
		tst_brkm(TBROK, tst_exit, "getsockopt SCTP_RTOINFO "
			 "ret:%d, errno:%d", ret, errno);

	tst_resm(TPASS, "getsockopt() SCTP_RTOINFO - SUCCESS");

	/*Assigning the values to RTO initial and max and min bounds*/
	srtoinfo.srto_initial=60;
	srtoinfo.srto_max=100;
	srtoinfo.srto_min=40;

	/*TEST2 Setting the values using setsockopt()*/
	ret = setsockopt(sd, IPPROTO_SCTP, SCTP_RTOINFO, &srtoinfo, 
		sizeof(struct sctp_rtoinfo));
	if (ret < 0)
		tst_brkm(TBROK, tst_exit, "setsockopt SCTP_RTOINFO "
			 "ret:%d, errno:%d", ret, errno);

	tst_resm(TPASS, "setsockopt() SCTP_RTOINFO - SUCCESS");

	/*Getting the values which are set using setsockopt()*/
	ret = getsockopt(sd, IPPROTO_SCTP, SCTP_RTOINFO, &grtoinfo, &len);
	if (ret < 0)
		tst_brkm(TBROK, tst_exit, "getsockopt SCTP_RTOINFO "
			 "ret:%d, errno:%d", ret, errno);

	/* TEST3 Compare the get values with the set values. */ 
	if (srtoinfo.srto_initial != grtoinfo.srto_initial &&
            srtoinfo.srto_max != grtoinfo.srto_max &&
            srtoinfo.srto_min != grtoinfo.srto_min)
		tst_brkm(TBROK, tst_exit, "setsockopt/getsockopt SCTP_RTOINFO "
			 "compare failed");

	tst_resm(TPASS, "setsockopt()/getsockopt SCTP_RTOINFO compare - "
		 "SUCCESS");

	close(sd);

	return 0;
}
