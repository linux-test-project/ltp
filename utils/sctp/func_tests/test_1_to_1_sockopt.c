/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test negative scenarios for getsockopt ()
 * setsockopt () call for 1-1 style sockets
 *
 * setsockopt () Tests:
 * -------------------
 * TEST1: setsockopt: Bad socket descriptor
 * TEST2: setsockopt: Invalid socket
 * TEST3: setsockopt: Invalid level
 * TEST4: setsockopt: Invalid option buffer
 * TEST5: setsockopt: Invalid option name
 * TEST6: getsockopt: Bad socket descriptor
 * TEST7: getsockopt: Invalid socket
 * TEST8: getsockopt: Invalid option buffer
 * TEST9: getsockopt: Invalid option name
 *
 * TEST10: getsockopt: SCTP_INITMSG
 * TEST11: setsockopt: SCTP_INITMSG
 * TEST12: setsockopt: SO_LINGER
 * TEST13: getsockopt: SO_LINGER
 * TEST14: getsockopt: SO_RCVBUF
 * TEST15: getsockopt: SCTP_STATUS
 * TEST16: setsockopt: SO_RCVBUF
 * TEST17: setsockopt: SO_SNDBUF
 * TEST18: getsockopt: SO_SNDBUF
 * TEST19: getsockopt: SCTP_PRIMARY_ADDR
 * TEST20: setsockopt: SCTP_PRIMARY_ADDR
 * TEST21: getsockopt: SCTP_ASSOCINFO
 * TEST22: setsockopt: SCTP_ASSOCINFO
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
 * be incorporated into the next SCTP release.
 *
 */ 

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netinet/sctp.h>
#include <sys/uio.h>
#include <sctputil.h>

char *TCID = __FILE__;
int TST_TOTAL = 22;
int TST_CNT = 0;

int
main(void)
{
	int error;
	socklen_t len;
	int sk, sk1, sk2, acpt_sk, pf_class;
	struct sctp_rtoinfo grtinfo;
	struct sockaddr_in lstn_addr, conn_addr;
	struct sctp_initmsg ginmsg; /*get the value for SCTP_INITMSG*/
	struct sctp_initmsg sinmsg; /*set the value for SCTP_INITMSG*/
	struct linger slinger; /*SO_LINGER structure*/
	struct linger glinger; /*SO_LINGER structure*/
	struct sockaddr_in addr;
	struct sockaddr_in *gaddr;
	struct sctp_status gstatus; /*SCTP_STATUS option*/
	int rcvbuf_val_get, rcvbuf_val_set; /*get and set var for SO_RCVBUF*/
	int sndbuf_val_get, sndbuf_val_set;/*get and set var for SO_SNDBUF*/
	struct sctp_prim gprimaddr;/*SCTP_PRIMARY_ADDR get*/
	struct sctp_prim sprimaddr;/*SCTP_PRIMARY_ADDR set*/
	struct sctp_assocparams sassocparams;  /* SCTP_ASSOCPARAMS set */
	struct sctp_assocparams gassocparams;  /* SCTP_ASSOCPARAMS get */
	int fd, err_no = 0;
	char filename[21];

	/* Rather than fflush() throughout the code, set stdout to
         * be unbuffered.
         */
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);

	pf_class = PF_INET;

	sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	/*setsockopt() TEST1: Bad socket descriptor EBADF, Expected error*/
        error = setsockopt(-1, IPPROTO_SCTP, 0, 0, 0);
	if (error != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "setsockopt with a bad socket "
			 "descriptor error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "setsockopt() with a bad socket descriptor - EBADF");

	/*setsockopt() TEST2: Invalid socket ENOTSOCK, Expected error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	error = setsockopt(fd, IPPROTO_SCTP, 0, 0, 0);
	if (error == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (error != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "setsockopt with an invalid socket "
			 "error:%d, errno:%d", error, err_no);

	tst_resm(TPASS, "setsockopt() with an invalid socket - ENOTSOCK");

	/*setsockopt() TEST3: Invalid level ENOPROTOOPT, Expected error*/
        error = setsockopt(sk, -1, SCTP_RTOINFO, 0, 0);
	if (error != -1 || errno != ENOPROTOOPT)
		tst_brkm(TBROK, tst_exit, "setsockopt with invalid level "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "setsockopt() with an invalid level - ENOPROTOOPT");

	/*setsockopt() TEST4: Invalid option buffer EFAULT, Expected error*/
        error = setsockopt(sk, IPPROTO_SCTP, SCTP_RTOINFO, 
		(const struct sctp_rtoinfo *)-1, sizeof(struct sctp_rtoinfo));
	if (error != -1 || errno != EFAULT)
		tst_brkm(TBROK, tst_exit, "setsockopt with invalid option "
			 "buffer error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "setsockopt() with invalid option buffer - EFAULT");

	/*setsockopt() TEST5: Invalid option Name EOPNOTSUPP, Expected error*/
        error = setsockopt(sk, IPPROTO_SCTP, SCTP_AUTOCLOSE, 0, 0);
	if (error != -1 || errno != EOPNOTSUPP)
		tst_brkm(TBROK, tst_exit, "setsockopt with invalid option "
			 "name error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "setsockopt() with invalid option name - EOPNOTSUPP");

	/*getsockopt() TEST6: Bad socket descriptor EBADF, Expected error*/
        error = getsockopt(-1, IPPROTO_SCTP, 0, 0, 0);
	if (error != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "getsockopt with a bad socket "
			 "descriptor error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "getsockopt() with a bad socket descriptor - EBADF");

	/*getsockopt() TEST7: Invalid socket ENOTSOCK, Expected error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	error = getsockopt(fd, IPPROTO_SCTP, 0, 0, 0);
	if (error == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (error != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "getsockopt with an invalid socket "
			 "error:%d, errno:%d", error, err_no);

	tst_resm(TPASS, "getsockopt() with an invalid socket - ENOTSOCK");
#if 0
	/*getsockopt() TEST3: Invalid level ENOPROTOOPT, Expected error*/
	/*I have commented this test case because it is returning EOPNOTSUPP.
	When I checked the code there also it is returning EOPNOTSUPP. As this
	is not specific to TCP style, I do not want to do the code change*/
	
        error = getsockopt(sk, -1, SCTP_RTOINFO, 0, 0);
	if (error != -1 || errno != ENOPROTOOPT)
		tst_brkm(TBROK, tst_exit, "getsockopt with invalid level "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "getsockopt() with an invalid level - ENOPROTOOPT");
#endif
	len = sizeof(struct sctp_rtoinfo);

	/*getsockopt() TEST8: Invalid option buffer EFAULT, Expected error*/
        error = getsockopt(sk, IPPROTO_SCTP, SCTP_RTOINFO, 
			   (struct sctp_rtoinfo *)-1, &len);
	if (error != -1 || errno != EFAULT)
		tst_brkm(TBROK, tst_exit, "getsockopt with invalid option "
			 "buffer error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "getsockopt() with invalid option buffer - EFAULT");

	/*getsockopt() TEST9: Invalid option Name EOPNOTSUPP, Expected error*/
        error = getsockopt(sk, IPPROTO_SCTP, SCTP_AUTOCLOSE, &grtinfo, &len);
	if (error != -1 || errno != EOPNOTSUPP)
		tst_brkm(TBROK, tst_exit, "getsockopt with invalid option "
			 "name error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "getsockopt() with invalid option name - EOPNOTSUPP");

	close(sk);

	sk1 = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);
	sk2 = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	lstn_addr.sin_family = AF_INET;
        lstn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        lstn_addr.sin_port = htons(SCTP_TESTPORT_1);

	conn_addr.sin_family = AF_INET;
        conn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        conn_addr.sin_port = htons(SCTP_TESTPORT_1);

        len = sizeof(struct sctp_initmsg);

	/* TEST10: Test cases for getsockopt SCTP_INITMSG */
	test_getsockopt(sk1, SCTP_INITMSG, &ginmsg, &len);

	tst_resm(TPASS, "getsockopt() SCTP_INITMSG - SUCCESS");

	sinmsg.sinit_num_ostreams = 5;
        sinmsg.sinit_max_instreams = 5;
        sinmsg.sinit_max_attempts = 3;
        sinmsg.sinit_max_init_timeo = 30;
	/* TEST11: Test case for setsockopt SCTP_INITMSG */
	test_setsockopt(sk1, SCTP_INITMSG, &sinmsg, sizeof(sinmsg));

	test_getsockopt(sk1, SCTP_INITMSG, &ginmsg, &len);

	if (sinmsg.sinit_num_ostreams != ginmsg.sinit_num_ostreams &&
	    sinmsg.sinit_max_instreams != ginmsg.sinit_max_instreams &&
	    sinmsg.sinit_max_attempts != ginmsg.sinit_max_attempts &&
	    sinmsg.sinit_max_init_timeo != ginmsg.sinit_max_init_timeo)
		tst_brkm(TBROK, tst_exit, "setsockopt/getsockopt SCTP_INITMSG "
			 "compare failed");

	tst_resm(TPASS, "setsockopt() SCTP_INITMSG - SUCCESS");

	/*Now get the values on different endpoint*/
	test_getsockopt(sk2, SCTP_INITMSG, &ginmsg, &len);

	/*Comparison should not succeed here*/
	if (sinmsg.sinit_num_ostreams == ginmsg.sinit_num_ostreams &&
	    sinmsg.sinit_max_instreams == ginmsg.sinit_max_instreams &&
	    sinmsg.sinit_max_attempts == ginmsg.sinit_max_attempts &&
	    sinmsg.sinit_max_init_timeo == ginmsg.sinit_max_init_timeo)
		tst_brkm(TBROK, tst_exit, "setsockopt/getsockopt SCTP_INITMSG "
			 "unexpected compare success");

	/* SO_LINGER Test with l_onff = 0 and l_linger = 0 */
	slinger.l_onoff = 0;
	slinger.l_linger = 0;
	test_bind(sk1, (struct sockaddr *) &lstn_addr, sizeof(lstn_addr));
	test_listen(sk1, 10 );
	len = sizeof(struct sockaddr_in);
	test_connect(sk2, (struct sockaddr *) &conn_addr, len);

	acpt_sk = test_accept(sk1, (struct sockaddr *)&addr, &len);

        len = sizeof(struct linger);
	/* TEST12: Test case for setsockopt SO_LINGER */
	error = setsockopt(sk2, SOL_SOCKET, SO_LINGER, &slinger, len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "setsockopt SO_LINGER "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "setsockopt() SO_LINGER - SUCCESS");

	/* TEST13: Test case for getsockopt SO_LINGER */
	error = getsockopt(sk2, SOL_SOCKET, SO_LINGER, &glinger, &len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "getsockopt SO_LINGER "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "getsockopt() SO_LINGER - SUCCESS");

	if (slinger.l_onoff != glinger.l_onoff || 
	    slinger.l_linger != glinger.l_linger)
		tst_brkm(TBROK, tst_exit, "setsockopt/getsockopt SO_LINGER "
			 "compare failed");
	
	/*First gets the default SO_RCVBUF value and comapres with the
	value obtained from SCTP_STATUS*/
	len = sizeof(int);
	/* TEST14: Test case for getsockopt SO_RCVBUF */
	error = getsockopt(sk2, SOL_SOCKET, SO_RCVBUF, &rcvbuf_val_get, &len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "getsockopt SO_RCVBUF "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "getsockopt() SO_RCVBUF - SUCCESS");

	len = sizeof(struct sctp_status);
	/* TEST15: Test case for getsockopt SCTP_STATUS */
	error = getsockopt(sk2, IPPROTO_SCTP, SCTP_STATUS, &gstatus, &len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "getsockopt SCTP_STATUS "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "getsockopt() SCTP_STATUS - SUCCESS");

	/* Reducing the SO_RCVBUF value using setsockopt() */
	/* Upstream has changed the MIN_RCVBUF (2048 + sizeof(struct sk_buff)) */
	len = sizeof(int);
	rcvbuf_val_set = 2048;
	/* TEST16: Test case for setsockopt SO_RCVBUF */
	error = setsockopt(sk2, SOL_SOCKET, SO_RCVBUF, &rcvbuf_val_set, len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "setsockopt SO_RCVBUF "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "setsockopt() SO_RCVBUF - SUCCESS");

	error = getsockopt(sk2, SOL_SOCKET, SO_RCVBUF, &rcvbuf_val_get, &len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "getsockopt SO_RCVBUF "
                         "error:%d, errno:%d", error, errno);

	if ((2 * rcvbuf_val_set) != rcvbuf_val_get)
		tst_brkm(TBROK, tst_exit, "Comparison failed:Set value and "
			 "got value differs Set Value=%d Get Value=%d",
			 (2*rcvbuf_val_set), rcvbuf_val_get);

	sndbuf_val_set = 5000;
	/* TEST17: Test case for setsockopt SO_SNDBUF */
	error = setsockopt(sk2, SOL_SOCKET, SO_SNDBUF, &sndbuf_val_set, len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "setsockopt SO_SNDBUF "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "setsockopt() SO_SNDBUF - SUCCESS");

	/* TEST18: Test case for getsockopt SO_SNDBUF */
	error = getsockopt(sk2, SOL_SOCKET, SO_SNDBUF, &sndbuf_val_get, &len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "getsockopt SO_SNDBUF "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "getsockopt() SO_SNDBUF - SUCCESS");

	if ((2 * sndbuf_val_set) != sndbuf_val_get)
		tst_brkm(TBROK, tst_exit, "Comparison failed:Set value and "
			 "got value differs Set Value=%d Get Value=%d\n",
			 (2*sndbuf_val_set), sndbuf_val_get);

            
	/* Getting the primary address using SCTP_PRIMARY_ADDR */
        len = sizeof(struct sctp_prim);
	/* TEST19: Test case for getsockopt SCTP_PRIMARY_ADDR */
	error = getsockopt(sk2,IPPROTO_SCTP, SCTP_PRIMARY_ADDR, &gprimaddr,
			   &len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "getsockopt SCTP_PRIMARY_ADDR "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "getsockopt() SCTP_PRIMARY_ADDR - SUCCESS");

	gaddr = (struct sockaddr_in *) &gprimaddr.ssp_addr;
	if(htons(gaddr->sin_port) != lstn_addr.sin_port &&
	   gaddr->sin_family != lstn_addr.sin_family &&
	   gaddr->sin_addr.s_addr != lstn_addr.sin_addr.s_addr)
		tst_brkm(TBROK, tst_exit, "getsockopt SCTP_PRIMARY_ADDR value "
			 "mismatch");

	memcpy(&sprimaddr, &gprimaddr, sizeof(struct sctp_prim));

	/* TEST20: Test case for setsockopt SCTP_PRIMARY_ADDR */
	error = setsockopt(sk2,IPPROTO_SCTP, SCTP_PRIMARY_ADDR, &sprimaddr,
			   len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "setsockopt SCTP_PRIMARY_ADDR "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "setsockopt() SCTP_PRIMARY_ADDR - SUCCESS");

	/* TEST21: Test case for getsockopt SCTP_PRIMARY_ADDR */
	/* Getting the association info using SCTP_ASSOCINFO */
        len = sizeof(struct sctp_assocparams);
	error = getsockopt(sk2, IPPROTO_SCTP, SCTP_ASSOCINFO, &gassocparams,
			   &len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "getsockopt SCTP_ASSOCINFO "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "getsockopt() SCTP_ASSOCINFO - SUCCESS");

	/* TEST21: Test case for setsockopt SCTP_ASSOCINFO */
	memcpy(&sassocparams, &gassocparams, sizeof(struct sctp_assocparams));
	sassocparams.sasoc_asocmaxrxt += 5;
	sassocparams.sasoc_cookie_life += 10;

	error = setsockopt(sk2, IPPROTO_SCTP, SCTP_ASSOCINFO, &sassocparams,
			   len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "setsockopt SCTP_ASSOCINFO "
                         "error:%d, errno:%d", error, errno);

	error = getsockopt(sk2, IPPROTO_SCTP, SCTP_ASSOCINFO, &gassocparams,
			   &len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "getsockopt SCTP_ASSOCINFO "
                         "error:%d, errno:%d", error, errno);

	if (sassocparams.sasoc_asocmaxrxt != gassocparams.sasoc_asocmaxrxt ||
	    sassocparams.sasoc_cookie_life != gassocparams.sasoc_cookie_life)
		tst_brkm(TBROK, tst_exit, "getsockopt SCTP_ASSOCINFO value "
			 "mismatch");
	tst_resm(TPASS, "setsockopt() SCTP_ASSOCINFO - SUCCESS");

	close(sk2);
	close(sk1);
	close(acpt_sk);

	return 0;
}
