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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: rand_lines.c,v 1.1 2001/01/22 17:50:31 nstraz Exp $ */
/**************************************************************
 *
 *    OS Testing - Silicon Graphics, Inc.
 *
 *    TOOL IDENTIFIER   : rand_lines
 *
 *    DESCRIPTION       : prints lines from a file in random order
 *
 *    SYNOPSIS:
 *      rand_line [-hg][-S seed][-l numlines] [files...]
 *
 *    AUTHOR            : Richard Logan
 *
 *    CO-PILOT(s)       : 
 *
 *    DATE STARTED      : 05/94
 *
 *    INPUT SPECIFICATIONS
 *     This tool will print lines of a file in random order.
 *     There is max line length is 2048.
 *     The options supported are:
 *       -h     This option prints an help message then exits.
 *
 *       -g     This option specifies to count the number of lines
 *		in the file before randomizing.  This option overrides
 *		-l option.  Using this option, will give you the best
 *		randomization, but it requires processing
 *		the file an additional time.
 *		       
 *       -l numlines : This option specifies to randomize file in
 *		numlines chucks.  The default size if 2048.
 *
 *       -S seed     : sets randomization seed to seed. 
 *		The default is time(0).  If seed is zero, time(0) is used.
 *
 *	 file   A readable, seekable filename.  The cmd allows the user
 *	 	to specify multiple files, but each file is dealt with
 *		separately.
 *
 *    DESIGN DESCRIPTION
 *	This tool uses a simple algorithm where the file is read.
 *	The offset to the each line is randomly placed into an
 *	array.  The array is then processed sequentially.  The infile's
 *	line who's offset in the array element is thus reread then printed.
 *	This output will thus be infile's lines in random order.
 *
 *    SPECIAL REQUIREMENTS
 *	None.
 *
 *    UPDATE HISTORY
 *      This should contain the description, author, and date of any
 *      "interesting" modifications (i.e. info should helpful in
 *      maintaining/enhancing this tool).
 *      username     description
 *      ----------------------------------------------------------------
 *	rrl 	    Creatation of program
 *
 *    BUGS/LIMITATIONS
 *	This program can not deal with non-seekable file like
 *	stdin or a pipe.  If more than one file is specified,
 *	each file is randomized one at a time.  The max line
 *	length is 2048 characters.
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

void usage();
void help();

/*
 * Structure used to hold file line offset.
 */
struct offset_t {
	long used;
	long offset;
};

#define DEF_SIZE	2048
#define MAX_LN_SZ	2048		/* max line size */

extern int errno;

extern int random_range();
extern int random_range_seed();

#ifndef SEEK_SET
#define SEEK_SET	0
#endif

char *Progname;

/***********************************************************************
 *  MAIN
 ***********************************************************************/
main(argc, argv)
int argc;
char **argv;
{
    FILE *infile;
    int c;
    long seed = -1;		/* use time as seed */
    int lsize = DEF_SIZE;	/* num lines to randomize */
    int getfilelines = 0;	/* if set, count lines first */
    extern int optind;
    extern char *optarg;

    Progname = argv[0];

    while ((c = getopt (argc, argv, "hgS:l:")) != EOF){
	switch(c) {
	case 'h':
	    help();
	    exit(0);
	    break;
	case 'S':	/* seed */
	    if ( sscanf(optarg, "%li", &seed) != 1 ) {
		fprintf(stderr, "%s: --S option argument is invalid\n", Progname);
		exit(1);
	    }
	    break;

	case 'l':	/* number of lines */
	    if ( sscanf(optarg, "%i", &lsize) != 1 ) {
		fprintf(stderr, "%s: --s option argument is invalid\n", Progname);
		exit(1);
	    }
	    break;

	case 'g':
	    getfilelines++;
	    break;

	case '?':
	    usage(stderr);
	    exit(1);
	    break;
	}
    }

    if ( optind + 1 != argc ) {
	printf("missing argument\n");
	usage(stderr);
	exit(1);
    }

    if ( seed == -1 ) {
	seed = time(0);
    }
    
    if ( strcmp(argv[argc-1],"-") == 0 ) {
	infile = stdin;
	printf("Can not support stdin processing\n");
	exit(2);
    }
    else {

	if ((infile=fopen(argv[argc-1], "r")) == NULL) {
	    printf("unable to open file %s\n", argv[argc-1]);
	    exit(1);
	}

        if ( getfilelines ) {
	    lsize=get_numlines(infile);
	}

	rnd_file(infile, lsize, seed);
    }

    exit(0);
}

/***********************************************************************
 * Print usage message to stream.
 ***********************************************************************/
void
usage(FILE *stream)
{
    fprintf(stream,
	"Usage %s [-hg][-S seed][-l numlines] [files...]\n", Progname);

}

/***********************************************************************
 * Print help message to stdout.
 ***********************************************************************/
void
help()
{
    usage(stdout);
    printf("This tool will print lines in random order (max line len %d).\n\
  -h          : print this help and exit\n\
  -g          : count the number of lines in the file before randomizing\n\
	        This option overrides -l option.\n\
  -l numlines : randoms lines in numlines chuncks (def %d)\n\
  -S seed     : sets seed to seed (def time(0))\n",
    MAX_LN_SZ, DEF_SIZE);

}

/***********************************************************************
 * counts the number of lines in already open file.
 * Note: File must be seekable (not stdin or a pipe).
 ***********************************************************************/
get_numlines(infile)
FILE *infile;
{
    char line[MAX_LN_SZ];		/* max size of a line */
    int cnt=0;

    while ( fgets(line, MAX_LN_SZ, infile) != NULL ) {
	cnt++;
    }

    /* rewind the file */
    fseek(infile, 0, SEEK_SET);

    return cnt;
}

/***********************************************************************
 *
 *  infile must be a fseekable file.  Thus, it can not be stdin.
 * It will read each line in the file, randomly saving the offset
 * of each line in a array of struct offset_t.
 * It will then print each line in the array stored order.
 *
 ***********************************************************************/
rnd_file(infile, numlines, seed)
FILE *infile;
int numlines;		/* can be more or less than num lines in file */
			/* most opt randomized when num lines in files */
			/* or just a bit bigger */
long seed;
{

    int c;
    char line[MAX_LN_SZ];		/* max size of a line */
    int cnt;
    int coffset;		/* current line offset */
    int loffset;		/* last line offset */

    struct offset_t *offsets;
    struct offset_t *offsets_;
    int memsize;

    if ( numlines <= 0 ) {	/*use default */
	numlines = DEF_SIZE;
    }

    /*
     * Malloc space for numlines copies the offset_t structure.
     * This is where the randomization takes place.
     */
    memsize = sizeof(struct offset_t)*numlines;

    if ((offsets=(struct offset_t *)malloc(memsize)) == NULL ) {
	fprintf(stderr, "Unable to malloc(%d): errno:%d\n", memsize, errno);
	return -1;
    }

    random_range_seed(seed);

    coffset=0;
    loffset=0;

    while ( ! feof(infile) ) {

        fseek(infile, coffset, SEEK_SET);
        loffset=ftell(infile);
        bzero((char *)offsets, memsize);
        cnt=0;

	/*
	 * read the file in and place offset of each line randomly
	 * into offsets array.  Only numlines line can be randomized
	 * at a time.
	 */
        while ( cnt < numlines && fgets(line, MAX_LN_SZ, infile) != NULL ) {

	    c=rnd_insert(offsets, loffset, numlines);
	    cnt++;

	    coffset=ftell(infile);
	    loffset = coffset;
        }

        if ( cnt == 0 ) {
	    continue;
        }

        /*
         * print out lines based on offset.
         */
        for (cnt=0; cnt<numlines; cnt++) {

	    if ( offsets[cnt].used ) {
	        fseek(infile, offsets[cnt].offset, SEEK_SET);
	        fgets(line, MAX_LN_SZ, infile);
	        printf("%s", line);
	    }
        }

    }	/* end of file */

    return 0;
}

/***********************************************************************
 * This function randomly inserts offset information into
 * the offsets array.  The array has a size of size.
 * It will attempt 75 random array indexes before finding the first
 * open array element.
 *
 ***********************************************************************/
rnd_insert(offsets, offset, size)
struct offset_t offsets[];
int offset;
int size;
{
    int rand_num;
    int quick = 0;
    int ind;
    double randnum;
    struct offset_t *offptr;

    /*
     * Loop looking for random unused index.
     * It will only be attempted 75 times.
     */
    while ( quick < 75 ) {

	rand_num=random_range(0, size, 1, NULL);

	if ( ! offsets[rand_num].used ) {
	    offsets[rand_num].offset=offset;
	    offsets[rand_num].used++;
	    return rand_num;
	}
	quick++;
    }

    /*
     * an randomly choosen index was not found, find
     * first open index and use it.
     */
    ind=0;
    while ( ! offsets[ind].used)
        ind++;

    offsets[ind].offset=offset;
    offsets[ind].used++;
    return ind;

}



/***********************************************************************
 *
 * CODE NOT TESTED AT ALL - it must be tested before it is used.
 *
 * This function was written to allow rand_lines to work on non-seekable
 * file (i.e stdin).
 *
 ***********************************************************************/
int
rnd_stdin(infile, space, numlines, seed)
FILE *infile;
int space;		/* amount of space to use to read file into memory, */
			/* randomized and print.  randomize in chunks */
int numlines;		/* can be more or less than num lines in file */
			/* most opt randomized when num lines in files */
			/* or just a bit bigger */
long seed;
{

    int c;
    char line[MAX_LN_SZ];		/* max size of a line */
    int cnt;				/* offset printer counter */
    int loffset;			/* last line address */
    char *buffer;			/* malloc space for file reads */
    char *rdbuff;			/* where to start read */
    long stopaddr;			/* end of read space (address)*/
    int rdsz;				/* amount read */
    int sztord;
    char *chr;				/* buffer processing pointer */
    char *ptr;				/* printing processing pointer */
    char *lptr;				/* printing processing pointer */
    int loopcntl = 1;			/* main loop control flag */
    struct offset_t *offsets;		/* pointer to offset space */
    int memsize;			/* amount of offset space to malloc */
    int newbuffer = 1;			/* need new buffer */

    if ( numlines <= 0 ) {	/*use default */
	numlines = DEF_SIZE;
    }

    /*
     * Malloc space for file contents
     */
    if ((buffer=(char *)malloc(space)) == NULL ) {
	fprintf(stderr, "Unable to malloc(%d): errno:%d\n", space, errno);
	return -1;
    }

    /*
     * Malloc space for numlines copies the offset_t structure.
     * This is where the randomization takes place.
     */
    memsize = sizeof(struct offset_t)*numlines;

    if ((offsets=(struct offset_t *)malloc(memsize)) == NULL ) {
	fprintf(stderr, "Unable to malloc(%d): errno:%d\n", memsize, errno);
	return -1;
    }

    random_range_seed(seed);
    rdbuff = buffer;		/* read into start of buffer */
    sztord = space;		/* amount of space left in buffer */

    /*
     *  Loop until read doesn't read anything
     *  If last line does not end in newline, it is not printed
     */
    while ( loopcntl ) {
        /*
         *  read in file up to space size
	 *  only works if used as filter.
	 *  The code will randomize one reads worth at a time.
	 *  If typing in lines, read will read only one line - no randomizing.
         */

        chr = buffer;
        if ((rdsz=fread((void *)rdbuff, sztord, 1, infile)) == 0 ) {
	    fprintf(stderr, "input file is empty, done randomizing\n");
	    loopcntl=0;
	    return 0;
        }

	stopaddr = ((long)buffer + rdsz);

        loffset= (long)buffer;

	while ( ! newbuffer ) {

            while ( (long)chr < stopaddr && *chr != '\n' )
	        chr++;

	    chr++;

	    if ( (long)chr >= stopaddr ) {

	        fprintf(stderr, "end of read in buffer\n");

		/*
		 * print out lines based on offset.
		 */
		for (cnt=0; cnt<numlines; cnt++) {

		    if ( offsets[cnt].used ) {
			ptr = (char *)offsets[cnt].offset;
			/*
			 * copy buffer characters into line for printing
			 */
			lptr = line;
			while ( *ptr != '\n' ) 
			    *lptr++ = *ptr++;
				
			printf("%s\n", line);
		    }
		}

		/*
	         * move start of partically read line to beginning of buffer
		 * and adjust rdbuff to end of partically read line
		 */
		memcpy((void *)loffset, buffer, (stopaddr - loffset));
		rdbuff = buffer + (stopaddr - loffset);
		sztord = space - (stopaddr - loffset);

	        newbuffer++;
	    }

	    c=rnd_insert(offsets, loffset, numlines);

            loffset = (long)chr;
	}
    }

    return 0;

}

