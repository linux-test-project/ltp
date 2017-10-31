/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LAPI_TCP_H__
#define LAPI_TCP_H__

#include <netinet/tcp.h>

#ifndef TCP_FASTOPEN
# define TCP_FASTOPEN	23
#endif

#ifndef TCP_FASTOPEN_CONNECT
# define TCP_FASTOPEN_CONNECT	30	/* Attempt FastOpen with connect */
#endif

#endif	/* LAPI_TCP_H__ */
