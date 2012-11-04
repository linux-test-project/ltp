/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */
/*
 * Define io syscalls supported by doio
 */

#define	READ	1
#define	WRITE	2
#define	READA	3
#define	WRITEA	4
#define	SSREAD	5
#define	SSWRITE	6
#define	LISTIO  7
#define	LREAD	10		/* listio - single stride, single entry */
#define	LREADA	11
#define	LWRITE	12
#define	LWRITEA	13
#define	LSREAD	14		/* listio - multi-stride, single entry */
#define	LSREADA	15
#define	LSWRITE	16
#define	LSWRITEA 17
#define	LEREAD	18		/* listio - single stride, multiple entry */
#define	LEREADA	19
#define	LEWRITE	20
#define	LEWRITEA 21

/* Irix System Calls */
#define	PREAD	100
#define	PWRITE	101
#define	READV	102
#define	WRITEV	103
#define	AREAD	104
#define	AWRITE	105
#define	LLREAD	110
#define	LLAREAD	111
#define	LLWRITE	112
#define	LLAWRITE 113
#define	MMAPR	120
#define	MMAPW	121
#define RESVSP	122		/* fcntl(F_RESVSP) */
#define UNRESVSP 123		/* fcntl(F_UNRESVSP) */
#define	DFFSYNC	124		/* fcntl(F_FSYNC) */
#define	FSYNC2	125		/* fsync(2) */
#define	FDATASYNC 126		/* fdatasync(2) */
#define	BIOSIZE	127		/* fcntl(F_SETBIOSIZE) */

#ifdef CRAY
/* used: <<doio>> 1.? <<DOIO>> 1.5 <-DOIO-> 1.7*/
#define DOIO_MAGIC  '<[DOIO]>'
#else
#define DOIO_MAGIC  07116601
#endif

/*
 * Define various user flags (r_uflag field) that io requests can have
 * specified.
 */

#define F_WORD_ALIGNED		0001	/* force request to be word aligned */

/*
 * define various doio exit status's
 */

#define E_NORMAL    000	    	/* normal completion	    	    	*/
#define E_USAGE	    001	    	/* cmdline usage error	    	    	*/
#define E_SETUP	    002	    	/* any of a million setup conditions	*/
#define E_COMPARE   004	    	/* data compare error from doio child	*/
#define E_INTERNAL  010	    	/* various internal errors  	    	*/
#define E_LOCKD	    020	    	/* lockd startup/timeout errors	    	*/
#define E_SIGNAL    040	    	/* killed by signal 	    	    	*/

/*
 * Define async io completion strategies supported by doio.
 */

#define	A_POLL		1		/* poll iosw for completion	*/
#define A_SIGNAL	2		/* get signal for completion	*/
#define A_RECALL	3		/* use recall(2) to wait	*/
#define A_RECALLA	4		/* use recalla(2) to wait	*/
#define A_RECALLS	5		/* use recalls(2) to wait	*/
#define	A_SUSPEND	6		/* use aio_suspend(2) to wait	*/
#define A_CALLBACK	7		/* use a callback signal op.	*/

/*
 * Define individual structures for each syscall type.  These will all be
 * unionized into a single io_req structure which io generators fill in and
 * pass to doio.
 *
 * Note:	It is VERY important that the r_file, r_oflags, r_offset, and
 *		r_nbytes fields occupy the same record positions in the
 *		read_req, reada_req, write_req, and writea_req structures and
 *		that they have the same type.  It is also that r_pattern
 *		has the same type/offset in the write_req and writea_req
 *		structures.
 *
 *		Since doio.c accesses all information through the r_data
 *		union in io_req, if the above assumptions hold, the above
 *		fields can be accessed without regard to structure type.
 *		This is an allowed assumption in C.
 */

#define MAX_FNAME_LENGTH    128

struct read_req {
    char    r_file[MAX_FNAME_LENGTH];
    int	    r_oflags;			/* open flags */
    int	    r_offset;
    int	    r_nbytes;
    int	    r_uflags;			/* user flags: mem alignment */
    int	    r_aio_strat;		/* asynch read completion strategy */
    int	    r_nstrides;			/* listio: multiple strides */
    int	    r_nent;			/* listio: multiple list entries */
};

struct write_req {
    char    r_file[MAX_FNAME_LENGTH];
    int	    r_oflags;
    int	    r_offset;
    int	    r_nbytes;
    char    r_pattern;
    int	    r_uflags;			/* user flags: mem alignment */
    int	    r_aio_strat;		/* asynch write completion strategy */
    int	    r_nstrides;			/* listio: multiple strides */
    int	    r_nent;			/* listio: multiple list entries */
};

struct ssread_req {
    int	    r_nbytes;
};

struct sswrite_req {
    int	    r_nbytes;
    char    r_pattern;
};

struct listio_req {
	char	r_file[MAX_FNAME_LENGTH];
	int	r_cmd;			/* LC_START or LC_WAIT */
	int	r_offset;		/* file offset */
	int	r_opcode;		/* LO_READ, or LO_WRITE */
	int	r_nbytes;		/* bytes per stride */
	int	r_nstrides;		/* how many strides to make */
	int	r_nent;			/* how many listreq entries to make */
	int	r_filestride;		/* always 0 for now */
	int	r_memstride;		/* always 0 for now */
	char	r_pattern;		/* for LO_WRITE operations */
	int	r_oflags;		/* open(2) flags */
	int	r_aio_strat;		/* async I/O completion strategy */
	int	r_uflags;		/* user flags: memory alignment */
};

#define rw_req	listio_req	/* listio is superset of everything */

/*
 * Main structure for sending a request to doio.  Any tools which form IO
 * for doio must present it using one of these structures.
 */

struct io_req {
    int	    r_type; 	    	/* must be one of the #defines above	    */
    int	    r_magic;	    	/* must be set to DOIO_MAGIC by requestor   */
    union {
	struct read_req		read;
	struct write_req	write;
	struct ssread_req	ssread;
	struct sswrite_req	sswrite;
	struct listio_req	listio;
	struct rw_req		io;
    } r_data;
};
