/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 18 Jan 2009  0.5  Testa cases for MAX_EM28XX_INPUT and MAX_EM28XX_TVNORMS
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
#include "test_VIDIOC_QUERYCTRL.h"
#include "test_VIDIOC_CROPCAP.h"

#include "test_VIDIOC_ENUMAUDIO.h"
#include "test_VIDIOC_ENUMAUDOUT.h"
#include "test_VIDIOC_ENUMSTD.h"
#include "test_VIDIOC_ENUM_FMT.h"
#include "test_VIDIOC_ENUMINPUT.h"
#include "test_VIDIOC_ENUMOUTPUT.h"

#include "test_VIDIOC_STD.h"
#include "test_VIDIOC_INPUT.h"
#include "test_VIDIOC_LOG_STATUS.h"
#include "test_invalid_ioctl.h"


static CU_TestInfo suite_querycap[] = {
  { "VIDIOC_QUERYCAP", test_VIDIOC_QUERYCAP },

  { "VIDIOC_CROPCAP", test_VIDIOC_CROPCAP },
  { "VIDIOC_CROPCAP with different inputs", test_VIDIOC_CROPCAP_enum_INPUT },
  { "VIDIOC_CROPCAP with NULL parameter", test_VIDIOC_CROPCAP_NULL },

  CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_enums[] = {
  { "VIDIOC_ENUMAUDIO", test_VIDIOC_ENUMAUDIO },
  { "VIDIOC_ENUMAUDIO, index=S32_MAX", test_VIDIOC_ENUMAUDIO_S32_MAX },
  { "VIDIOC_ENUMAUDIO, index=S32_MAX+1", test_VIDIOC_ENUMAUDIO_S32_MAX_1 },
  { "VIDIOC_ENUMAUDIO, index=U32_MAX", test_VIDIOC_ENUMAUDIO_U32_MAX },
  { "VIDIOC_ENUMAUDIO with NULL parameter", test_VIDIOC_ENUMAUDIO_NULL },

  { "VIDIOC_ENUMAUDOUT", test_VIDIOC_ENUMAUDOUT },
  { "VIDIOC_ENUMAUDOUT, index=S32_MAX", test_VIDIOC_ENUMAUDOUT_S32_MAX },
  { "VIDIOC_ENUMAUDOUT, index=S32_MAX+1", test_VIDIOC_ENUMAUDOUT_S32_MAX_1 },
  { "VIDIOC_ENUMAUDOUT, index=U32_MAX", test_VIDIOC_ENUMAUDOUT_U32_MAX },
  { "VIDIOC_ENUMAUDOUT with NULL parameter", test_VIDIOC_ENUMAUDOUT_NULL },

  { "VIDIOC_ENUM_FMT", test_VIDIOC_ENUM_FMT },
  { "VIDIOC_ENUM_FMT, index=S32_MAX", test_VIDIOC_ENUM_FMT_S32_MAX },
  { "VIDIOC_ENUM_FMT, index=S32_MAX+1", test_VIDIOC_ENUM_FMT_S32_MAX_1 },
  { "VIDIOC_ENUM_FMT, index=U32_MAX", test_VIDIOC_ENUM_FMT_U32_MAX },
  { "VIDIOC_ENUM_FMT, invalid type", test_VIDIOC_ENUM_FMT_invalid_type },
  { "VIDIOC_ENUM_FMT with NULL parameter", test_VIDIOC_ENUM_FMT_NULL },

  { "VIDIOC_ENUMINPUT", test_VIDIOC_ENUMINPUT },
  { "VIDIOC_ENUMINPUT, index=S32_MAX", test_VIDIOC_ENUMINPUT_S32_MAX },
  { "VIDIOC_ENUMINPUT, index=S32_MAX+1", test_VIDIOC_ENUMINPUT_S32_MAX_1 },
  { "VIDIOC_ENUMINPUT, index=U32_MAX", test_VIDIOC_ENUMINPUT_U32_MAX },
  { "VIDIOC_ENUMINPUT with NULL parameter", test_VIDIOC_ENUMINPUT_NULL },

  { "VIDIOC_ENUMOUTPUT", test_VIDIOC_ENUMOUTPUT },
  { "VIDIOC_ENUMOUTPUT, index=S32_MAX", test_VIDIOC_ENUMOUTPUT_S32_MAX },
  { "VIDIOC_ENUMOUTPUT, index=S32_MAX+1", test_VIDIOC_ENUMOUTPUT_S32_MAX_1 },
  { "VIDIOC_ENUMOUTPUT, index=U32_MAX", test_VIDIOC_ENUMOUTPUT_U32_MAX },
  { "VIDIOC_ENUMOUTPUT with NULL parameter", test_VIDIOC_ENUMOUTPUT_NULL },

  { "VIDIOC_ENUMSTD", test_VIDIOC_ENUMSTD },
  { "VIDIOC_ENUMSTD, index=S32_MAX", test_VIDIOC_ENUMSTD_S32_MAX },
  { "VIDIOC_ENUMSTD, index=S32_MAX+1", test_VIDIOC_ENUMSTD_S32_MAX_1 },
  { "VIDIOC_ENUMSTD, index=U32_MAX", test_VIDIOC_ENUMSTD_U32_MAX },
  { "VIDIOC_ENUMSTD with NULL parameter", test_VIDIOC_ENUMSTD_NULL },

  { "VIDIOC_QUERYCTRL", test_VIDIOC_QUERYCTRL },
  { "VIDIOC_QUERYCTRL, id=V4L2_CID_BASE-1", test_VIDIOC_QUERYCTRL_BASE_1 },
  { "VIDIOC_QUERYCTRL, id=V4L2_CID_LASTP1", test_VIDIOC_QUERYCTRL_LASTP1 },
  { "VIDIOC_QUERYCTRL, id=V4L2_CID_LASTP1+1", test_VIDIOC_QUERYCTRL_LASTP1_1 },
  { "VIDIOC_QUERYCTRL with V4L2_CTRL_FLAG_NEXT_CTRL", test_VIDIOC_QUERYCTRL_flag_NEXT_CTRL },
  { "VIDIOC_QUERYCTRL, enumerate private controls", test_VIDIOC_QUERYCTRL_private },
  { "VIDIOC_QUERYCTRL, V4L2_CID_PRIVATE_BASE-1", test_VIDIOC_QUERYCTRL_private_base_1 },
  { "VIDIOC_QUERYCTRL, last private control+1", test_VIDIOC_QUERYCTRL_private_last_1 },
  { "VIDIOC_QUERYCTRL with NULL parameter", test_VIDIOC_QUERYCTRL_NULL },

  CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_get_set_try[] = {
  { "VIDIOC_G_STD", test_VIDIOC_G_STD },
  { "VIDIOC_S_STD with the enumerated values", test_VIDIOC_S_STD_from_enum },
  { "VIDIOC_S_STD", test_VIDIOC_S_STD },
  { "VIDIOC_S_STD with invalid standard", test_VIDIOC_S_STD_invalid_standard },
  { "VIDIOC_G_STD with NULL parameter", test_VIDIOC_G_STD_NULL },
  { "VIDIOC_S_STD with NULL parameter", test_VIDIOC_S_STD_NULL },

  { "VIDIOC_G_INPUT", test_VIDIOC_G_INPUT },
  { "VIDIOC_S_INPUT from enum", test_VIDIOC_S_INPUT_from_enum },
  { "VIDIOC_S_INPUT with invalid inputs", test_VIDIOC_S_INPUT_invalid_inputs },
  { "VIDIOC_G_INPUT with NULL parameter", test_VIDIOC_G_INPUT_NULL },
  { "VIDIOC_S_INPUT with NULL parameter", test_VIDIOC_S_INPUT_NULL },

  CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_invalid_ioctl[] = {
  { "invalid ioctl _IO(0, 0)", test_invalid_ioctl_1 },
  { "invalid ioctl _IO(0xFF, 0xFF)", test_invalid_ioctl_2 },
  { "invalid v4l1 ioctl _IO('v', 0xFF)", test_invalid_ioctl_3 },
  { "invalid v4l2 ioctl _IO('V', 0xFF)", test_invalid_ioctl_4 },

  CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_debug_ioctl[] = {
  { "test_VIDIOC_LOG_STATUS", test_VIDIOC_LOG_STATUS },

  CU_TEST_INFO_NULL,
};

static CU_SuiteInfo suites[] = {
  { "VIDIOC_QUERYCAP", open_video, close_video, suite_querycap },
  { "VIDIOC_ENUM* ioctl calls", open_video, close_video, suite_enums },
  { "VIDIOC_G_*, VIDIOC_S_* and VIDIOC_TRY_* ioctl calls", open_video, close_video, suite_get_set_try },
  { "debug ioctl calls", open_video, close_video, suite_debug_ioctl },
  { "invalid ioctl calls", open_video, close_video, suite_invalid_ioctl },
  CU_SUITE_INFO_NULL,
};

int main() {
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

	return 0;
}
