/* dump_message.c                                                   */

/* SCTP kernel reference Implementation
 * Copyright (c) 2002 International Business Machines, Corp.
 * 
 * The SCTP reference implementation  is distributed in the hope that it 
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
 * This is a basic functional test for the SCTP kernel reference
 * implementation state machine.
 * 
 * Mingqin Liu <liuming@us.ibm.com>
 *
 * We use functions which approximate the user level API defined in
 * draft-ietf-tsvwg-sctpsocket-03.txt.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <sys/errno.h>

int dump_assoc_change (union sctp_notification * sn,  FILE * event_file) {
	int retval = 0;

	if(COMMUNICATION_UP == sn->sn_assoc_change.sac_state) {
		fprintf(event_file, "COMM UP\n");
	} 
	else if (COMMUNICATION_LOST == sn->sn_assoc_change.sac_state) {
		fprintf(event_file, "COMM LOST\n");
	} 
	else if (RESTART == sn->sn_assoc_change.sac_state) {
				fprintf(event_file, "RESTART\n");
	} 
	else if (SHUTDOWN_COMPLETE== sn->sn_assoc_change.sac_state) {
				fprintf(event_file, "SHUTDOWN COMPLETE\n");
	} 
	else if (CANT_START_ASSOC== sn->sn_assoc_change.sac_state) {
		fprintf(event_file, "CANNOT START ASSOC\n");
	} 
	else {
		fprintf(event_file, "unknown notification type\n");
		retval = 1;
	}
	return retval;
	
} /*dump_assoc_event */

int dump_peer_addr_change(union sctp_notification * sn,  FILE * event_file) {
	int retval = 0;

	if (ADDRESS_AVAILABLE == sn->sn_padr_change.spc_state) {

	} 
	else if (ADDRESS_REMOVED == sn->sn_padr_change.spc_state) {

	} 
	else if (ADDRESS_ADDED == sn->sn_padr_change.spc_state) {

	} 
	else if (ADDRESS_MADE_PRIM == sn->sn_padr_change.spc_state) {

	} 
	else {
		fprintf(event_file, "unknown notification type\n");
		retval = 1;
	}  
	return retval;
}/* dump_peer_addr_change  */

int  dump_remote_error(union sctp_notification * sn,  FILE * evt_file) {

	int retval = 0;
	return retval;
	
}/* dump_remote_error */

int dump_shutdown_event(union sctp_notification * sn,  FILE * evt_file) {
	int retval = 0;
	return retval;
	
}/* dump_shutdown_event */

int dump_adaption_indication(union sctp_notification * sn,  FILE * evt_file) {
	int retval = 0;
	return retval;
	
}/* dump_adaption_indication  */

int dump_partial_delivery_event(union sctp_notification * sn,  FILE * evt_file) {
	int retval = 0;
	return retval;
	
}/* dump_partial_delivery_event */

int dump_event (union sctp_notification * sn,  FILE * evt_file, int *size) { 
	int retval = 0;

	if (SCTP_ASSOC_CHANGE == sn->h.sn_type) {
	 	*size = sizeof (struct sctp_assoc_change);	
		return dump_assoc_change(sn, evt_file);
	
	} 
	else if (SCTP_PEER_ADDR_CHANGE== sn->h.sn_type) {
		*size = sizeof (struct sctp_paddr_change);
		return dump_peer_addr_change(sn, evt_file);

	}  
	else if (SCTP_REMOTE_ERROR== sn->h.sn_type) {
		*size = sizeof (struct sctp_remote_error);
		return dump_remote_error(sn, evt_file);		
	} 
	/*else if (SCTP_SEND_FAILED== sn->h.sn_type) {
		*size = sizeof (struct sctp_send_failed);
		return dump_send_failed(sn, evt_file);
	} */ 
	else if (SCTP_SHUTDOWN_EVENT== sn->h.sn_type) {
		*size = sizeof (struct sctp_shutdown_event);
		return dump_shutdown_event(sn, evt_file);	
	} 
	/*else if (SCTP_ADAPTION_INDICATION== sn->h.sn_type) {
		*size = sizeof (struct sctp_adaption_indication;
		return dump_adaption_indication(sn, evt_file);
	}
	else if (SCTP_PARTIAL_DELIVERY_EVENT== sn->h.sn_type) {
		*size = sizeof (struct sctp_partial_delivery_event);
		return dump_partial_delivery_event(sn, evt_file);
	}*/
	else {
		fprintf(stderr, "Unknown event:%i\n", sn->h.sn_type);
		retval = 1;
	}
	return retval;	
} /* dump_event */

/*
 * Name
 *     dump_message 
 *
 * Description
 *     Dump a message to given event file, data file or ancillary data file. 
 *     
 *
 * Parameters
 *     msg
 *         A pointer to a msghdr structure. The message to dump from.  
 *     msg_len
 *         The length of the message data.  
 *     data_file
 *         A FILE pointer, used to dump data to. 
 *     ctrl_file
 *         A FILE pointer, used to dump ancillary data to.
 *     event_fle
 *         A FILE pointer, used to dump notification to. 
 *
 * Return_value
 *     0 on success.
 *
 */

int dump_message(struct msghdr *msg, size_t msg_len) {

        int size, body_len;
        struct msghdr *smsg = msg;
        struct cmsghdr *scmsg;
        struct SCTP_cmsghdr *cmsg; 
        sctp_cmsg_t type; 
        sctp_cmsg_data_t *data; 
        union sctp_notification * sn;
        int retval = 0;
	FILE *log_file, *data_file, *ctrl_file, *event_file;

       	if (msg_len < sizeof (struct cmsghdr))  {
		printf("invalid message\n"); 
		return (-1);
	} 
	printf("sizeof (struct cmsghdr):%i\n", sizeof(struct cmsghdr));
	cmsg = (struct SCTP_cmsghdr *)smsg->msg_control;
	printf("get here\n");
	printf("cmsg: %s\n", (char*)cmsg);
	if (cmsg == NULL) 
		return 1;
	
        type = cmsg->cmsg_type;
        data = &(cmsg->cmsg_data);

	ctrl_file = log_file;
	event_file = log_file;
	data_file = log_file;

        body_len = msg_len;

        /*printf ("Ready to dump data\n"); */
        if (!(smsg->msg_flags & MSG_NOTIFICATION) && NULL != data_file) {
                int index = 0;
                /* Make sure that everything is printable and that we
                 * are NUL terminated...
                 */
                /*printf("body_len = %i\n", body_len);*/
                while ( body_len > 0 ) {
                        char *text;
                        int len;
                        
                        text = smsg->msg_iov[index].iov_base;
                        len = smsg->msg_iov[index].iov_len;
                       
                        if ( len > body_len ) {
                                text[(len = body_len) - 1] = '\0';
                        }

                        if ( (body_len -= len) > 0 ) { 
				index++; 
			}

                        fprintf(data_file, "%s", text);
                }

                /*fprintf(data_file, "\n"); */
        } /* if(we have DATA) */
        else if (MSG_NOTIFICATION & smsg->msg_flags && NULL != event_file) {
                
                int index = 0;
                char *text;
                int so_far = 0;
        
                text = malloc(msg_len);
                if (NULL == text) {
                        fprintf(log_file, "Cannot allocate memory");
                        return (-1);    
                } 

                fprintf(event_file, "Notification:  ");

                body_len = msg_len;
                while (body_len > 0 ) {
                        int len;
                        len = smsg->msg_iov[index].iov_len;
                        
                        if ( len > body_len ) {
                                len = body_len;
                        }
                        memcpy (text+so_far, 
                                smsg->msg_iov[index].iov_base, len);
                        so_far += len;
                        if ( (body_len -= len) > 0 ) { 
				index++; 
			}
                } /* while (msg_len > 0) */

                body_len = msg_len;
                do {    
                        sn = (union sctp_notification *)text;
                        if (0==(retval = dump_event(sn, event_file, &size))) {
                                body_len -= size;
                        } 
			else {
                                break;
                        }
                } while (body_len > 0);
                fprintf(event_file, "\n");
        
                free(text);
        
        } /* else if (MSG_NOTIFICATION & smsg->msg_flags) */ 
 
        return retval;
} /* dump_message */


