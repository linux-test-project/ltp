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
* $Id: dump.c,v 1.3 2005/05/04 17:54:00 mridge Exp $
* $Log: dump.c,v $
* Revision 1.3  2005/05/04 17:54:00  mridge
* Update to version 1.2.8
*
* Revision 1.5  2004/11/02 20:47:13  yardleyb
* Added -F functions.
* lots of minor fixes. see README
*
* Revision 1.4  2003/09/12 18:10:08  yardleyb
* Updated to version 1.11
* Code added to fix compile
* time warnings
*
* Revision 1.3  2002/04/03 20:18:29  yardleyb
* Added case for failed read in
* do_dump.  Set initialization for
* count and buffer in do_dump
*
* Revision 1.2  2002/03/30 01:32:14  yardleyb
* Major Changes:
*
* Added Dumping routines for
* data miscompares,
*
* Updated performance output
* based on command line.  Gave
* one decimal in MB/s output.
*
* Rewrote -pL IO routine to show
* correct stats.  Now show pass count
* when using -C.
*
* Minor Changes:
*
* Code cleanup to remove the plethera
* if #ifdef for windows/unix functional
* differences.
*
* Revision 1.1  2002/03/07 03:38:52  yardleyb
* Added dump function from command
* line.  Created formatted dump output
* for Data miscomare and command line.
* Can now leave off filespec the full
* path header as it will be added based
* on -I.
*
*/
#include <stdio.h>	/* *printf() */
#include <string.h>	/* memset(), strn***() */
#include <ctype.h>	/* isprint() */
#include <stdlib.h>	/* malloc(), free() */

#include "defs.h"
#include "io.h"
#include "sfunc.h"
#include "dump.h"

int format_str(size_t iBytes, const unsigned char *ibuff, size_t ibuff_siz, unsigned char *obuff, size_t obuff_siz)
{
	unsigned int i;
	unsigned char buff[10];
	static size_t TotalBytes = 0;

	if((iBytes == 0) &&
	   (ibuff == NULL) && (ibuff_siz == 0) &&
	   (obuff == NULL) && (obuff_siz == 0)) {
		TotalBytes = 0;
		return 0;
	}

	if((ibuff == NULL) || (obuff == NULL) || (iBytes < 1)) return -1;

	memset(obuff, 0, obuff_siz);
	sprintf(buff,"%08lX", (long)TotalBytes);
	strncat(obuff, buff, (obuff_siz-1)-strlen(obuff));
	for(i=0;i<iBytes;i++) {
		if((i%4) == 0) strncat(obuff, " ", (obuff_siz-1)-strlen(obuff));
		if((i%8) == 0) strncat(obuff, " ", (obuff_siz-1)-strlen(obuff));
		sprintf(buff,"%02X ", *(ibuff+i));
		strncat(obuff, buff, (obuff_siz-1)-strlen(obuff));
	}
	for(;i<ibuff_siz;i++) {
		if((i%4) == 0) strncat(obuff, " ", (obuff_siz-1)-strlen(obuff));
		if((i%8) == 0) strncat(obuff, " ", (obuff_siz-1)-strlen(obuff));
		strncat(obuff, "   ", (obuff_siz-1)-strlen(obuff));
	}
	strncat(obuff, " ", (obuff_siz-1)-strlen(obuff)); 
	for(i=0;i<iBytes;i++) {
		sprintf(buff, "%c", (isprint(*(ibuff+i))) ? *(ibuff+i) : '.');
		strncat(obuff, buff, (obuff_siz-1)-strlen(obuff));
	}
	TotalBytes += iBytes;
	return 0;
}

int format_raw(size_t iBytes, const unsigned char *ibuff, unsigned char *obuff, size_t obuff_siz)
{
	unsigned int i;
	unsigned char buff[10];
	static size_t TotalBytes = 0;

	if((iBytes == 0) && (ibuff == NULL) &&
	   (obuff == NULL) && (obuff_siz == 0)) {
		TotalBytes = 0;
		return 0;
	}

	if((ibuff == NULL) || (obuff == NULL) || (iBytes < 1)) return -1;

	memset(obuff, 0, obuff_siz);
	sprintf(buff,"%08lX ", (long)TotalBytes);
	strncat(obuff, buff, (obuff_siz-1)-strlen(obuff));
	for(i=0;i<iBytes;i++) {
		sprintf(buff,"%02X", *(ibuff+i));
		strncat(obuff, buff, (obuff_siz-1)-strlen(obuff));
	}
	TotalBytes += iBytes;
	return 0;
}

int dump_data(FILE *stream, const unsigned char *buff, const size_t buff_siz, const size_t ofd_siz, const int format)
{
	size_t TotalRemainingBytes, NumBytes, ibuff_siz, obuff_siz;
	unsigned char *ibuff, *obuff, *buff_curr;

	buff_curr = (unsigned char *) buff;
	TotalRemainingBytes = buff_siz;
	NumBytes = 0;
	ibuff_siz = ofd_siz;
	obuff_siz = 12+(3*ibuff_siz)+(ibuff_siz/4)+(ibuff_siz/8)+ibuff_siz;
	switch(format) {
		case FMT_STR:
			format_str(0, NULL, 0, NULL, 0);
			break;
		case FMT_RAW:
			format_raw(0, NULL, NULL, 0);
			break;
		default:
			return(-1);
	}

	if((ibuff = (unsigned char *) ALLOC(ibuff_siz)) == NULL) {
		fprintf(stderr, "Can't allocate ibuff\n");
		return(-1);
	}
	if((obuff = (unsigned char *) ALLOC(obuff_siz)) == NULL) {
		FREE(ibuff);
		fprintf(stderr, "Can't allocate obuff\n");
		return(-1);
	}

	while(TotalRemainingBytes > 0) {
		if(TotalRemainingBytes >= ibuff_siz) {
			memcpy(ibuff, buff_curr, ibuff_siz);
			TotalRemainingBytes -= ibuff_siz;
			NumBytes = ibuff_siz;
			buff_curr += NumBytes;
		} else {
			memcpy(ibuff, buff_curr, TotalRemainingBytes);
			NumBytes = TotalRemainingBytes;
			TotalRemainingBytes = 0;
		}
		switch(format) {
			case FMT_STR:
				format_str(NumBytes, ibuff, ibuff_siz, obuff, obuff_siz);
				fprintf(stream, "%s\n", obuff);
				break;
			case FMT_RAW:
				format_raw(NumBytes, ibuff, obuff, obuff_siz);
				fprintf(stream, "%s\n", obuff);
				break;
			default:
				return(-1);
		}
	}
	FREE(ibuff);
	FREE(obuff);
	return(0);
}

int do_dump(child_args_t *args)
{
	size_t NumBytes = 0;
	OFF_T TargetLBA, TotalBytes = 0;
	unsigned char buff[512];
	fd_t fd;

	memset(buff, 0, 512);

	fd = Open(args->device, args->flags | CLD_FLG_R);
	if(INVALID_FD(fd)) {
		pMsg(ERR, args, "could not open %s.\n",args->device);
		pMsg(ERR, args, "%s: Error = %u\n",args->device, GETLASTERROR());
		return(-1);
	}

	TargetLBA = Seek(fd, args->start_lba*BLK_SIZE);
	if(TargetLBA != (args->start_lba * (OFF_T) BLK_SIZE)) {
		pMsg(ERR, args, "Could not seek to start position.\n");
		CLOSE(fd);
		return(-1);
	}

	do {
		NumBytes = Read(fd, buff, 512);
		if((NumBytes > 512) || (NumBytes < 0)) {
			pMsg(ERR, args, "Failure reading %s\n", args->device);
			pMsg(ERR, args, "Last Error was %lu\n", GETLASTERROR());
			break;
		}
		dump_data(stdout, buff, NumBytes, 16, FMT_STR);
		TotalBytes += (OFF_T) NumBytes;
	} while((TotalBytes < (args->htrsiz*BLK_SIZE)) && (NumBytes > 0));

	CLOSE(fd);

	return 0;
}


