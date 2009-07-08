/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  5 Jul 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

#include <linux/videodev2.h>

typedef void (*V4L2InputTestFunc)(int ret_input_enum, int errno_input_enum, struct v4l2_input* input);

void foreach_input(V4L2InputTestFunc pFunc);
