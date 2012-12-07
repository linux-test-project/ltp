/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 16 Jul 2009  0.24 Test cases added for VIDIOC_G_JPEGCOMP and VIDIOC_ENUM_FRAMESIZES
 * 23 May 2009  0.23 Test cases added for VIDIOC_G_EXT_CTRLS, VIDIOC_S_EXT_CTRLS
 *  5 May 2009  0.22 Test cases added for VIDIOC_QUERYBUF
 * 29 Apr 2009  0.21 Test cases added for VIDIOC_REQBUFS
 * 18 Apr 2009  0.20 NULL parameter test suite split to read only, write only
 *                   and write/read ioctl suite
 * 16 Apr 2009  0.19 Test cases added for VIDIOC_S_FMT
 *  5 Apr 2009  0.18 Test cases for VIDIOC_QUERYMENU added
 *  4 Apr 2009  0.17 Test cases for VIDIOC_G_FMT added
 * 29 Mar 2009  0.16 Test case for VIDIOC_S_FREQUENCY with NULL parameter added
 * 22 Mar 2009  0.15 Test cases added for VIDIOC_G_OUTPUT and VIDIOC_S_OUTPUT
 * 18 Mar 2009  0.14 Test cases added for VIDIOC_G_PARM
 *  7 Mar 2009  0.13 Test cases added for VIDIOC_S_CROP
 * 22 Feb 2009  0.12 Test cases added for VIDIOC_S_CTRL
 * 19 Feb 2009  0.11 Test cases added for VIDIOC_G_CTRL
 *  7 Feb 2009  0.10 Test cases added for VIDIOC_G_AUDIO, VIDIOC_G_AUDOUT,
 *                   VIDIOC_S_AUDIO and VIDIOC_G_CROP
 *  3 Feb 2009  0.9  Test cases for VIDIOC_G_AUDIO and VIDIOC_G_AUDOUT added
 *  2 Feb 2009  0.8  Test cases for VIDIOC_G_MODULATOR, VIDIOC_G_PRIORITY
 *                   and VIDIOC_S_PRIORITY added
 *  1 Feb 2009  0.7  Test cases for VIDIOC_S_FREQUENCY added
 * 31 Jan 2009  0.6  Test cases for VIDIOC_G_TUNER added
 * 18 Jan 2009  0.5  Test cases for MAX_EM28XX_INPUT and MAX_EM28XX_TVNORMS
 *                   removed
 *  1 Jan 2009  0.4  Test cases for VIDIOC_ENUMOUTPUT, VIDIOC_ENUMAUDOUT,
 *                   VIDIOC_QUERYCTRL added;
 *                   New test cases for VIDIOC_ENUMAUDIO, VIDIOC_ENUM_FMT,
 *                   VIDIOC_ENUM_STD
 * 23 Dec 2008  0.3  Test cases for VIDIOC_LOG_STATUS added
 * 22 Dec 2008  0.2  Test cases with NULL parameter added;
 *                   Test cases for VIDIOC_CROPCAP added
 * 18 Dec 2008  0.1  First release
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

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"

#include "test_VIDIOC_QUERYCAP.h"
#include "test_VIDIOC_QUERYSTD.h"
#include "test_VIDIOC_QUERYCTRL.h"
#include "test_VIDIOC_QUERYMENU.h"
#include "test_VIDIOC_CROPCAP.h"
#include "test_VIDIOC_G_SLICED_VBI_CAP.h"

#include "test_VIDIOC_ENUMAUDIO.h"
#include "test_VIDIOC_ENUMAUDOUT.h"
#include "test_VIDIOC_ENUMSTD.h"
#include "test_VIDIOC_ENUM_FMT.h"
#include "test_VIDIOC_ENUMINPUT.h"
#include "test_VIDIOC_ENUMOUTPUT.h"
#include "test_VIDIOC_ENUM_FRAMESIZES.h"

#include "test_VIDIOC_STD.h"
#include "test_VIDIOC_INPUT.h"
#include "test_VIDIOC_OUTPUT.h"
#include "test_VIDIOC_TUNER.h"
#include "test_VIDIOC_MODULATOR.h"
#include "test_VIDIOC_FREQUENCY.h"
#include "test_VIDIOC_PRIORITY.h"
#include "test_VIDIOC_AUDIO.h"
#include "test_VIDIOC_AUDOUT.h"
#include "test_VIDIOC_CROP.h"
#include "test_VIDIOC_CTRL.h"
#include "test_VIDIOC_EXT_CTRLS.h"
#include "test_VIDIOC_PARM.h"
#include "test_VIDIOC_FMT.h"
#include "test_VIDIOC_JPEGCOMP.h"

#include "test_VIDIOC_REQBUFS.h"
#include "test_VIDIOC_QUERYBUF.h"

#include "test_VIDIOC_LOG_STATUS.h"
#include "test_invalid_ioctl.h"

static CU_TestInfo suite_querycap[] = {
	{"VIDIOC_QUERYCAP", test_VIDIOC_QUERYCAP},

	{"VIDIOC_CROPCAP", test_VIDIOC_CROPCAP},
	{"VIDIOC_CROPCAP with different inputs",
	 test_VIDIOC_CROPCAP_enum_INPUT},

	{"VIDIOC_G_SLICED_VBI_CAP", test_VIDIOC_G_SLICED_VBI_CAP},
	{"VIDIOC_G_SLICED_VBI_CAP with invalid types",
	 test_VIDIOC_G_SLICED_VBI_CAP_invalid},

	CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_enums[] = {
	{"VIDIOC_ENUMAUDIO", test_VIDIOC_ENUMAUDIO},
	{"VIDIOC_ENUMAUDIO, index=S32_MAX", test_VIDIOC_ENUMAUDIO_S32_MAX},
	{"VIDIOC_ENUMAUDIO, index=S32_MAX+1", test_VIDIOC_ENUMAUDIO_S32_MAX_1},
	{"VIDIOC_ENUMAUDIO, index=U32_MAX", test_VIDIOC_ENUMAUDIO_U32_MAX},

	{"VIDIOC_ENUMAUDOUT", test_VIDIOC_ENUMAUDOUT},
	{"VIDIOC_ENUMAUDOUT, index=S32_MAX", test_VIDIOC_ENUMAUDOUT_S32_MAX},
	{"VIDIOC_ENUMAUDOUT, index=S32_MAX+1",
	 test_VIDIOC_ENUMAUDOUT_S32_MAX_1},
	{"VIDIOC_ENUMAUDOUT, index=U32_MAX", test_VIDIOC_ENUMAUDOUT_U32_MAX},

	{"VIDIOC_ENUM_FMT", test_VIDIOC_ENUM_FMT},
	{"VIDIOC_ENUM_FMT, index=S32_MAX", test_VIDIOC_ENUM_FMT_S32_MAX},
	{"VIDIOC_ENUM_FMT, index=S32_MAX+1", test_VIDIOC_ENUM_FMT_S32_MAX_1},
	{"VIDIOC_ENUM_FMT, index=U32_MAX", test_VIDIOC_ENUM_FMT_U32_MAX},
	{"VIDIOC_ENUM_FMT, invalid type", test_VIDIOC_ENUM_FMT_invalid_type},

	{"VIDIOC_ENUMINPUT", test_VIDIOC_ENUMINPUT},
	{"VIDIOC_ENUMINPUT, index=S32_MAX", test_VIDIOC_ENUMINPUT_S32_MAX},
	{"VIDIOC_ENUMINPUT, index=S32_MAX+1", test_VIDIOC_ENUMINPUT_S32_MAX_1},
	{"VIDIOC_ENUMINPUT, index=U32_MAX", test_VIDIOC_ENUMINPUT_U32_MAX},

	{"VIDIOC_ENUMOUTPUT", test_VIDIOC_ENUMOUTPUT},
	{"VIDIOC_ENUMOUTPUT, index=S32_MAX", test_VIDIOC_ENUMOUTPUT_S32_MAX},
	{"VIDIOC_ENUMOUTPUT, index=S32_MAX+1",
	 test_VIDIOC_ENUMOUTPUT_S32_MAX_1},
	{"VIDIOC_ENUMOUTPUT, index=U32_MAX", test_VIDIOC_ENUMOUTPUT_U32_MAX},

	{"VIDIOC_ENUMSTD", test_VIDIOC_ENUMSTD},
	{"VIDIOC_ENUMSTD, index=S32_MAX", test_VIDIOC_ENUMSTD_S32_MAX},
	{"VIDIOC_ENUMSTD, index=S32_MAX+1", test_VIDIOC_ENUMSTD_S32_MAX_1},
	{"VIDIOC_ENUMSTD, index=U32_MAX", test_VIDIOC_ENUMSTD_U32_MAX},

	{"VIDIOC_QUERYCTRL", test_VIDIOC_QUERYCTRL},
	{"VIDIOC_QUERYCTRL, id=V4L2_CID_BASE-1", test_VIDIOC_QUERYCTRL_BASE_1},
	{"VIDIOC_QUERYCTRL, id=V4L2_CID_LASTP1", test_VIDIOC_QUERYCTRL_LASTP1},
	{"VIDIOC_QUERYCTRL, id=V4L2_CID_LASTP1+1",
	 test_VIDIOC_QUERYCTRL_LASTP1_1},
	{"VIDIOC_QUERYCTRL with V4L2_CTRL_FLAG_NEXT_CTRL",
	 test_VIDIOC_QUERYCTRL_flag_NEXT_CTRL},
	{"VIDIOC_QUERYCTRL, enumerate private controls",
	 test_VIDIOC_QUERYCTRL_private},
	{"VIDIOC_QUERYCTRL, V4L2_CID_PRIVATE_BASE-1",
	 test_VIDIOC_QUERYCTRL_private_base_1},
	{"VIDIOC_QUERYCTRL, last private control+1",
	 test_VIDIOC_QUERYCTRL_private_last_1},

	{"VIDIOC_QUERYMENU", test_VIDIOC_QUERYMENU},
	{"VIDIOC_QUERYMENU with invalid id", test_VIDIOC_QUERYMENU_invalid},
	{"VIDIOC_QUERYMENU with private controls",
	 test_VIDIOC_QUERYMENU_private},
	{"VIDIOC_QUERYMENU, last private control+1",
	 test_VIDIOC_QUERYMENU_private_last_1},

	{"VIDIOC_ENUM_FRAMESIZES", test_VIDIOC_ENUM_FRAMESIZES},
	{"VIDIOC_ENUM_FRAMESIZES with invalid index",
	 test_VIDIOC_ENUM_FRAMESIZES_invalid_index},
	{"VIDIOC_ENUM_FRAMESIZES with invalid pixel_format",
	 test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format},

	CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_get_set_try[] = {
	{"VIDIOC_G_STD", test_VIDIOC_G_STD},
	{"VIDIOC_S_STD with the enumerated values",
	 test_VIDIOC_S_STD_from_enum},
	{"VIDIOC_S_STD", test_VIDIOC_S_STD},
	{"VIDIOC_S_STD with invalid standard",
	 test_VIDIOC_S_STD_invalid_standard},

	{"VIDIOC_G_INPUT", test_VIDIOC_G_INPUT},
	{"VIDIOC_S_INPUT from enum", test_VIDIOC_S_INPUT_from_enum},
	{"VIDIOC_S_INPUT with invalid inputs",
	 test_VIDIOC_S_INPUT_invalid_inputs},

	{"VIDIOC_G_OUTPUT", test_VIDIOC_G_OUTPUT},
	{"VIDIOC_S_OUTPUT from enum", test_VIDIOC_S_OUTPUT_from_enum},
	{"VIDIOC_S_OUTPUT with invalid outputs",
	 test_VIDIOC_S_OUTPUT_invalid_outputs},

	{"VIDIOC_G_TUNER", test_VIDIOC_G_TUNER},
	{"VIDIOC_G_TUNER, index=S32_MAX", test_VIDIOC_G_TUNER_S32_MAX},
	{"VIDIOC_G_TUNER, index=S32_MAX+1", test_VIDIOC_G_TUNER_S32_MAX_1},
	{"VIDIOC_G_TUNER, index=U32_MAX", test_VIDIOC_G_TUNER_U32_MAX},

	{"VIDIOC_S_TUNER", test_VIDIOC_S_TUNER},
	{"VIDIOC_S_TUNER with invalid index and audmode parameters",
	 test_VIDIOC_S_TUNER_invalid},

	{"VIDIOC_G_MODULATOR", test_VIDIOC_G_MODULATOR},
	{"VIDIOC_G_MODULATOR, index=S32_MAX", test_VIDIOC_G_MODULATOR_S32_MAX},
	{"VIDIOC_G_MODULATOR, index=S32_MAX+1",
	 test_VIDIOC_G_MODULATOR_S32_MAX_1},
	{"VIDIOC_G_MODULATOR, index=U32_MAX", test_VIDIOC_G_MODULATOR_U32_MAX},

	{"VIDIOC_G_FREQUENCY", test_VIDIOC_G_FREQUENCY},
	{"VIDIOC_G_FREQUENCY, tuner=S32_MAX", test_VIDIOC_G_FREQUENCY_S32_MAX},
	{"VIDIOC_G_FREQUENCY, tuner=S32_MAX+1",
	 test_VIDIOC_G_FREQUENCY_S32_MAX_1},
	{"VIDIOC_G_FREQUENCY, tuner=U32_MAX", test_VIDIOC_G_FREQUENCY_U32_MAX},

	{"VIDIOC_S_FREQUENCY", test_VIDIOC_S_FREQUENCY},
	{"VIDIOC_S_FREQUENCY with boundary values",
	 test_VIDIOC_S_FREQUENCY_boundaries},
	{"VIDIOC_S_FREQUENCY scan all possbile values",
	 test_VIDIOC_S_FREQUENCY_scan},

	{"VIDIOC_G_PRIORITY", test_VIDIOC_G_PRIORITY},
	{"VIDIOC_S_PRIORITY", test_VIDIOC_S_PRIORITY},
	{"VIDIOC_S_PRIORITY with invalid values",
	 test_VIDIOC_S_PRIORITY_invalid},

	{"VIDIOC_G_AUDIO", test_VIDIOC_G_AUDIO},
	{"VIDIOC_G_AUDIO, ignore index value",
	 test_VIDIOC_G_AUDIO_ignore_index},

	{"VIDIOC_S_AUDIO", test_VIDIOC_S_AUDIO},
	{"VIDIOC_S_AUDIO, index=S32_MAX", test_VIDIOC_S_AUDIO_S32_MAX},
	{"VIDIOC_S_AUDIO, index=S32_MAX+1", test_VIDIOC_S_AUDIO_S32_MAX_1},
	{"VIDIOC_S_AUDIO, index=U32_MAX", test_VIDIOC_S_AUDIO_U32_MAX},

	{"VIDIOC_G_AUDOUT", test_VIDIOC_G_AUDOUT},
	{"VIDIOC_G_AUDOUT, ignore index value",
	 test_VIDIOC_G_AUDOUT_ignore_index},

	{"VIDIOC_S_AUDOUT", test_VIDIOC_S_AUDOUT},
	{"VIDIOC_S_AUDOUT, index=S32_MAX", test_VIDIOC_S_AUDOUT_S32_MAX},
	{"VIDIOC_S_AUDOUT, index=S32_MAX+1", test_VIDIOC_S_AUDOUT_S32_MAX_1},
	{"VIDIOC_S_AUDOUT, index=U32_MAX", test_VIDIOC_S_AUDOUT_U32_MAX},

	{"VIDIOC_G_CROP", test_VIDIOC_G_CROP},
	{"VIDIOC_G_CROP with invalid type", test_VIDIOC_G_CROP_invalid},
	{"VIDIOC_S_CROP", test_VIDIOC_S_CROP},
	{"VIDIOC_S_CROP with invalid type", test_VIDIOC_S_CROP_invalid},

	{"VIDIOC_G_CTRL", test_VIDIOC_G_CTRL},

	{"VIDIOC_S_CTRL", test_VIDIOC_S_CTRL},
	{"VIDIOC_S_CTRL with invalid value parameter",
	 test_VIDIOC_S_CTRL_invalid},
	{"VIDIOC_S_CTRL, withe balance", test_VIDIOC_S_CTRL_white_balance},
	{"VIDIOC_S_CTRL, white balance with invalid value parameter",
	 test_VIDIOC_S_CTRL_white_balance_invalid},
	{"VIDIOC_S_CTRL, gain control", test_VIDIOC_S_CTRL_gain},
	{"VIDIOC_S_CTRL, gain control with invalid value parameter",
	 test_VIDIOC_S_CTRL_gain_invalid},

	{"VIDIOC_G_EXT_CTRLS with zero items to get",
	 test_VIDIOC_G_EXT_CTRLS_zero},
	{"VIDIOC_G_EXT_CTRLS with zero items to get, but with invalid count values",
	 test_VIDIOC_G_EXT_CTRLS_zero_invalid_count},
	{"VIDIOC_G_EXT_CTRLS with only one item to get",
	 test_VIDIOC_G_EXT_CTRLS_one},

	{"VIDIOC_S_EXT_CTRLS with zero items to set",
	 test_VIDIOC_S_EXT_CTRLS_zero},
	{"VIDIOC_S_EXT_CTRLS with zero items to set, but with invalid count values",
	 test_VIDIOC_S_EXT_CTRLS_zero_invalid_count},

	{"VIDIOC_TRY_EXT_CTRLS with zero items to try",
	 test_VIDIOC_TRY_EXT_CTRLS_zero},
	{"VIDIOC_TRY_EXT_CTRLS with zero items to try, but with invalid count values",
	 test_VIDIOC_TRY_EXT_CTRLS_zero_invalid_count},

	{"VIDIOC_G_PARM", test_VIDIOC_G_PARM},
	{"VIDIOC_G_PARM with invalid type parameter",
	 test_VIDIOC_G_PARM_invalid},

	{"VIDIOC_G_FMT", test_VIDIOC_G_FMT},
	{"VIDIOC_G_FMT with invalid type parameter",
	 test_VIDIOC_G_FMT_invalid_type},

	{"VIDIOC_S_FMT with enumerated values", test_VIDIOC_S_FMT_enum},
	{"VIDIOC_S_FMT with invalid type parameter", test_VIDIOC_S_FMT_type},

	{"VIDIOC_G_JPEGCOMP", test_VIDIOC_G_JPEGCOMP},

	CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_querystd[] = {
	{"VIDIOC_QUERYSTD", test_VIDIOC_QUERYSTD},

	CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_buffs[] = {
	{"VIDIOC_REQBUFS with memory map capture streaming i/o",
	 test_VIDIOC_REQBUFS_capture_mmap},
	{"VIDIOC_REQBUFS with user pointer capture streaming i/o",
	 test_VIDIOC_REQBUFS_capture_userptr},
	{"VIDIOC_REQBUFS with memory map output streaming i/o",
	 test_VIDIOC_REQBUFS_output_mmap},
	{"VIDIOC_REQBUFS with user pointer output streaming i/o",
	 test_VIDIOC_REQBUFS_output_userptr},
	{"VIDIOC_REQBUFS with invalid memory parameter, capture",
	 test_VIDIOC_REQBUFS_invalid_memory_capture},
	{"VIDIOC_REQBUFS with invalid memory parameter, output",
	 test_VIDIOC_REQBUFS_invalid_memory_output},
	{"VIDIOC_REQBUFS with invalid type parameter, memory mapped i/o",
	 test_VIDIOC_REQUBUFS_invalid_type_mmap},
	{"VIDIOC_REQBUFS with invalid type parameter, user pointer i/o",
	 test_VIDIOC_REQUBUFS_invalid_type_userptr},

	{"VIDIOC_QUERYBUF with memory map capture streaming i/o",
	 test_VIDIOC_QUERYBUF_capture_mmap},
	{"VIDIOC_QUERYBUF with user pointer capture streaming i/o",
	 test_VIDIOC_QUERYBUF_capture_userptr},
	{"VIDIOC_QUERYBUF with memory map output streaming i/o",
	 test_VIDIOC_QUERYBUF_output_mmap},
	{"VIDIOC_QUERYBUF with user pointer output streaming i/o",
	 test_VIDIOC_QUERYBUF_output_userptr},
	{"VIDIOC_QUERYBUF with overlay capture (invalid)",
	 test_VIDIOC_QUERYBUF_overlay_capture},
	{"VIDIOC_QUERYBUF with overlay output (invalid)",
	 test_VIDIOC_QUERYBUF_overlay_output},
	{"VIDIOC_QUERYBUF with invalid memory parameter, capture",
	 test_VIDIOC_QUERYBUF_invalid_memory_capture},
	{"VIDIOC_QUERYBUF with invalid memory parameter, output",
	 test_VIDIOC_QUERYBUF_invalid_memory_output},
	{"VIDIOC_QUERYBUF with invalid type parameter, memory mapped i/o",
	 test_VIDIOC_QUERYBUF_invalid_type_mmap},
	{"VIDIOC_QUERYBUF with invalid type parameter, user pointer i/o",
	 test_VIDIOC_QUERYBUF_invalid_type_userptr},

	CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_null_readonly[] = {
	{"VIDIOC_QUERYCAP with NULL parameter", test_VIDIOC_QUERYCAP_NULL},
	/* { "VIDIOC_G_FBUF with NULL parameter", }, */
	{"VIDIOC_G_STD with NULL parameter", test_VIDIOC_G_STD_NULL},
	{"VIDIOC_G_AUDIO with NULL parameter", test_VIDIOC_G_AUDIO_NULL},
	{"VIDIOC_G_INPUT with NULL parameter", test_VIDIOC_G_INPUT_NULL},
	{"VIDIOC_G_OUTPUT with NULL parameter", test_VIDIOC_G_OUTPUT_NULL},
	{"VIDIOC_G_AUDOUT with NULL parameter", test_VIDIOC_G_AUDOUT_NULL},
	{"VIDIOC_G_JPEGCOMP with NULL parameter", test_VIDIOC_G_JPEGCOMP_NULL},
	{"VIDIOC_QUERYSTD with NULL parameter", test_VIDIOC_QUERYSTD_NULL},
	{"VIDIOC_G_PRIORITY with NULL parameter", test_VIDIOC_G_PRIORITY_NULL},
	/* { "VIDIOC_G_ENC_INDEX with NULL parameter", }, */

	CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_null_writeonly[] = {
	/* { "VIDIOC_S_FBUF with NULL parameter", }, */
	/* { "VIDIOC_OVERLAY with NULL parameter", }, */
	/* { "VIDIOC_STREAMON with NULL parameter", }, */
	/* { "VIDIOC_STREAMOFF with NULL parameter", }, */
	{"VIDIOC_S_STD with NULL parameter", test_VIDIOC_S_STD_NULL},
	{"VIDIOC_S_TUNER with NULL parameter", test_VIDIOC_S_TUNER_NULL},
	{"VIDIOC_S_AUDIO with NULL parameter", test_VIDIOC_S_AUDIO_NULL},
	{"VIDIOC_S_AUDOUT with NULL parameter", test_VIDIOC_S_AUDOUT_NULL},
	/* { "VIDIOC_S_MODULATOR with NULL parameter", }, */
	{"VIDIOC_S_FREQUENCY with NULL parameter",
	 test_VIDIOC_S_FREQUENCY_NULL},
	{"VIDIOC_S_CROP with NULL parameter", test_VIDIOC_S_CROP_NULL},
	/* { "VIDIOC_S_JPEGCOMP with NULL parameter", }, */
	{"VIDIOC_S_PRIORITY with NULL parameter", test_VIDIOC_S_PRIORITY_NULL},
	/* { "VIDIOC_DBG_S_REGISTER with NULL parameter", }, */
	/* { "VIDIOC_S_HW_FREQ_SEEK with NULL parameter", }, */

	CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_null_writeread[] = {
	{"VIDIOC_ENUM_FMT with NULL parameter", test_VIDIOC_ENUM_FMT_NULL},
	{"VIDIOC_G_FMT with NULL parameter", test_VIDIOC_G_FMT_NULL},
	/* { "VIDIOC_S_FMT with NULL parameter", }, */
	/* { "VIDIOC_REQBUFS with NULL parameter", } */
	/* { "VIDIOC_QUERYBUF with NULL parameter", } */
	/* { "VIDIOC_QBUF with NULL parameter", }, */
	/* { "VIDIOC_DQBUF with NULL parameter", }, */
	{"VIDIOC_G_PARM with NULL parameter", test_VIDIOC_G_PARM_NULL},
	/* { "VIDIOC_S_PARM with NULL parameter", }, */
	{"VIDIOC_ENUMSTD with NULL parameter", test_VIDIOC_ENUMSTD_NULL},
	{"VIDIOC_ENUMINPUT with NULL parameter", test_VIDIOC_ENUMINPUT_NULL},
	{"VIDIOC_G_CTRL with NULL parameter", test_VIDIOC_G_CTRL_NULL},
	{"VIDIOC_S_CTRL with NULL parameter", test_VIDIOC_S_CTRL_NULL},
	{"VIDIOC_G_TUNER with NULL parameter", test_VIDIOC_G_TUNER_NULL},
	{"VIDIOC_QUERYCTRL with NULL parameter", test_VIDIOC_QUERYCTRL_NULL},
	{"VIDIOC_QUERYMENU with NULL parameter", test_VIDIOC_QUERYMENU_NULL},
	{"VIDIOC_S_INPUT with NULL parameter", test_VIDIOC_S_INPUT_NULL},
	{"VIDIOC_S_OUTPUT with NULL parameter", test_VIDIOC_S_OUTPUT_NULL},
	{"VIDIOC_ENUMOUTPUT with NULL parameter", test_VIDIOC_ENUMOUTPUT_NULL},
	{"VIDIOC_G_MODULATOR with NULL parameter",
	 test_VIDIOC_G_MODULATOR_NULL},
	{"VIDIOC_G_FREQUENCY with NULL parameter",
	 test_VIDIOC_G_FREQUENCY_NULL},
	{"VIDIOC_CROPCAP with NULL parameter", test_VIDIOC_CROPCAP_NULL},
	{"VIDIOC_G_CROP with NULL parameter", test_VIDIOC_G_CROP_NULL},
	/* { "VIDIOC_TRY_FMT with NULL parameter", }, */
	{"VIDIOC_ENUMAUDIO with NULL parameter", test_VIDIOC_ENUMAUDIO_NULL},
	{"VIDIOC_ENUMAUDOUT with NULL parameter", test_VIDIOC_ENUMAUDOUT_NULL},
	{"VIDIOC_G_SLICED_VBI_CAP with NULL parameter",
	 test_VIDIOC_G_SLICED_VBI_CAP_NULL},
	{"VIDIOC_G_EXT_CTRLS with NULL parameter",
	 test_VIDIOC_G_EXT_CTRLS_NULL},
	{"VIDIOC_S_EXT_CTRLS with NULL parameter",
	 test_VIDIOC_S_EXT_CTRLS_NULL},
	{"VIDIOC_TRY_EXT_CTRLS with NULL parameter",
	 test_VIDIOC_TRY_EXT_CTRLS_NULL},
	{"VIDIOC_ENUM_FRAMESIZES with NULL parameter",
	 test_VIDIOC_ENUM_FRAMESIZES_NULL},
	/* { "VIDIOC_ENUM_FRAMEINTERVALS with NULL parameter", }, */
	/* { "VIDIOC_ENCODER_CMD with NULL parameter", }, */
	/* { "VIDIOC_TRY_ENCODER_CMD with NULL parameter", }, */
	/* { "VIDIOC_DBG_G_REGISTER with NULL parameter", }, */
	/* { "VIDIOC_DBG_G_CHIP_IDENT with NULL parameter", }, */

	CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_invalid_ioctl[] = {
	{"invalid ioctl _IO(0, 0)", test_invalid_ioctl_1},
	{"invalid ioctl _IO(0xFF, 0xFF)", test_invalid_ioctl_2},
	{"invalid v4l1 ioctl _IO('v', 0xFF)", test_invalid_ioctl_3},
	{"invalid v4l2 ioctl _IO('V', 0xFF)", test_invalid_ioctl_4},

	CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_debug_ioctl[] = {
	{"test_VIDIOC_LOG_STATUS", test_VIDIOC_LOG_STATUS},

	CU_TEST_INFO_NULL,
};

static CU_SuiteInfo suites[] = {
	{"VIDIOC_QUERYCAP", open_video, close_video, suite_querycap},
	{"VIDIOC_ENUM* ioctl calls", open_video, close_video, suite_enums},
	{"VIDIOC_G_*, VIDIOC_S_* and VIDIOC_TRY_* ioctl calls", open_video,
	 close_video, suite_get_set_try},
	{"VIDIOC_QUERYSTD", open_video, close_video, suite_querystd},
	{"buffer i/o", open_video, close_video, suite_buffs},
	{"read only IOCTLs with NULL parameter", open_video, close_video,
	 suite_null_readonly},
	{"write only IOCTLs with NULL parameter", open_video, close_video,
	 suite_null_writeonly},
	{"write and read IOCTLs with NULL parameter", open_video, close_video,
	 suite_null_writeread},
	{"debug ioctl calls", open_video, close_video, suite_debug_ioctl},
	{"invalid ioctl calls", open_video, close_video, suite_invalid_ioctl},
	CU_SUITE_INFO_NULL,
};

int main()
{
	CU_ErrorCode err;

	err = CU_initialize_registry();
	if (err != CUE_SUCCESS) {
		printf("ERROR: cannot initialize CUNIT registry, giving up.\n");
		return 1;
	}

	err = CU_register_suites(suites);
	if (err == CUE_SUCCESS) {

		//CU_basic_set_mode(CU_BRM_NORMAL);
		//CU_basic_set_mode(CU_BRM_SILENT);
		CU_basic_set_mode(CU_BRM_VERBOSE);
		err = CU_basic_run_tests();
		if (err != CUE_SUCCESS) {
			printf("CU_basic_run_tests returned %i\n", err);
		}

	} else {
		printf("ERROR: cannot add test suites\n");
	}

	CU_cleanup_registry();

	tst_exit();
}
