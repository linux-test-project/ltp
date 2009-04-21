/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 15 Apr 2009  0.2  Added test cases for VIDIOC_S_FMT
 *  4 Apr 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_G_FMT(void);
void test_VIDIOC_G_FMT_invalid_type(void);
void test_VIDIOC_G_FMT_NULL(void);

void test_VIDIOC_S_FMT_enum(void);
void test_VIDIOC_S_FMT_type(void);

