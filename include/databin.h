#ifndef _DATABIN_H_
#define _DATABIN_H_

/*******************************************************************************
* NAME
*       databingen - fill a buffer with a data pattern
*
* SYNOPSIS
*       (void) databingen(mode, buffer, bsize, offset)
*       int     mode;
*       char    *buffer;
*       int     bsize;
*	int 	offset;
*
* DESCRIPTION
*       datagen fills the buffer pointed to by 'buffer' with 'bsize' bytes
*       of data of the form indicated by 'mode'.  
*	All modes (expect r -random) are file offset based.
*	This allows more than process to do writing to the file without
*	corrupting it if the same modes were used.
*	They data modes to choose from, these are:
*
*               'a' - writes an alternating bit pattern (i.e. 0x5555555...)
*
*               'c' - writes a checkerboard pattern (i.e. 0xff00ff00ff00...)
*
*		'C' - writes counting pattern (i.e. 0 - 07, 0 - 07, ...);
*
*		'o' - writes all bits set (i.e. 0xffffffffffffff...)
*
*		'z' - writes all bits cleared (i.e. 0x000000000...);
*
*               'r' - writes random integers
*
* RETURN VALUE
*       None
*
*******************************************************************************/

void databingen( int mode, unsigned char *buffer, int bsize, int offset );

void databinchedk( int mode, unsigned char *buffer, int bsize, int offset, char **errmsg);

#endif
