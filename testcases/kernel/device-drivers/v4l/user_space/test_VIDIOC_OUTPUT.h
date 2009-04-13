/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 22 Dec 2008  0.2  Test case with NULL parameter added
 * 18 Dec 2008  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_G_OUTPUT(void);
void test_VIDIOC_S_OUTPUT_from_enum(void);
void test_VIDIOC_S_OUTPUT_invalid_outputs(void);

void test_VIDIOC_G_OUTPUT_NULL(void);
void test_VIDIOC_S_OUTPUT_NULL(void);
