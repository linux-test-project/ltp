/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 19 May 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_G_EXT_CTRLS_zero(void);
void test_VIDIOC_G_EXT_CTRLS_zero_invalid_count(void);
void test_VIDIOC_G_EXT_CTRLS_one(void);
void test_VIDIOC_G_EXT_CTRLS_NULL(void);

void test_VIDIOC_S_EXT_CTRLS_zero(void);
void test_VIDIOC_S_EXT_CTRLS_zero_invalid_count(void);
void test_VIDIOC_S_EXT_CTRLS_NULL(void);

void test_VIDIOC_TRY_EXT_CTRLS_zero(void);
void test_VIDIOC_TRY_EXT_CTRLS_zero_invalid_count(void);
void test_VIDIOC_TRY_EXT_CTRLS_NULL(void);
