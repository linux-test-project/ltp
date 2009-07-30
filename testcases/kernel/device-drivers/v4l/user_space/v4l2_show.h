/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 17 Jul 2009  0.3  show_v4l2_frmsizeenum() added
 *  5 Jul 2009  0.2  show_v4l2_input() introduced
 *  7 May 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

#include <linux/videodev2.h>

void show_v4l2_requestbuffers(struct v4l2_requestbuffers *reqbuf);
void show_v4l2_buffer(struct v4l2_buffer *buf);
void show_v4l2_input(struct v4l2_input *input);
void show_v4l2_frmsizeenum(struct v4l2_frmsizeenum* framesize);
