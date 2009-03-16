/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 12 Mar 2009  0.4  S16_MIN, S16_MAX, U16_MIN and U16_MAX added
 * 22 Feb 2009  0.3  S32_MIN and U32_MIN added
 *  1 Jan 2009  0.2  SINT_MAX and SINT_MIN added
 * 18 Dec 2008  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

#include <limits.h>

#define S32_MIN		((__s32)0x80000000)
#define S32_MAX		0x7FFFFFFF

#define U32_MIN		0
#define U32_MAX		0xFFFFFFFFU

#define S16_MIN		-32768
#define S16_MAX		32767

#define U16_MIN		0
#define U16_MAX		0xFFFFU

#define SINT_MIN        INT_MIN
#define SINT_MAX        INT_MAX
