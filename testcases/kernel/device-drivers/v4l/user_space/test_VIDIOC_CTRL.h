/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 19 Feb 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_G_CTRL(void);
void test_VIDIOC_G_CTRL_NULL(void);

void test_VIDIOC_S_CTRL(void);
void test_VIDIOC_S_CTRL_invalid(void);
void test_VIDIOC_S_CTRL_white_balance(void);
void test_VIDIOC_S_CTRL_white_balance_invalid(void);
void test_VIDIOC_S_CTRL_gain(void);
void test_VIDIOC_S_CTRL_gain_invalid(void);
void test_VIDIOC_S_CTRL_NULL(void);
