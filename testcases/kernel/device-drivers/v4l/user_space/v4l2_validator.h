/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 30 Jan 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

#include <linux/videodev2.h>

int valid_v4l2_std_id(v4l2_std_id std_id);
int valid_tuner_capability(__u32 capability);
int valid_modulator_capability(__u32 capability);
