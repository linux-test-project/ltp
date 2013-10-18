/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *

 * Remember that you want to seperate your header
 * files between what is needed in kernel space
 * only, and what will also be needed by a user
 * space program that is using this module. For
 * that reason keep all structures that will need
 * kernel space pointers in a seperate header file
 * from where ioctl flags aer kept
 *
 * author: Sean Ruyle
 * date:   06/11/2003
 *
 */

struct ltpmod_user {

//put any pointers in here that might be needed by other ioctl calls

};
typedef struct ltpmod_user ltpmod_user_t;

