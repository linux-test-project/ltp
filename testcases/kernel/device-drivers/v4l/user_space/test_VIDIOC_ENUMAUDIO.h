/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  1 Jan 2009  0.3  Added index=S32_MAX, S32_MAX+1 and U32_MAX
 * 22 Dec 2008  0.2  Test case with NULL parameter added
 * 18 Dec 2008  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_ENUMAUDIO(void);
void test_VIDIOC_ENUMAUDIO_S32_MAX(void);
void test_VIDIOC_ENUMAUDIO_S32_MAX_1(void);
void test_VIDIOC_ENUMAUDIO_U32_MAX(void);
void test_VIDIOC_ENUMAUDIO_NULL(void);
