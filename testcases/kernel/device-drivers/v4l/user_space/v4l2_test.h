/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 18 Dec 2008  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

//#define DEBUG 1

#ifdef DEBUG
#define dprintf(fmt, ...)	printf(fmt, __VA_ARGS__)
#else
#define dprintf(fmt, ...)
#endif

