/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  7 Feb 2009  0.2  Test case test_VIDIOC_G_AUDIO_ignore_index added
 *  3 Feb 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_G_AUDIO(void);
void test_VIDIOC_G_AUDIO_ignore_index(void);
void test_VIDIOC_G_AUDIO_NULL(void);

void test_VIDIOC_S_AUDIO(void);
void test_VIDIOC_S_AUDIO_S32_MAX(void);
void test_VIDIOC_S_AUDIO_S32_MAX_1(void);
void test_VIDIOC_S_AUDIO_U32_MAX(void);
void test_VIDIOC_S_AUDIO_NULL(void);
