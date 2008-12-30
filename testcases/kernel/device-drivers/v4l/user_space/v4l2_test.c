
/* v4l-test: Test environment for Video For Linux Two API
 *
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
#include "test_VIDIOC_CROPCAP.h"
#include "test_VIDIOC_ENUMSTD.h"
#include "test_VIDIOC_ENUM_FMT.h"
#include "test_VIDIOC_ENUMINPUT.h"
#include "test_VIDIOC_ENUMAUDIO.h"
#include "test_VIDIOC_STD.h"
#include "test_VIDIOC_INPUT.h"
#include "test_invalid_ioctl.h"


static CU_TestInfo suite_querycap[] = {
  { "VIDIOC_QUERYCAP", test_VIDIOC_QUERYCAP },

  { "VIDIOC_CROPCAP", test_VIDIOC_CROPCAP },
  { "VIDIOC_CROPCAP with different inputs", test_VIDIOC_CROPCAP_enum_INPUT },
  { "VIDIOC_CROPCAP with NULL parameter", test_VIDIOC_CROPCAP_NULL },

  CU_TEST_INFO_NULL,
};

static CU_TestInfo suite_enums[] = {
  { "VIDIOC_ENUMSTD", test_VIDIOC_ENUMSTD_1 },
  { "VIDIOC_ENUMSTD, index=U32_MAX", test_VIDIOC_ENUMSTD_2 },
  { "VIDIOC_ENUMSTD, index=MAX_EM28XX_TVNORMS", test_VIDIOC_ENUMSTD_3 },
  { "VIDIOC_ENUMSTD with NULL parameter", test_VIDIOC_ENUMSTD_NULL },

  { "VIDIOC_ENUMINPUT", test_VIDIOC_ENUMINPUT_1 },
  { "VIDIOC_ENUMINPUT, index=U32_MAX", test_VIDIOC_ENUMINPUT_2 },
  { "VIDIOC_ENUMINPUT, index=MAX_EM28XX_INPUT", test_VIDIOC_ENUMINPUT_3 },
  { "VIDIOC_ENUMINPUT with NULL parameter", test_VIDIOC_ENUMINPUT_NULL },

  { "VIDIOC_ENUM_FMT", test_VIDIOC_ENUM_FMT_1 },
  { "VIDIOC_ENUM_FMT, index=U32_MAX", test_VIDIOC_ENUM_FMT_2 },
  { "VIDIOC_ENUM_FMT, invalid type", test_VIDIOC_ENUM_FMT_3 },
  { "VIDIOC_ENUM_FMT with NULL parameter", test_VIDIOC_ENUM_FMT_NULL },

  { "VIDIOC_ENUMAUDIO", test_VIDIOC_ENUMAUDIO },
  { "VIDIOC_ENUMAUDIO with NULL parameter", test_VIDIOC_ENUMAUDIO_NULL },

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


static CU_SuiteInfo suites[] = {
  { "VIDIOC_QUERYCAP", open_video, close_video, suite_querycap },
  { "VIDIOC_ENUM* ioctl calls", open_video, close_video, suite_enums },
  { "VIDIOC_G_*, VIDIOC_S_* and VIDIOC_TRY_* ioctl calls", open_video, close_video, suite_get_set_try },
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
