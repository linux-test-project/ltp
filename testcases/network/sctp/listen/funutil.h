/* Copyright (C) 1999 Cisco
 * Copyright (C) 1999-2000 Motorola
 * Copyright (C) 2001 International Business Machines Corp., 2001
 # Copyright (C) 2001 Nokia
 * Copyright (C) 2001 La Monte H.P. Yarroll
 * 
 * This file is part of the SCTP Linux kernel reference implementation
 * 
 * $Header: /cvsroot/ltp/ltp/testcases/network/sctp/listen/Attic/funutil.h,v 1.1 2002/06/20 18:58:20 robbiew Exp $
 * 
 * These functions populate the sctp protocol structure for sockets.
 * 
 * The SCTP reference implementation  is free software; 
 * you can redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * The SCTP reference implementation is distributed in the hope that it
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
 * Please send any bug reports or fixes you make to one of the following
 * email addresses:
 * 
 * La Monte H.P. Yarroll <piggy@acm.org>
 * Narasimha Budihal <narsi@refcode.org>
 * Karl Knutson <karl@athena.chicago.il.us>
 * Jon Grimm <jgrimm@us.ibm.com>
 * Daisy Chang <daisyc@us.ibm.com>
 * 
 * Any bugs reported given to us we will try to fix... any fixes shared will
 * be incorporealated into the next SCTP release.
 */
static char *cvs_id __attribute__ ((unused)) = "$Id: funutil.h,v 1.1 2002/06/20 18:58:20 robbiew Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h> /* for sockaddr_in */
#include <sys/errno.h>
#include <errno.h>
#include <netinet/sctp.h>

#include <errno.h> /* for sys_errlist[] */

/* This function prints the cmsg data. */
int
test_print_cmsg(sctp_cmsg_t type, sctp_cmsg_data_t *data)
{
	switch(type) {
	case SCTP_INIT:
		printf("INIT\n");
		printf("sinit_num_ostreams %d\n",
		       data->init.sinit_num_ostreams);
		printf("sinit_max_instreams %d\n",
		       data->init.sinit_max_instreams);
		printf("sinit_max_attempts %d\n",
		       data->init.sinit_max_attempts);
		printf("sinit_max_init_timeo %d\n",
		       data->init.sinit_max_init_timeo);
		
		break;
	case SCTP_SNDRCV:
		printf("SNDRCV\n");
		printf("sinfo_stream %u\n",	data->sndrcv.sinfo_stream);
		printf("sinfo_ssn %u\n",	data->sndrcv.sinfo_ssn);
		printf("sinfo_flags 0x%x\n",	data->sndrcv.sinfo_flags);
		printf("sinfo_ppid %u\n",	data->sndrcv.sinfo_ppid);
		printf("sinfo_context %x\n",	data->sndrcv.sinfo_context);
		
		break;
		
	default:
		printf("Unknown type: %d\n", type);
		break;
	}

	return 0;

} /* test_print_cmsg() */

/* This function prints the message. */
int
test_print_message(struct msghdr *msg, size_t msg_len) {
	struct msghdr *smsg = msg;
	struct SCTP_cmsghdr *cmsg = (struct SCTP_cmsghdr *)smsg->msg_control;
	sctp_cmsg_data_t *data = &cmsg->cmsg_data;
	struct cmsghdr *scmsg;
	int i;
	int done = 0;

	printf("\n\n****TEST PRINT MESSAGE****\n\n");
	for (scmsg = CMSG_FIRSTHDR(msg);
	     scmsg != NULL;
	     scmsg = CMSG_NXTHDR(msg, scmsg)) {
		     data = (sctp_cmsg_data_t *)CMSG_DATA(scmsg);
		     test_print_cmsg(scmsg->cmsg_type, data);
	}

	if (!(MSG_NOTIFICATION & smsg->msg_flags)) {
		int index = 0;
		/* Make sure that everything is printable and that we
		 * are NUL terminated...
		 */
		printf("Body:  ");
		while ( msg_len > 0 ) {
			char *text;
			int len;

			text = smsg->msg_iov[index].iov_base;
			len = smsg->msg_iov[index].iov_len;
                        
			if ( len > msg_len ) {
                                text[(len = msg_len) - 1] = '\0';
                        }

			if ( (msg_len -= len) > 0 ) { index++; }

			for (i = 0; i < len - 1; ++i) {
                                if (!isprint(text[i])) text[i] = '.';
                        }
		
			printf("%s", text);

			if ( (done = !strcmp(text, "exit")) ) { break; }
		}

		printf("\n");
	} /* if(we have DATA) */

	printf("\n\n****END TEST PRINT MESSAGE****\n\n");
	return done;

} /* test_print_message() */

