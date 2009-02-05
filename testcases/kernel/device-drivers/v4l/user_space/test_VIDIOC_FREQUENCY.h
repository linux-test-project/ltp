/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  1 Feb 2009  0.2  Added test cases for VIDIOC_S_FREQUENCY
 * 31 Jan 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_G_FREQUENCY(void);
void test_VIDIOC_G_FREQUENCY_S32_MAX(void);
void test_VIDIOC_G_FREQUENCY_S32_MAX_1(void);
void test_VIDIOC_G_FREQUENCY_U32_MAX(void);
void test_VIDIOC_G_FREQUENCY_NULL(void);

void test_VIDIOC_S_FREQUENCY(void);
void test_VIDIOC_S_FREQUENCY_boundaries(void);
void test_VIDIOC_S_FREQUENCY_scan(void);
