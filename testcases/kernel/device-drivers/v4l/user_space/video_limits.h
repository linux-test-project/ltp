/*
 * v4l-test: Test environment for Video For Linux Two API
 *
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

#define SINT_MIN        INT_MIN
#define SINT_MAX        INT_MAX
