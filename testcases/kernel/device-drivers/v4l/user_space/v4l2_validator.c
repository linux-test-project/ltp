/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 30 Jan 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include <linux/videodev2.h>
#include <linux/errno.h>

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"
#include "v4l2_validator.h"

int valid_v4l2_std_id(v4l2_std_id std_id)
{
	int valid = 0;

	if ((std_id & ~(V4L2_STD_PAL_B |
			V4L2_STD_PAL_B1 |
			V4L2_STD_PAL_G |
			V4L2_STD_PAL_H |
			V4L2_STD_PAL_I |
			V4L2_STD_PAL_D |
			V4L2_STD_PAL_D1 |
			V4L2_STD_PAL_K |
			V4L2_STD_PAL_M |
			V4L2_STD_PAL_N |
			V4L2_STD_PAL_Nc |
			V4L2_STD_PAL_60 |
			V4L2_STD_NTSC_M |
			V4L2_STD_NTSC_M_JP |
			V4L2_STD_NTSC_443 |
			V4L2_STD_NTSC_M_KR |
			V4L2_STD_SECAM_B |
			V4L2_STD_SECAM_D |
			V4L2_STD_SECAM_G |
			V4L2_STD_SECAM_H |
			V4L2_STD_SECAM_K |
			V4L2_STD_SECAM_K1 |
			V4L2_STD_SECAM_L |
			V4L2_STD_SECAM_LC |
			V4L2_STD_ATSC_8_VSB | V4L2_STD_ATSC_16_VSB))
	    == 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

int valid_tuner_capability(__u32 capability)
{
	int valid = 0;

	if ((capability & ~(V4L2_TUNER_CAP_LOW |
			    V4L2_TUNER_CAP_NORM |
			    V4L2_TUNER_CAP_STEREO |
			    V4L2_TUNER_CAP_LANG1 |
			    V4L2_TUNER_CAP_LANG2 | V4L2_TUNER_CAP_SAP))
	    == 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

int valid_modulator_capability(__u32 capability)
{
	return valid_tuner_capability(capability);
}

int valid_string(char *str, unsigned int max_length)
{
	int valid = 1;
	unsigned int i;

	i = 0;
	while (i < max_length && str[i] != 0) {
		/* Printable characters start with SPACE (32) and
		 * ends with '~' (126) inclusively.
		 */
		if (str[i] < 32 || 126 < str[i]) {
			valid = 0;
			break;
		}
		i++;
	}

	/* check if the string was closed with zero byte properly */
	if (i == max_length) {
		valid = 0;
	}

	return valid;
}
