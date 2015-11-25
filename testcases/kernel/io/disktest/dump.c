/*
* Disktest
* Copyright (c) International Business Machines Corp., 2001
*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Please send e-mail to yardleyb@us.ibm.com if you have
*  questions or comments.
*
*  Project Website:  TBD
*
* $Id: dump.c,v 1.7 2009/02/26 12:02:22 subrata_modak Exp $
*
*/
#include <stdio.h>		/* *printf() */
#include <string.h>		/* memset(), strn***() */
#include <ctype.h>		/* isprint() */
#include <stdlib.h>		/* malloc(), free() */

#include "defs.h"
#include "io.h"
#include "sfunc.h"
#include "dump.h"

int format_str(size_t iBytes, const char *ibuff, size_t ibuff_siz, char *obuff,
	       size_t obuff_siz)
{
	unsigned int i;
	char buff[10];
	static size_t TotalBytes = 0;

	if ((iBytes == 0) &&
	    (ibuff == NULL) && (ibuff_siz == 0) &&
	    (obuff == NULL) && (obuff_siz == 0)) {
		TotalBytes = 0;
		return 0;
	}

	if ((ibuff == NULL) || (obuff == NULL) || (iBytes < 1))
		return -1;

	memset(obuff, 0, obuff_siz);
	sprintf(buff, "%08lX", (long)TotalBytes);
	strncat(obuff, buff, (obuff_siz - 1) - strlen(obuff));
	for (i = 0; i < iBytes; i++) {
		if ((i % 4) == 0)
			strncat(obuff, " ", (obuff_siz - 1) - strlen(obuff));
		if ((i % 8) == 0)
			strncat(obuff, " ", (obuff_siz - 1) - strlen(obuff));
		sprintf(buff, "%02X ", *(ibuff + i));
		strncat(obuff, buff, (obuff_siz - 1) - strlen(obuff));
	}
	for (; i < ibuff_siz; i++) {
		if ((i % 4) == 0)
			strncat(obuff, " ", (obuff_siz - 1) - strlen(obuff));
		if ((i % 8) == 0)
			strncat(obuff, " ", (obuff_siz - 1) - strlen(obuff));
		strncat(obuff, "   ", (obuff_siz - 1) - strlen(obuff));
	}
	strncat(obuff, " ", (obuff_siz - 1) - strlen(obuff));
	for (i = 0; i < iBytes; i++) {
		sprintf(buff, "%c",
			(isprint(*(ibuff + i))) ? *(ibuff + i) : '.');
		strncat(obuff, buff, (obuff_siz - 1) - strlen(obuff));
	}
	TotalBytes += iBytes;
	return 0;
}

int format_raw(size_t iBytes, const char *ibuff, char *obuff, size_t obuff_siz)
{
	unsigned int i;
	char buff[10];
	static size_t TotalBytes = 0;

	if ((iBytes == 0) && (ibuff == NULL) &&
	    (obuff == NULL) && (obuff_siz == 0)) {
		TotalBytes = 0;
		return 0;
	}

	if ((ibuff == NULL) || (obuff == NULL) || (iBytes < 1))
		return -1;

	memset(obuff, 0, obuff_siz);
	sprintf(buff, "%08lX ", (long)TotalBytes);
	strncat(obuff, buff, (obuff_siz - 1) - strlen(obuff));
	for (i = 0; i < iBytes; i++) {
		sprintf(buff, "%02X", *(ibuff + i));
		strncat(obuff, buff, (obuff_siz - 1) - strlen(obuff));
	}
	TotalBytes += iBytes;
	return 0;
}

int dump_data(FILE * stream, const char *buff, const size_t buff_siz,
	      const size_t ofd_siz, const size_t offset, const int format)
{
	size_t TotalRemainingBytes, NumBytes, ibuff_siz, obuff_siz;
	char *ibuff, *obuff, *buff_curr;

	buff_curr = (char *)buff;
	buff_curr += offset;
	TotalRemainingBytes = buff_siz;
	NumBytes = 0;
	ibuff_siz = ofd_siz;
	obuff_siz =
	    12 + (3 * ibuff_siz) + (ibuff_siz / 4) + (ibuff_siz / 8) +
	    ibuff_siz;
	switch (format) {
	case FMT_STR:
		format_str(0, NULL, 0, NULL, 0);
		break;
	case FMT_RAW:
		format_raw(0, NULL, NULL, 0);
		break;
	default:
		return (-1);
	}

	if ((ibuff = (char *)ALLOC(ibuff_siz)) == NULL) {
		fprintf(stderr, "Can't allocate ibuff\n");
		return (-1);
	}
	if ((obuff = (char *)ALLOC(obuff_siz)) == NULL) {
		FREE(ibuff);
		fprintf(stderr, "Can't allocate obuff\n");
		return (-1);
	}

	while (TotalRemainingBytes > 0) {
		if (TotalRemainingBytes >= ibuff_siz) {
			memcpy(ibuff, buff_curr, ibuff_siz);
			TotalRemainingBytes -= ibuff_siz;
			NumBytes = ibuff_siz;
			buff_curr += NumBytes;
		} else {
			memcpy(ibuff, buff_curr, TotalRemainingBytes);
			NumBytes = TotalRemainingBytes;
			TotalRemainingBytes = 0;
		}
		switch (format) {
		case FMT_STR:
			format_str(NumBytes, ibuff, ibuff_siz, obuff,
				   obuff_siz);
			fprintf(stream, "%s\n", obuff);
			break;
		case FMT_RAW:
			format_raw(NumBytes, ibuff, obuff, obuff_siz);
			fprintf(stream, "%s\n", obuff);
			break;
		default:
			FREE(ibuff);
			FREE(obuff);
			return (-1);
		}
	}
	FREE(ibuff);
	FREE(obuff);
	return 0;
}

int do_dump(child_args_t * args)
{
	ssize_t NumBytes = 0;
	OFF_T TargetLBA, TotalBytes = 0;
	char *buff;
	fd_t fd;

	if ((buff = (char *)ALLOC(args->htrsiz * BLK_SIZE)) == NULL) {
		fprintf(stderr, "Can't allocate buffer\n");
		return (-1);
	}

	memset(buff, 0, args->htrsiz * BLK_SIZE);

	fd = Open(args->device, args->flags | CLD_FLG_R);
	if (INVALID_FD(fd)) {
		pMsg(ERR, args, "could not open %s.\n", args->device);
		pMsg(ERR, args, "%s: Error = %u\n", args->device,
		     GETLASTERROR());
		FREE(buff);
		return (-1);
	}

	TargetLBA = Seek(fd, args->start_lba * BLK_SIZE);
	if (TargetLBA != (args->start_lba * (OFF_T) BLK_SIZE)) {
		pMsg(ERR, args, "Could not seek to start position.\n");
		FREE(buff);
		CLOSE(fd);
		return (-1);
	}

	do {
		NumBytes = Read(fd, buff, args->htrsiz * BLK_SIZE);
		if ((NumBytes > args->htrsiz * BLK_SIZE) || (NumBytes < 0)) {
			pMsg(ERR, args, "Failure reading %s\n", args->device);
			pMsg(ERR, args, "Last Error was %lu\n", GETLASTERROR());
			break;
		}
		dump_data(stdout, buff, NumBytes, 16, 0, FMT_STR);
		TotalBytes += (OFF_T) NumBytes;
	} while ((TotalBytes < (args->htrsiz * BLK_SIZE)) && (NumBytes > 0));

	FREE(buff);
	CLOSE(fd);

	return 0;
}
