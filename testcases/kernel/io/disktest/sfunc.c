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
*
* $Id: sfunc.c,v 1.5 2006/02/24 02:13:40 vapier Exp $
* $Log: sfunc.c,v $
* Revision 1.5  2006/02/24 02:13:40  vapier
* kill off warning about j being unused
*
* Revision 1.4  2005/05/04 17:54:00  mridge
* Update to version 1.2.8
*
* Revision 1.25  2005/05/03 16:24:38  yardleyb
* Added needed code changes to support windows
*
* Revision 1.24  2005/04/28 21:25:18  yardleyb
* Fixed up some issues with AIX compilation due to the change made
* in endian support in Linux.
*
* Revision 1.23  2005/01/08 21:18:34  yardleyb
* Update performance output and usage.  Fixed pass count check
*
* Revision 1.22  2004/12/17 06:34:56  yardleyb
* removed -mf -ml.  These mark options cause to may issues when using
* random block size transfers.  Fixed -ma option for endian-ness.  Fixed
* false data misscompare during multiple cycles.
*
* Revision 1.21  2004/11/20 05:05:58  yardleyb
* added more command line checking
*
* Revision 1.20  2004/11/20 04:43:42  yardleyb
* Minor code fixes.  Checking for alloc errors.
*
* Revision 1.19  2004/11/02 21:12:21  yardleyb
* Added PPC ifdef for ioctl BLKGETSIZE
*
* Revision 1.17  2003/09/12 21:23:01  yardleyb
* The following isses have been fixed:
* - Updated to Version 1.12
* - Disktest will falsely detect a data miscompare
* when using random block sizes and random data
* - If the linear option is used while doing random
* block sizes and read/write/error checks, disktest
* will hang
* - Disktest will use the wrong transfer size on
* the last IO when using random block transfer size
* and the number of seeks are specified.
* - Total Reads and Writes not reported correctly
* - While running linear write/read tests while
* doing the heartbeat performance and you get an
* error on the 'write' side of the test, disktest
* does not exit
*
* Revision 1.16  2003/01/13 21:58:23  yardleyb
* Added includes for AIX change
*
* Revision 1.15  2003/01/13 21:33:31  yardleyb
* Added code to detect AIX volume size.
* Updated mask for random LBA to use start_lba offset
* Updated version to 1.1.10
*
* Revision 1.14  2002/05/31 18:47:59  yardleyb
* Updates to -pl -pL options.
* Fixed test status to fail on
* failure to open filespec.
* Version set to 1.1.9
*
* Revision 1.13  2002/04/24 01:45:31  yardleyb
* Minor Fixes:
* Read/write time could exceeds overall time
* Heartbeat options sometimes only displayed once
* Cleanup time for large number of threads was very long (windows)
* If heartbeat specified, now checks for performance option also
* No IO was performed when -S0:0 and -pr specified
*
* Revision 1.12  2002/03/30 01:32:14  yardleyb
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
* Revision 1.11  2002/03/07 03:34:42  yardleyb
* Rearranged string length
* calculation, removed appname
*
* Revision 1.10  2002/02/28 02:04:32  yardleyb
* Moved FileSeek64 to IO
* source files.
*
* Revision 1.9  2002/02/19 02:46:37  yardleyb
* Added changes to compile for AIX.
* Update getvsiz so it returns a -1
* if the ioctl fails and we handle
* that fact correctly.  Added check
* to force vsiz to always be greater
* then stop_lba.
*
* Revision 1.8  2002/02/04 20:35:38  yardleyb
* Changed max. number of threads to 64k.
* Check for max threads in parsing.
* Fixed windows getopt to return correctly
* when a bad option is given.
* Update time output to be in the format:
*   YEAR/MONTH/DAY-HOUR:MIN:SEC
* instead of epoch time.
*
* Revision 1.7  2001/12/04 18:51:06  yardleyb
* Checkin of new source files and removal
* of outdated source
*
* Revision 1.5  2001/10/10 00:17:14  yardleyb
* Added Copyright and GPL license text.
* Miner bug fixes throughout text.
*
* Revision 1.4  2001/10/01 23:29:12  yardleyb
* Added column for device name in pMsg.
* Shorted buffer size for the level string.
*
* Revision 1.3  2001/09/22 03:44:25  yardleyb
* Added level code pMsg.
*
* Revision 1.2  2001/09/06 18:23:30  yardleyb
* Added duty cycle -D.  Updated usage. Added
* make option to create .tar.gz of all files
*
* Revision 1.1  2001/09/05 22:44:42  yardleyb
* Split out some of the special functions.
* added O_DIRECT -Id.  Updated usage.  Lots
* of clean up to functions.  Added header info
* to pMsg.
*
*
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#ifdef WINDOWS
#include <process.h>
#include <windows.h>
#include <winbase.h>
#include <winioctl.h>
#else
#ifdef AIX
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#endif
#include <unistd.h>
#include <ctype.h>
#endif

#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#ifdef LINUX
#include <endian.h>
#endif

#include "main.h"
#include "sfunc.h"
#include "defs.h"
#include "globals.h"
#include "io.h"

/*
 * Generates a random 32bit number.
 */
long Rand32(void)
{
	/*
	 * based on the fact that rand returns
	 * 0 - 0x7FFF
	 */
	long myRandomNumber = 0;

	myRandomNumber  = ((long) (rand() & 0x7FFF)) << 16;
	myRandomNumber |= ((long) (rand() & 0x7FFF)) << 1;
	myRandomNumber |= ((long) (rand() & 0x1));

	return(myRandomNumber);
}

/*
 * Generates a random 64bit number.
 */
OFF_T Rand64(void)
{
	OFF_T myRandomNumber = 0;

	myRandomNumber  = ((OFF_T) (rand() & 0x7FFF)) << 48;
	myRandomNumber |= ((OFF_T) (rand() & 0x7FFF)) << 33;
	myRandomNumber |= ((OFF_T) (rand() & 0x7FFF)) << 18;
	myRandomNumber |= ((OFF_T) (rand() & 0x7FFF)) << 3;
	myRandomNumber |= ((OFF_T) (rand() & 0x7));

	return(myRandomNumber);
}

/*
* could not find a function that represented a conversion
* between a long long and a string.
*/
OFF_T my_strtofft(const char *pStr)
{
	OFF_T value = 0;
	int bOct = 0, bHex = 0;

	int neg = 0;

	for(;;pStr++) {
		switch(*pStr) {
			case '0':
				bOct = 1;
				continue;
			case 'x':
				if(bOct) bHex = 1;
				continue;
			case ' ':
			case '\t':
				continue;
			case '-':
				neg = 1;
				/*FALLTHROUGH*/
			case '+':
				pStr++;
		}
		break;
	}
	if((!bOct) && (!bHex)) {
		while (*pStr >= '0' && *pStr <= '9') {
			value = (value * 10) + (*pStr++ - '0');
		}
	} else if(bHex) {
		while ((*pStr >= '0' && *pStr <= '9') ||
			   (*pStr >= 'A' && *pStr <= 'F') ||
			   (*pStr >= 'a' && *pStr <= 'f')) {
			if(*pStr >= '0' && *pStr <= '9')
				value = (value << 4) + (*pStr++ - '0');
			else if(*pStr >= 'A' && *pStr <= 'F')
				value = (value << 4) + 10 + (*pStr++ - 'A');
			else if(*pStr >= 'a' && *pStr <= 'f')
				value = (value << 4) + 10 + (*pStr++ - 'a');
		}
	} else if(bOct) {
		while (*pStr >= '0' && *pStr <= '7') {
			value = (value * 8) + (*pStr++ - '0');
		}
	}
	return (neg ? -value : value);
}

/*
* prints messages to stdout. with added formating
*/
int pMsg(lvl_t level, child_args_t *args, char *Msg,...)
{
#define FORMAT "| %s | %s | %d | %s | %s | %s"
#define TIME_FORMAT "%04d/%02d/%02d-%02d:%02d:%02d"
#define TIME_FMT_LEN 20
	va_list l;
	int rv = 0;
	size_t len = 0;
	char *cpTheMsg;
	char levelStr[10];
	struct tm *pstruct_time;
	char time_str[TIME_FMT_LEN];
	extern unsigned long glb_flags;
	
	time_t my_time = time(NULL);
	pstruct_time = localtime(&my_time);

	if((glb_flags & GLB_FLG_QUIET) && (level == INFO))
		return(0);

	va_start(l, Msg);

	if(glb_flags & GLB_FLG_SUPRESS) {
		rv = vprintf(Msg,l);
		va_end(l);
		return rv;
	}

	switch(level) {
		case START:
			strcpy(levelStr, "START");
			break;
		case END:
			strcpy(levelStr, "END  ");
			break;
		case STAT:
			strcpy(levelStr, "STAT ");
			break;
		case INFO:
			strcpy(levelStr, "INFO ");
			break;
		case DEBUG:
			strcpy(levelStr, "DEBUG");
			break;
		case WARN:
			strcpy(levelStr, "WARN ");
			break;
		case ERR:
			strcpy(levelStr, "ERROR");
			break;
	}

	sprintf(time_str, TIME_FORMAT, pstruct_time->tm_year+1900,
		pstruct_time->tm_mon+1,
		pstruct_time->tm_mday,
		pstruct_time->tm_hour,
		pstruct_time->tm_min,
		pstruct_time->tm_sec
	);

	len += strlen(FORMAT);
	len += strlen(time_str);
	len += strlen(levelStr);
	len += sizeof(pid_t)*8 + 1;
	len += strlen(VER_STR);
	len += strlen(args->device);
	len += strlen(Msg);

	if((cpTheMsg = (char *)ALLOC(len)) == NULL) {
		printf("Can't print formatted message, printing message raw.\n");
		rv = vprintf(Msg,l);
		va_end(l);
		return rv;
	}


	memset(cpTheMsg, 0, len);
	sprintf(cpTheMsg, FORMAT, time_str, levelStr, args->pid, VER_STR, args->device, Msg);

	rv = vprintf(cpTheMsg,l);
	FREE(cpTheMsg);

	va_end(l);
	return rv;
}

void mark_buffer(void *buf, const size_t buf_len, void *lba, const OFF_T pass_count, const unsigned short mark_type)
{
	unsigned char *ucharpattern;
	OFF_T *plocal_lba = lba;
	OFF_T local_lba = *plocal_lba;
	OFF_T *off_tbuf = buf;
	OFF_T off_tpat = 0, off_tpat2 = 0;
	size_t i = 0;

#ifdef WINDOWS
	ucharpattern = (unsigned char *) &pass_count;
	for(i=0;i<sizeof(OFF_T);i++) {
		off_tpat2 |= (((OFF_T)(ucharpattern[i])) << sizeof(OFF_T)*((sizeof(OFF_T)-1)-i));
	}
#elif defined AIX
	off_tpat2 = pass_count;
#elif defined LINUX
# if __BYTE_ORDER == __LITTLE_ENDIAN
	ucharpattern = (unsigned char *) &pass_count;
	for(i=0;i<sizeof(OFF_T);i++) {
		off_tpat2 |= (((OFF_T)(ucharpattern[i])) << sizeof(OFF_T)*((sizeof(OFF_T)-1)-i));
	}
# else
	off_tpat2 = pass_count;
# endif
#endif

	ucharpattern = (unsigned char *) &local_lba;
	switch(mark_type) {
		case MARK_FIRST :
			printf("Depricated mark function please use -ma\n");
			exit(1);
		case MARK_LAST :
			printf("Depricated mark function please use -ma\n");
			exit(1);
		case MARK_ALL :
			for(i=0;i<buf_len;i=i+BLK_SIZE) {
#ifdef WINDOWS
				size_t j;
				off_tpat = 0;
				for(j=0;j<sizeof(OFF_T);j++) {
					off_tpat |= (((OFF_T)(ucharpattern[j])) << sizeof(OFF_T)*((sizeof(OFF_T)-1)-j));
				}
#elif defined AIX
				off_tpat = local_lba;
#elif defined LINUX
# if __BYTE_ORDER == __LITTLE_ENDIAN
				size_t j;
				off_tpat = 0;
				for(j=0;j<sizeof(OFF_T);j++) {
					off_tpat |= (((OFF_T)(ucharpattern[j])) << sizeof(OFF_T)*((sizeof(OFF_T)-1)-j));
				}
# else
				off_tpat = local_lba;
# endif
#endif
				/* fill first 8 bytes with lba number */
				*(off_tbuf+(i/sizeof(OFF_T))) = off_tpat;
				/* fill second 8 bytes with pass_count */
				*(off_tbuf+(i/sizeof(OFF_T))+1) = off_tpat2;
				local_lba++;
			}
			break;
		default :
			printf("Unknown mark type\n");
			exit(1);
	}
}

/*
* function fill_buffer
* This function fills the passed buffer with data based on the pattern and patten type.
* for pattern types of counting the pattern does not matter.  For lba pattern type, the
* pattern will be the address of the lba.
*/

void fill_buffer(void *buf, size_t len, void *pattern, size_t pattern_len, const unsigned int pattern_type)
{
	size_t i, j;
	unsigned char *ucharbuf = buf;
	OFF_T *off_tbuf = buf;
	unsigned char *ucharpattern = pattern;
	OFF_T *poff_tpattern = pattern;
	OFF_T off_tpat, off_tpat2;

	switch (pattern_type) { /* the pattern type should only be one of the following */
		case CLD_FLG_CPTYPE :
			/* Will fill buffer with counting pattern 0x00 thru 0xff */
			for(i=0;i<len;i++)
				ucharbuf[i] = (unsigned char) (i & 0xff);
			break;
		case CLD_FLG_FPTYPE :
			/* arrange data to go on the wire correctly */
			off_tpat = 0;
			for(j=0;j<(sizeof(OFF_T)/pattern_len);j++)
				for(i=0;i<pattern_len;++i)
#ifdef WINDOWS
					off_tpat |= (((OFF_T)(ucharpattern[i])) << 8*(7-((j*pattern_len)+i)));
#endif
#ifdef AIX
					off_tpat |= (((OFF_T)(ucharpattern[(8-pattern_len)+i])) << 8*(7-((j*pattern_len)+i)));
#endif
#ifdef LINUX
#if __BYTE_ORDER == __LITTLE_ENDIAN
					off_tpat |= (((OFF_T)(ucharpattern[i])) << 8*(7-((j*pattern_len)+i)));
#else
					off_tpat |= (((OFF_T)(ucharpattern[(8-pattern_len)+i])) << 8*(7-((j*pattern_len)+i)));
#endif
#endif

			/* fill buffer with fixed pattern */
			for(i=0;i<len/8;i++)
				*(off_tbuf+i) = off_tpat;
			break;
		case CLD_FLG_LPTYPE :
			off_tpat2 = *poff_tpattern;
			for(j=0;j<len;j++) {
				/* arrange data to go on the wire correctly */
				ucharpattern = (unsigned char *) &off_tpat2;
				off_tpat = 0;
				for(i=0;i<pattern_len;i++)
#ifdef WINDOWS
					off_tpat |= (((OFF_T)(ucharpattern[i])) << 8*(7-i));
#endif
#ifdef AIX
					off_tpat |= (((OFF_T)(ucharpattern[(8-pattern_len)+i])) << 8*(7-i));
#endif
#ifdef LINUX
#if __BYTE_ORDER == __LITTLE_ENDIAN
					off_tpat |= (((OFF_T)(ucharpattern[i])) << 8*(7-i));
#else
					off_tpat |= (((OFF_T)(ucharpattern[(8-pattern_len)+i])) << 8*(7-i));
#endif
#endif

				/* fill buffer with lba number */
				for(i=0;i<BLK_SIZE/8;i++) {
					*(off_tbuf+i+(j*(BLK_SIZE/8))) = off_tpat;
				}
				off_tpat2++;
			}
			break;
		case CLD_FLG_RPTYPE :
			/* Will fill buffer with a random pattern.
			 * Unfortunatly, every LBA, 512 bytes of data will be
			 * the same random data set, this is due to the LBA
			 * boundary requirement of disktest.  This should be fixed
			 * at some point...
			 */
			for(i=0;i<BLK_SIZE/sizeof(OFF_T);i++)
				*(off_tbuf+i) = Rand64();

			for(i=BLK_SIZE;i<len;i+=BLK_SIZE)
				memcpy((ucharbuf+i), ucharbuf, BLK_SIZE);
			break;
		default :
			printf("Unknown fill pattern\n");
			exit(1);
	}
}

void normalize_percs(child_args_t *args)
{
	int i, j;

	if((args->flags & CLD_FLG_R) && !(args->flags & CLD_FLG_W)) {
		if((args->flags & CLD_FLG_DUTY) && (args->rperc < 100)) {
			pMsg(WARN, args, "Read specified w/o write, ignoring -D, forcing read only...\n");
		}
		args->rperc = 100;
		args->wperc = 0;
	} else if((args->flags & CLD_FLG_W) && !(args->flags & CLD_FLG_R)) {
		if((args->flags & CLD_FLG_DUTY) && (args->wperc < 100)) {
			pMsg(WARN, args, "Write specified w/o read, ignoring -D, forcing write only...\n");
		}
		args->rperc = 0;
		args->wperc = 100;
	} else { /* must be reading and writing */
		if (args->rperc == 0 && args->wperc == 0) {
			args->rperc = 50;
			args->wperc = 50;
		} else if(args->rperc == 0) {
			args->rperc = 100 - args->wperc;
		} else if (args->wperc == 0) {
			args->wperc = 100 - args->rperc;
		}
	}

	if (args->rperc + args->wperc != 100) {
		pMsg(INFO, args, "Balancing percentage between reads and writes\n");
		if((args->flags & CLD_FLG_R) && (args->flags & CLD_FLG_W)) {
			i = 100 - (args->rperc + args->wperc);
			j = i / 2;
			args->wperc += j;
			args->rperc += (i - j);
		}
	}
}

#ifndef WINDOWS
char *strupr(char *String) {
	unsigned int i;

	for(i=0;i<strlen(String);i++) {
		*(String+i) = toupper(*(String+i));
	}
	return(String);
}

char *strlwr(char *String) {
	unsigned int i;

	for(i=0;i<strlen(String);i++) {
		*(String+i) = tolower(*(String+i));
	}
	return(String);
}
#endif

OFF_T get_file_size(char *device) {
	OFF_T size = 0;
	fd_t fd;

#ifdef WINDOWS
	SetLastError(0);

	fd = CreateFile(device, 
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
#else
	fd = open(device, 0);
#endif

	if(INVALID_FD(fd)) {
		return size;
	}

	size = SeekEnd(fd);
	size /= BLK_SIZE;

	CLOSE(fd);
	return size;
}


OFF_T get_vsiz(const char *device)
{
#ifdef PPC
	unsigned long size = 0;
#else
	OFF_T size = 0;
#endif

#ifdef WINDOWS
	HANDLE hFileHandle;
	BOOL bRV;
	DWORD dwLength;
	GET_LENGTH_INFORMATION myLengthInfo;
	DISK_GEOMETRY DiskGeom;

	hFileHandle = CreateFile(device, 
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if(hFileHandle == INVALID_HANDLE_VALUE) {
		return(GetLastError());
	}

	SetLastError(0);
	bRV = DeviceIoControl(hFileHandle,
		IOCTL_DISK_GET_LENGTH_INFO,
		NULL,
		0,
		&myLengthInfo,
		sizeof(GET_LENGTH_INFORMATION),
		&dwLength,
		NULL);


	if(bRV) {
		size = myLengthInfo.Length.QuadPart;
	} else {
		bRV = DeviceIoControl(hFileHandle,
			IOCTL_DISK_GET_DRIVE_GEOMETRY,
			NULL,
			0,
			&DiskGeom,
			sizeof(DISK_GEOMETRY),
			&dwLength,
			NULL);

		if(bRV) {
			size =  (OFF_T) DiskGeom.Cylinders.QuadPart;
			size *= (OFF_T) DiskGeom.TracksPerCylinder;
			size *= (OFF_T) DiskGeom.SectorsPerTrack;
		} else {
			size = 0;
		}
	}
	CloseHandle(hFileHandle);
#else
	int fd = 0;
#if AIX
	struct devinfo *my_devinfo = NULL;
#endif

	if((fd = open(device, 0)) < 0) {
		return 0;
	}

#if AIX
	my_devinfo = (struct devinfo*) ALLOC(sizeof(struct devinfo));
	if(my_devinfo != NULL) {
		memset(my_devinfo, 0, sizeof(struct devinfo));
		if(ioctl(fd, IOCINFO, my_devinfo) == -1) size = -1;
		else size = (OFF_T)(my_devinfo->un.scdk.numblks) - (OFF_T)1;
		FREE(my_devinfo);
	}
#else
	if(ioctl(fd, BLKGETSIZE, &size) == -1) size = -1;
	else size--;
#endif

	close(fd);
#endif

#ifdef PPC
	return((OFF_T)size);
#else
	return(size);
#endif
}

#ifndef WINDOWS
void Sleep(unsigned int msecs)
{
	usleep(msecs*1000);
}
#endif

fmt_time_t format_time(time_t seconds)
{
	fmt_time_t time_struct;

	time_struct.days    = seconds/86400;
	time_struct.hours   = (seconds%86400)/3600;
	time_struct.minutes = (seconds%3600)/60;
	time_struct.seconds = seconds%60;

	return time_struct;
}


