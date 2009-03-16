/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  7 Mar 2009  0.3  Test cases added for VIDIOC_S_CROP
 * 13 Feb 2009  0.2  Test cases added for VIDIOC_G_CROP
 *  7 Feb 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_G_CROP(void);
void test_VIDIOC_G_CROP_invalid(void);
void test_VIDIOC_G_CROP_NULL(void);
void test_VIDIOC_S_CROP(void);
void test_VIDIOC_S_CROP_invalid(void);
void test_VIDIOC_S_CROP_NULL(void);
