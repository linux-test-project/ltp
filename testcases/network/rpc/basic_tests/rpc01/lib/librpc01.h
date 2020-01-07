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

#ifndef __LIBRPC_H__
#define __LIBRPC_H__

#include "rpc.h"

struct data {
	long address;
	long request_id;
	long data_length;
	char *data;
};

bool_t xdr_receive_data(XDR *xdrs, struct data **buffer);
bool_t xdr_send_data(XDR *xdrs, struct data *buffer);

#endif /* __LIBRPC_H__ */
