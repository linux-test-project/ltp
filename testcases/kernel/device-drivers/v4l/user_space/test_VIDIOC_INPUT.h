/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 22 Mar 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_G_INPUT(void);
void test_VIDIOC_S_INPUT_from_enum(void);
void test_VIDIOC_S_INPUT_invalid_inputs(void);

void test_VIDIOC_G_INPUT_NULL(void);
void test_VIDIOC_S_INPUT_NULL(void);
