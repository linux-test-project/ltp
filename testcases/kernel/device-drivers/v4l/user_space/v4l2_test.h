/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  1 Jan 2008  0.2  Include stdio.h if needed;
 *                   dprintf1() added
 * 18 Dec 2008  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

//#define DEBUG 1

#ifdef DEBUG
#include <stdio.h>
#define dprintf1(fmt)	printf(fmt)
#define dprintf(fmt, ...)	printf(fmt, __VA_ARGS__)
#else
#define dprintf1(fmt)
#define dprintf(fmt, ...)
#endif
