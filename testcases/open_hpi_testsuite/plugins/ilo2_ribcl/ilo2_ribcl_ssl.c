/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
 *                     All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the Hewlett-Packard Corporation, nor the names
 * of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s)
 *     Shuah Khan <shuah.khan@hp.com>
 *     Richard White <richard.white@hp.com>
 */

/*****************************
 * This file implements the iLO2 RIBCL plug-in iLO2 SSL connection
 * management functionality that is not part of the infrastructure.
 * It supports the following function:
 *
 * ilo2_ribcl_ssl_send_command	- Send a command, reading the response
*****************************/

/* TODO: Evaluate whether a shorter timeout value should be used.  This
 * affects calls to oh_ssl_connect(), oh_ssl_read(), and oh_ssl_write().
 */

/* Header files */
#include <oh_ssl.h>
#include <ilo2_ribcl_ssl.h>
#include <ilo2_ribcl_cmnds.h>


/**
 * ilo2_ribcl_ssl_send_command
 * @ir_handler: Ptr to this instance's private handler.
 * @cmnd_buf: Ptr to buffer containing the RIBCL command(s) to send.
 * @resp_buf: Ptr to buffer into which the response will be written.
 * @resp_buf_size: Size (in bytes) of the response buffer.
 *
 * This routine will send the contents of the RIBCL command buffer cmnd_buf
 * to the iLO2 addressed by the plugin private handler ir_handler. The response
 * from iLO2 will be stored in the null terminated character buffer pointed
 * to by resp_buf.
 *
 * Return values:
 * 0 success.
 * -1 failure.
 **/
int ilo2_ribcl_ssl_send_command( ilo2_ribcl_handler_t *ir_handler,
				 char *cmnd_buf, char *resp_buf, int resp_size)
{
	void *ssl_handler = NULL;
	int in_index;
	int ret;

	/* Zero out the response buffer */
	memset( resp_buf, 0, resp_size);

	ssl_handler = oh_ssl_connect( ir_handler->ilo2_hostport,
				ir_handler->ssl_ctx, 0);

	if( ssl_handler == NULL){
		err("ilo2_ribcl_ssl_send_command(): "
		    "oh_ssl_connect returned NULL.");
		return( -1);
	}

	/* Send the XML header. iLO2 requires the header to be sent ahead 
	   separately from the buffer containing the command. */

	ret = oh_ssl_write(ssl_handler, ILO2_RIBCL_XML_HDR,
				sizeof(ILO2_RIBCL_XML_HDR), 0);
	if( ret < 0){
		err("ilo2_ribcl_ssl_send_command(): "
		    "write of xml header to socket failed.");
		oh_ssl_disconnect(ssl_handler, OH_SSL_BI);
		return( -1);
	}

	/* Send the command buffer. */
	ret = oh_ssl_write(ssl_handler, cmnd_buf, strlen(cmnd_buf), 0);
	if( ret < 0){
		err("ilo2_ribcl_ssl_send_command(): "
		    "write of xml command to socket failed.");
		oh_ssl_disconnect(ssl_handler, OH_SSL_BI);
		return( -1);
	}

	ret = 0;
	in_index = 0;
	while( 1){
		ret = oh_ssl_read( ssl_handler,
			 &(resp_buf[in_index]), (resp_size - in_index), 0);
	
		if( ret <= 0){
			break;
		}

		in_index = in_index + ret;
	}
	resp_buf[in_index] = '\0';

	/* cleanup */
	oh_ssl_disconnect(ssl_handler, OH_SSL_BI);

	return( 0);

} /* end ilo2_ribcl_ssl_send_command */
