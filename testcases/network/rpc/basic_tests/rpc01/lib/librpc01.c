/*
 * Copyright (c) 2013 Linux Test Project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "rpc.h"
#include "librpc01.h"

bool_t xdr_receive_data(XDR *xdrs, struct data **buffer)
{
	struct data *bp;
	int i, rc;
	char *p;

	bp = *buffer = malloc(sizeof(struct data));
	rc = xdr_long(xdrs, &(bp->address));
	rc = rc && xdr_long(xdrs, &bp->request_id);
	rc = rc && xdr_long(xdrs, &bp->data_length);
	p = (*buffer)->data = malloc(bp->data_length);
	for (i = 0; rc && i < bp->data_length; p++, i++)
		rc = xdr_char(xdrs, p);
	return rc;
}

bool_t xdr_send_data(XDR *xdrs, struct data *buffer)
{
	int i, rc;
	char *p;

	rc = xdr_long(xdrs, &buffer->address);
	rc = rc && xdr_long(xdrs, &buffer->request_id);
	rc = rc && xdr_long(xdrs, &buffer->data_length);
	for (i = 0, p = buffer->data; rc && i < buffer->data_length; i++, p++)
		rc = xdr_char(xdrs, p);
	return rc;
}
