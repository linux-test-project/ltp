/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  7 May 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

#include <linux/videodev2.h>

void show_v4l2_requestbuffers(struct v4l2_requestbuffers *reqbuf);
void show_v4l2_buffer(struct v4l2_buffer *buf);
