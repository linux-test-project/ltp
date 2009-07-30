/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 16 Jul 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_ENUM_FRAMESIZES(void);
void test_VIDIOC_ENUM_FRAMESIZES_invalid_index(void);
void test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(void);
void test_VIDIOC_ENUM_FRAMESIZES_NULL(void);
