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
* $Id: sfunc.c,v 1.1 2002/02/21 16:49:04 robbiew Exp $
* $Log: sfunc.c,v $
* Revision 1.1  2002/02/21 16:49:04  robbiew
* Relocated disktest to /kernel/io/.
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
#ifdef WIN32
#include <process.h>
#include <windows.h>
#include <winbase.h>
#else
#include <unistd.h>
#include <ctype.h>
#endif

#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "main.h"
#include "sfunc.h"
#include "defs.h"
#include "globals.h"


/*
 * Generates a random 64bit number.
 */
OFF_T Rand64(void)
{
	OFF_T myRandomNumber = 0;

	myRandomNumber = (OFF_T) rand() << 48;
	myRandomNumber |= (OFF_T) rand() << 32;
	myRandomNumber |= (OFF_T) rand() << 16;
	myRandomNumber |= (OFF_T) rand();

	return(myRandomNumber);
}

#ifdef WIN32
/*
 * wrapper for file seeking in WIN32 API to hind the ugle 32 bit
 * interface of SetFile Pointer
 */
OFF_T FileSeek64(HANDLE hf, OFF_T distance, DWORD MoveMethod)
{
   LARGE_INTEGER li;

   li.QuadPart = distance;

   li.LowPart = SetFilePointer(hf, li.LowPart, &li.HighPart, MoveMethod);

   if (li.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR)
   {
      li.QuadPart = -1;
   }

   return li.QuadPart;
}
#endif

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
int pMsg(lvl_t level, char *Msg,...)
{
#define FORMAT "| %s | %s | %d | %s | %s | %s"
#define TIME_FORMAT "%04d/%02d/%02d-%02d:%02d:%02d"
#define TIME_FMT_LEN 19
	va_list l;
	int rv = 0;
	int len = strlen(Msg);
	char *cpTheMsg;
	char levelStr[10];
	struct tm *pstruct_time;
	char time_str[TIME_FMT_LEN];
	extern char *appname;
	extern char *devname;
	extern unsigned long glb_flags;
	

#ifdef WIN32
	pid_t my_pid = _getpid();
#else
	pid_t my_pid = getpid();
#endif
	time_t my_time = time(NULL);
	pstruct_time = localtime(&my_time);

	if((glb_flags & GLB_FLG_QUIET) && (level == INFO))
		return(0);

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

	len += TIME_FMT_LEN;
	len += strlen(levelStr);
	len += strlen(FORMAT);
	len += strlen(appname);
	len += strlen(VER_STR);
	len += sizeof(pid_t)*8 + 1;
	va_start(l, Msg);

	if((cpTheMsg = (char *)malloc(len)) == NULL) {
		printf("Can't print formatted message, printing message raw.\n");
		rv = vprintf(Msg,l);
		va_end(l);
		return rv;
	}

	sprintf(time_str, TIME_FORMAT, pstruct_time->tm_year+1900,
		pstruct_time->tm_mon+1,
		pstruct_time->tm_mday,
		pstruct_time->tm_hour,
		pstruct_time->tm_min,
		pstruct_time->tm_sec
	);

	memset(cpTheMsg, 0, len);
	sprintf(cpTheMsg, FORMAT, time_str, levelStr, my_pid, VER_STR, devname, Msg);

	rv = vprintf(cpTheMsg,l);
	free(cpTheMsg);

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
	size_t i, j;

	ucharpattern = (unsigned char *) &pass_count;
	for(i=0;i<sizeof(OFF_T);i++) {
		off_tpat2 |= (((OFF_T)(ucharpattern[i])) << sizeof(OFF_T)*((sizeof(OFF_T)-1)-i));
	}

	ucharpattern = (unsigned char *) &local_lba;

	switch(mark_type) {
		case MARK_FIRST :
			for(i=0;i<sizeof(OFF_T);i++) {
				off_tpat |= (((OFF_T)(ucharpattern[i])) << sizeof(OFF_T)*((sizeof(OFF_T)-1)-i));
			}
			
			/* fill first 8 bytes with lba number */
			*(off_tbuf) = off_tpat;
			/* fill second 8 bytes with pass_count */
			*(off_tbuf+1) = off_tpat2;
			break;
		case MARK_LAST :
			local_lba += (buf_len-BLK_SIZE);
			for(i=0;i<sizeof(OFF_T);i++) {
				off_tpat |= (((OFF_T)(ucharpattern[i])) << sizeof(OFF_T)*((sizeof(OFF_T)-1)-i));
			}

			/* fill second to last 8 bytes with lba number */
			*(off_tbuf+((buf_len-BLK_SIZE)/sizeof(OFF_T))) = off_tpat;
			/* fill last 8 bytes with pass_count */
			*(off_tbuf+((buf_len-BLK_SIZE)/sizeof(OFF_T))+1) = off_tpat2;
			break;
		case MARK_ALL :
			for(i=0;i<buf_len;i=i+BLK_SIZE) {
				for(j=0;j<sizeof(OFF_T);j++) {
					off_tpat |= (((OFF_T)(ucharpattern[j])) << sizeof(OFF_T)*((sizeof(OFF_T)-1)-j));
				}

				/* fill first 8 bytes with lba number */
				*(off_tbuf+(i/sizeof(OFF_T))) = off_tpat;
				/* fill second 8 bytes with pass_count */
				*(off_tbuf+(i/sizeof(OFF_T))+1) = off_tpat2;
				local_lba++;
				off_tpat = 0;
			}
			break;
		default :
			pMsg(WARN,"Unknown mark type\n");
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
	/* unsigned int rand_seed; */
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
					off_tpat |= (((OFF_T)(ucharpattern[i])) << 8*(7-((j*pattern_len)+i)));

			/* fill buffer with fixed pattern */
			for(i=0;i<len/8;i++)
				*(off_tbuf+i) = off_tpat;
			break;
		case CLD_FLG_LPTYPE :
			off_tpat2 = *poff_tpattern;
			for(j=0;j<len;j++) {
				off_tpat = 0;
				/* arrange data to go on the wire correctly */
				ucharpattern = (unsigned char *) &off_tpat2;
				for(i=0;i<pattern_len;i++) {
					off_tpat |= (((OFF_T)(ucharpattern[i])) << 8*(7-i));
				}

				/* fill buffer with lba number */
				for(i=0;i<BLK_SIZE/8;i++) {
					*(off_tbuf+i+(j*(BLK_SIZE/8))) = off_tpat;
				}
				off_tpat2++;
			}
			break;
		case CLD_FLG_RPTYPE :
			/*
			* Will fill buffer with a random pattern.  The first 64 bits
			* are the seed to the rest of data.  This is done so
			* that the data can be regenerated on the fly when it is read.
			* The seed is passed in through pattern
			*/
			for(i=1;i<len;i++)
				*(off_tbuf+i) = Rand64();
			*off_tbuf = (*poff_tpattern & 0xFFFFFFFF);  /* seed is only 32 bits */
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
		pMsg(INFO, "Forcing to 100%% read\n");
		args->rperc = 100;
	} else if((args->flags & CLD_FLG_W) && !(args->flags & CLD_FLG_R)) {
		pMsg(INFO, "Forcing to 100%% write\n");
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
		pMsg(INFO, "Balancing percentage between reads and writes\n");
		if((args->flags & CLD_FLG_R) && (args->flags & CLD_FLG_W)) {
			i = 100 - (args->rperc + args->wperc);
			j = i / 2;
			args->wperc += j;
			args->rperc += (i - j);
		}
	}
}

char *strupr(char *String)
{
	unsigned int i;

	for(i=0;i<strlen(String);i++) {
		*(String+i) = toupper(*(String+i));
	}
	return(String);
}

char *strlwr(char *String)
{
	unsigned int i;

	for(i=0;i<strlen(String);i++) {
		*(String+i) = tolower(*(String+i));
	}
	return(String);
}


OFF_T get_vsiz(char *device)
{
	OFF_T size = 0;

#ifdef WIN32
	HANDLE hFileHandle;
	DISK_GEOMETRY DiskGeom;
	BOOL bRV;

	DWORD dwLength;
	
	hFileHandle = CreateFile(device, 
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if(hFileHandle == INVALID_HANDLE_VALUE) {
		pMsg(ERR, "could not open %s.\n",device);
		pMsg(ERR, "Error = %u\n",GetLastError());
		return(GetLastError());
	}

	bRV = DeviceIoControl(hFileHandle,
		IOCTL_DISK_GET_DRIVE_GEOMETRY,
		NULL,
		0,
		&DiskGeom,
		sizeof(DISK_GEOMETRY),
		&dwLength,
		NULL);

	CloseHandle(hFileHandle);

	if(bRV) {
		size =  (OFF_T) DiskGeom.Cylinders.QuadPart;
		size *= (OFF_T) DiskGeom.TracksPerCylinder;
		size *= (OFF_T) DiskGeom.SectorsPerTrack;
	}
#else
	int fd;

	if((fd = open(device, 0)) < 0) {
		pMsg(ERR,"Cannot open %s\n", device);
		exit(1);
	}
	if(ioctl(fd, BLKGETSIZE, &size) == -1) size = -1;
	close(fd);
#endif
	return(size);
}

#ifndef WIN32
void Sleep(unsigned int msecs)
{
	usleep(msecs*1000);
}
#endif

