/*
 * memdrain.c
 *
 * DTT helper applications
 *
 * Copyright (C) 2005  NTT Data Corporation
 *
 * Author: Fernando Luis Vazquez Cao
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Contact information:
 * Fernando Luis Vazquez Cao <fernando@intellilink.co.jp>
 * NTT Data Intellilink Corporation, Kayaba-cho Tower (2nd floor),
 * 1-21-2 Shinkawa, Chuo-ku, Tokyo 104-0033, Japan
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MEGABYTE 1024*1024
#define iter 1000

int main (int argc, char **argv) {
	void *tmp;
	int i;

	printf("ENTERING memdrain\n");

	for (i = 0; i < iter; i++) {
		printf("Allocating memory\n");
		tmp = malloc(100*MEGABYTE);
		if (tmp == NULL) {
			printf("Unable to allocate the requested memory\n");
			exit(-1);
		}
		memset(tmp, i, 100*MEGABYTE);
		getchar();
	}
	
	printf("LEAVING memdrain\n");
	
	return 0;
}
