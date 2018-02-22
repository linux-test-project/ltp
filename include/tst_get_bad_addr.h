/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
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

#ifndef TST_GET_BAD_ADDR_H__
#define TST_GET_BAD_ADDR_H__

/* Functions from lib/tst_get_bad_addr.c */
void *tst_get_bad_addr(void (*cleanup_fn) (void));

#endif	/* TST_GET_BAD_ADDR_H__ */
