/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 31 Jan 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_G_TUNER(void);
void test_VIDIOC_G_TUNER_S32_MAX(void);
void test_VIDIOC_G_TUNER_S32_MAX_1(void);
void test_VIDIOC_G_TUNER_U32_MAX(void);
void test_VIDIOC_G_TUNER_NULL(void);

void test_VIDIOC_S_TUNER(void);
void test_VIDIOC_S_TUNER_invalid(void);
void test_VIDIOC_S_TUNER_NULL(void);
