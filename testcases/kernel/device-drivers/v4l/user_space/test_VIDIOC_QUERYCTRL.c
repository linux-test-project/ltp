/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 20 Apr 2009  0.4  Added string content validation
 * 18 Apr 2009  0.3  More strict check for strings
 * 28 Mar 2009  0.2  Clean up ret and errno variable names and dprintf() output
 *  2 Jan 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

/*
 * Note: V4L2_CID_LASTP1 != V4L2_CID_BASE_LASTP1
 */

#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include <linux/videodev2.h>
#include <linux/errno.h>

#include <CUnit/CUnit.h>

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"
#include "v4l2_validator.h"

#include "test_VIDIOC_QUERYCTRL.h"

static int valid_control_flag(__u32 flags)
{
	int valid = 0;

	if ((flags & ~(V4L2_CTRL_FLAG_DISABLED |
		       V4L2_CTRL_FLAG_GRABBED |
		       V4L2_CTRL_FLAG_READ_ONLY |
		       V4L2_CTRL_FLAG_UPDATE |
		       V4L2_CTRL_FLAG_INACTIVE | V4L2_CTRL_FLAG_SLIDER))
	    == 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

static int valid_control_type(__u32 type)
{
	int valid = 0;

	switch (type) {
	case V4L2_CTRL_TYPE_INTEGER:
	case V4L2_CTRL_TYPE_BOOLEAN:
	case V4L2_CTRL_TYPE_MENU:
	case V4L2_CTRL_TYPE_BUTTON:
	case V4L2_CTRL_TYPE_INTEGER64:
	case V4L2_CTRL_TYPE_CTRL_CLASS:
		valid = 1;
		break;
	default:
		valid = 0;
	}
	return valid;
}

void test_VIDIOC_QUERYCTRL()
{
	int ret_query, errno_query;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;
	__u32 i;

	/* The available controls and their parameters
	 * may change with different
	 *  - input or output
	 *  - tuner or modulator
	 *  - audio input or audio output
	 * See V4L API specification rev. 0.24, Chapter 1.8.
	 * "User Controls" for details
	 *
	 * TODO: iterate through the mentioned settings.
	 * TODO: check for deprecated controls (maybe in a
	 * separated test case which could fail when a
	 * deprecated control is supported)
	 */

	for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {

		memset(&queryctrl, 0xff, sizeof(queryctrl));
		queryctrl.id = i;
		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
		errno_query = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_QUERYCTRL, id=%u (V4L2_CID_BASE+%i), ret_query=%i, errno_query=%i\n",
		     __FILE__, __LINE__, i, i - V4L2_CID_BASE, ret_query,
		     errno_query);

		if (ret_query == 0) {
			CU_ASSERT_EQUAL(ret_query, 0);
			CU_ASSERT_EQUAL(queryctrl.id, i);

			CU_ASSERT(0 < strlen((char *)queryctrl.name));
			CU_ASSERT(valid_string
				  ((char *)queryctrl.name,
				   sizeof(queryctrl.name)));

			CU_ASSERT(valid_control_type(queryctrl.type));

			switch (queryctrl.type) {
			case V4L2_CTRL_TYPE_INTEGER:
				/* min < max, because otherwise this control makes no sense */
				CU_ASSERT(queryctrl.minimum <
					  queryctrl.maximum);

				CU_ASSERT(0 < queryctrl.step);

				CU_ASSERT(queryctrl.minimum <=
					  queryctrl.default_value);
				CU_ASSERT(queryctrl.default_value <=
					  queryctrl.maximum);
				break;

			case V4L2_CTRL_TYPE_BOOLEAN:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 1);
				CU_ASSERT_EQUAL(queryctrl.step, 1);
				CU_ASSERT((queryctrl.default_value == 0)
					  || (queryctrl.default_value == 1));
				break;

			case V4L2_CTRL_TYPE_MENU:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT(queryctrl.minimum <=
					  queryctrl.default_value);
				CU_ASSERT_EQUAL(queryctrl.step, 1);
				CU_ASSERT(queryctrl.minimum <=
					  queryctrl.default_value);
				CU_ASSERT(queryctrl.default_value <=
					  queryctrl.maximum);
				break;

			case V4L2_CTRL_TYPE_BUTTON:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 0);
				CU_ASSERT_EQUAL(queryctrl.step, 0);
				CU_ASSERT_EQUAL(queryctrl.default_value, 0);
				break;

			case V4L2_CTRL_TYPE_INTEGER64:	/* fallthrough */
			case V4L2_CTRL_TYPE_CTRL_CLASS:
				/* These parameters are defined as n/a by V4L2, so
				 * they should be filled with zeros, the same like
				 * the reserved fields.
				 */
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 0);
				CU_ASSERT_EQUAL(queryctrl.step, 0);
				CU_ASSERT_EQUAL(queryctrl.default_value, 0);
				break;

			default:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 0);
				CU_ASSERT_EQUAL(queryctrl.step, 0);
				CU_ASSERT_EQUAL(queryctrl.default_value, 0);
			}

			CU_ASSERT(valid_control_flag(queryctrl.flags));

			CU_ASSERT_EQUAL(queryctrl.reserved[0], 0);
			CU_ASSERT_EQUAL(queryctrl.reserved[1], 0);

			/* Check if the unused bytes of the name string are
			 * also filled with zeros. Also check if there is any
			 * padding byte between any two fields then this
			 * padding byte is also filled with zeros.
			 */
			memset(&queryctrl2, 0, sizeof(queryctrl2));
			queryctrl2.id = queryctrl.id;
			queryctrl2.type = queryctrl.type;
			strncpy((char *)queryctrl2.name, (char *)queryctrl.name,
				sizeof(queryctrl2.name));
			queryctrl2.minimum = queryctrl.minimum;
			queryctrl2.maximum = queryctrl.maximum;
			queryctrl2.step = queryctrl.step;
			queryctrl2.default_value = queryctrl.default_value;
			queryctrl2.flags = queryctrl.flags;
			CU_ASSERT_EQUAL(memcmp
					(&queryctrl, &queryctrl2,
					 sizeof(queryctrl)), 0);

			dprintf
			    ("\tqueryctrl = {.id=%u, .type=%i, .name=\"%s\", "
			     ".minimum=%i, .maximum=%i, .step=%i, "
			     ".default_value=%i, " ".flags=0x%X, "
			     ".reserved[]={ 0x%X, 0x%X } }\n", queryctrl.id,
			     queryctrl.type, queryctrl.name, queryctrl.minimum,
			     queryctrl.maximum, queryctrl.step,
			     queryctrl.default_value, queryctrl.flags,
			     queryctrl.reserved[0], queryctrl.reserved[1]
			    );

		} else {
			CU_ASSERT_EQUAL(ret_query, -1);
			CU_ASSERT_EQUAL(errno_query, EINVAL);

			memset(&queryctrl2, 0xff, sizeof(queryctrl2));
			queryctrl2.id = i;
			CU_ASSERT_EQUAL(memcmp
					(&queryctrl, &queryctrl2,
					 sizeof(queryctrl)), 0);

		}
	}

}

void test_VIDIOC_QUERYCTRL_BASE_1()
{
	int ret_query, errno_query;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;

	memset(&queryctrl, 0xff, sizeof(queryctrl));
	queryctrl.id = V4L2_CID_BASE - 1;
	ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
	errno_query = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_QUERYCTRL, id=%u (V4L2_CID_BASE-1), ret_query=%i, errno_query=%i\n",
	     __FILE__, __LINE__, V4L2_CID_BASE - 1, ret_query, errno_query);

	CU_ASSERT_EQUAL(ret_query, -1);
	CU_ASSERT_EQUAL(errno_query, EINVAL);

	memset(&queryctrl2, 0xff, sizeof(queryctrl2));
	queryctrl2.id = V4L2_CID_BASE - 1;
	CU_ASSERT_EQUAL(memcmp(&queryctrl, &queryctrl2, sizeof(queryctrl)), 0);

}

void test_VIDIOC_QUERYCTRL_LASTP1()
{
	int ret_query, errno_query;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;

	memset(&queryctrl, 0xff, sizeof(queryctrl));
	queryctrl.id = V4L2_CID_LASTP1;
	ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
	errno_query = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_QUERYCTRL, id=%u (V4L2_CID_LASTP1), ret_query=%i, errno_query=%i\n",
	     __FILE__, __LINE__, V4L2_CID_LASTP1, ret_query, errno_query);

	CU_ASSERT_EQUAL(ret_query, -1);
	CU_ASSERT_EQUAL(errno_query, EINVAL);

	memset(&queryctrl2, 0xff, sizeof(queryctrl2));
	queryctrl2.id = V4L2_CID_LASTP1;
	CU_ASSERT_EQUAL(memcmp(&queryctrl, &queryctrl2, sizeof(queryctrl)), 0);

}

void test_VIDIOC_QUERYCTRL_LASTP1_1()
{
	int ret_query, errno_query;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;

	memset(&queryctrl, 0xff, sizeof(queryctrl));
	queryctrl.id = V4L2_CID_LASTP1 + 1;
	ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
	errno_query = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_QUERYCTRL, id=%u (V4L2_CID_LASTP1+1), ret_query=%i, errno_query=%i\n",
	     __FILE__, __LINE__, V4L2_CID_LASTP1 + 1, ret_query, errno_query);

	CU_ASSERT_EQUAL(ret_query, -1);
	CU_ASSERT_EQUAL(errno_query, EINVAL);

	memset(&queryctrl2, 0xff, sizeof(queryctrl2));
	queryctrl2.id = V4L2_CID_LASTP1 + 1;
	CU_ASSERT_EQUAL(memcmp(&queryctrl, &queryctrl2, sizeof(queryctrl)), 0);

}

void test_VIDIOC_QUERYCTRL_flag_NEXT_CTRL()
{
	int ret_query, errno_query;
	char count_controls1[V4L2_CID_LASTP1 - V4L2_CID_BASE];
	char count_controls2[V4L2_CID_LASTP1 - V4L2_CID_BASE];
	struct v4l2_queryctrl controls[V4L2_CID_LASTP1 - V4L2_CID_BASE];
	struct v4l2_queryctrl queryctrl;
	__u32 i;

	/* find out all the possible user controls */
	memset(count_controls1, 0, sizeof(count_controls1));
	memset(controls, 0, sizeof(controls));

	for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {

		memset(&queryctrl, 0xff, sizeof(queryctrl));
		queryctrl.id = i;
		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
		errno_query = errno;

		if (ret_query == 0) {
			CU_ASSERT_EQUAL(ret_query, 0);
			CU_ASSERT_EQUAL(queryctrl.id, i);
			count_controls1[i - V4L2_CID_BASE]++;
			controls[i - V4L2_CID_BASE] = queryctrl;

			dprintf
			    ("\tqueryctrl = {.id=%u, .type=%i, .name=\"%s\", "
			     ".minimum=%i, .maximum=%i, .step=%i, "
			     ".default_value=%i, " ".flags=0x%X, "
			     ".reserved[]={ 0x%X, 0x%X } }\n", queryctrl.id,
			     queryctrl.type, queryctrl.name, queryctrl.minimum,
			     queryctrl.maximum, queryctrl.step,
			     queryctrl.default_value, queryctrl.flags,
			     queryctrl.reserved[0], queryctrl.reserved[1]
			    );

		} else {
			CU_ASSERT_EQUAL(ret_query, -1);
			CU_ASSERT_EQUAL(errno_query, EINVAL);
		}
	}

	/* enumerate the controls with V4L2_CTRL_FLAG_NEXT_CTRL */
	dprintf1("\tStarting enumeration with V4L2_CTRL_FLAG_NEXT_CTRL\n");
	memset(count_controls2, 0, sizeof(count_controls2));

	/* As described at V4L2 Chapter 1.9.3. Enumerating Extended Controls */
	i = 0;
	memset(&queryctrl, 0xff, sizeof(queryctrl));
	queryctrl.id = i | V4L2_CTRL_FLAG_NEXT_CTRL;
	dprintf
	    ("\tasking for id=%i=V4L2_CID_BASE+%i | V4L2_CTRL_FLAG_NEXT_CTRL\n",
	     i, i - V4L2_CID_BASE);
	ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
	errno_query = errno;

	dprintf("\tret_query=%i\n", ret_query);

	if (ret_query == 0) {
		do {
			/* protect the count_controls2[] from overindexing */
			if ((V4L2_CID_BASE <= queryctrl.id)
			    && (queryctrl.id < V4L2_CID_LASTP1)) {
				count_controls2[queryctrl.id - V4L2_CID_BASE]++;
				CU_ASSERT_EQUAL(memcmp
						(&queryctrl,
						 &controls[queryctrl.id -
							   V4L2_CID_BASE],
						 sizeof(queryctrl)), 0);
			}

			/* "The VIDIOC_QUERYCTRL ioctl will return the first
			 *  control with a higher ID than the specified one."
			 */
			CU_ASSERT(i < queryctrl.id);

			CU_ASSERT(V4L2_CID_BASE <= queryctrl.id);
			CU_ASSERT(queryctrl.id < V4L2_CID_LASTP1);

			dprintf
			    ("\tqueryctrl = {.id=%u, .type=%i, .name=\"%s\", "
			     ".minimum=%i, .maximum=%i, .step=%i, "
			     ".default_value=%i, " ".flags=0x%X, "
			     ".reserved[]={ 0x%X, 0x%X } }\n", queryctrl.id,
			     queryctrl.type, queryctrl.name, queryctrl.minimum,
			     queryctrl.maximum, queryctrl.step,
			     queryctrl.default_value, queryctrl.flags,
			     queryctrl.reserved[0], queryctrl.reserved[1]
			    );

			i = queryctrl.id;
			memset(&queryctrl, 0xff, sizeof(queryctrl));
			queryctrl.id = i | V4L2_CTRL_FLAG_NEXT_CTRL;
			dprintf
			    ("\tasking for id=%i=V4L2_CID_BASE+%i | V4L2_CTRL_FLAG_NEXT_CTRL\n",
			     i, i - V4L2_CID_BASE);
			ret_query =
			    ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
			errno_query = errno;

			dprintf("\tret_query=%i\n", ret_query);

		} while (ret_query == 0
			 && V4L2_CTRL_ID2CLASS(queryctrl.id) ==
			 V4L2_CTRL_CLASS_USER);

		if (ret_query == 0) {
			/* some other controls also exists, stop for now. */
		} else {
			CU_ASSERT_EQUAL(ret_query, -1);
			CU_ASSERT_EQUAL(errno_query, EINVAL);
		}

		/* Check whether the same controls are reported if using
		 * V4L2_CTRL_FLAG_NEXT_CTRL and without using it.
		 * This also checks if one control is not reported twice.
		 */
		CU_ASSERT_EQUAL(memcmp
				(count_controls1, count_controls2,
				 sizeof(count_controls1)), 0);

		dprintf1("count_controls1 = { ");
		for (i = 0;
		     i < sizeof(count_controls1) / sizeof(*count_controls1);
		     i++) {
			dprintf("%i ", count_controls1[i]);
		}
		dprintf1("}\n");

		dprintf1("count_controls2 = { ");
		for (i = 0;
		     i < sizeof(count_controls2) / sizeof(*count_controls2);
		     i++) {
			dprintf("%i ", count_controls2[i]);
		}
		dprintf1("}\n");

	} else {
		dprintf1
		    ("V4L2_CTRL_FLAG_NEXT_CTRL is not supported or no control is available\n");
		/* The flag V4L2_CTRL_FLAG_NEXT_CTRL is not supported
		 * or no control is avaliable at all. Do not continue the
		 * enumeration.
		 */
		CU_ASSERT_EQUAL(ret_query, -1);
		CU_ASSERT_EQUAL(errno_query, EINVAL);
	}

}

void test_VIDIOC_QUERYCTRL_private()
{
	int ret_query, errno_query;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;
	__u32 i;

	i = V4L2_CID_PRIVATE_BASE;
	do {
		memset(&queryctrl, 0xff, sizeof(queryctrl));
		queryctrl.id = i;
		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
		errno_query = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_QUERYCTRL, id=%u (V4L2_CID_BASE+%i), ret_query=%i, errno_query=%i\n",
		     __FILE__, __LINE__, i, i - V4L2_CID_BASE, ret_query,
		     errno_query);

		if (ret_query == 0) {
			CU_ASSERT_EQUAL(ret_query, 0);
			CU_ASSERT_EQUAL(queryctrl.id, i);

			CU_ASSERT(0 < strlen((char *)queryctrl.name));
			CU_ASSERT(valid_string
				  ((char *)queryctrl.name,
				   sizeof(queryctrl.name)));

			CU_ASSERT(valid_control_type(queryctrl.type));

			switch (queryctrl.type) {
			case V4L2_CTRL_TYPE_INTEGER:
				/* min < max, because otherwise this control makes no sense */
				CU_ASSERT(queryctrl.minimum <
					  queryctrl.maximum);

				CU_ASSERT(0 < queryctrl.step);

				CU_ASSERT(queryctrl.minimum <=
					  queryctrl.default_value);
				CU_ASSERT(queryctrl.default_value <=
					  queryctrl.maximum);
				break;

			case V4L2_CTRL_TYPE_BOOLEAN:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 1);
				CU_ASSERT_EQUAL(queryctrl.step, 1);
				CU_ASSERT((queryctrl.default_value == 0)
					  || (queryctrl.default_value == 1));
				break;

			case V4L2_CTRL_TYPE_MENU:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT(queryctrl.minimum <=
					  queryctrl.default_value);
				CU_ASSERT_EQUAL(queryctrl.step, 1);
				CU_ASSERT(queryctrl.minimum <=
					  queryctrl.default_value);
				CU_ASSERT(queryctrl.default_value <=
					  queryctrl.maximum);
				break;

			case V4L2_CTRL_TYPE_BUTTON:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 0);
				CU_ASSERT_EQUAL(queryctrl.step, 0);
				CU_ASSERT_EQUAL(queryctrl.default_value, 0);
				break;

			case V4L2_CTRL_TYPE_INTEGER64:	/* fallthrough */
			case V4L2_CTRL_TYPE_CTRL_CLASS:
				/* These parameters are defined as n/a by V4L2, so
				 * they should be filled with zeros, the same like
				 * the reserved fields.
				 */
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 0);
				CU_ASSERT_EQUAL(queryctrl.step, 0);
				CU_ASSERT_EQUAL(queryctrl.default_value, 0);
				break;

			default:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 0);
				CU_ASSERT_EQUAL(queryctrl.step, 0);
				CU_ASSERT_EQUAL(queryctrl.default_value, 0);
			}

			CU_ASSERT(valid_control_flag(queryctrl.flags));

			CU_ASSERT_EQUAL(queryctrl.reserved[0], 0);
			CU_ASSERT_EQUAL(queryctrl.reserved[1], 0);

			/* Check if the unused bytes of the name string are
			 * also filled with zeros. Also check if there is any
			 * padding byte between any two fields then this
			 * padding byte is also filled with zeros.
			 */
			memset(&queryctrl2, 0, sizeof(queryctrl2));
			queryctrl2.id = queryctrl.id;
			queryctrl2.type = queryctrl.type;
			strncpy((char *)queryctrl2.name, (char *)queryctrl.name,
				sizeof(queryctrl2.name));
			queryctrl2.minimum = queryctrl.minimum;
			queryctrl2.maximum = queryctrl.maximum;
			queryctrl2.step = queryctrl.step;
			queryctrl2.default_value = queryctrl.default_value;
			queryctrl2.flags = queryctrl.flags;
			CU_ASSERT_EQUAL(memcmp
					(&queryctrl, &queryctrl2,
					 sizeof(queryctrl)), 0);

			dprintf
			    ("\tqueryctrl = {.id=%u, .type=%i, .name=\"%s\", "
			     ".minimum=%i, .maximum=%i, .step=%i, "
			     ".default_value=%i, " ".flags=0x%X, "
			     ".reserved[]={ 0x%X, 0x%X } }\n", queryctrl.id,
			     queryctrl.type, queryctrl.name, queryctrl.minimum,
			     queryctrl.maximum, queryctrl.step,
			     queryctrl.default_value, queryctrl.flags,
			     queryctrl.reserved[0], queryctrl.reserved[1]
			    );

		} else {
			CU_ASSERT_EQUAL(ret_query, -1);
			CU_ASSERT_EQUAL(errno_query, EINVAL);

			memset(&queryctrl2, 0xff, sizeof(queryctrl2));
			queryctrl2.id = i;
			CU_ASSERT_EQUAL(memcmp
					(&queryctrl, &queryctrl2,
					 sizeof(queryctrl)), 0);

		}
	} while (ret_query == 0);

}

void test_VIDIOC_QUERYCTRL_private_base_1()
{
	int ret_query, errno_query;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;

	memset(&queryctrl, 0xff, sizeof(queryctrl));
	queryctrl.id = V4L2_CID_PRIVATE_BASE - 1;
	ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
	errno_query = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_QUERYCTRL, id=%u (V4L2_CID_PRIVATE_BASE-1), ret_query=%i, errno_query=%i\n",
	     __FILE__, __LINE__, V4L2_CID_PRIVATE_BASE - 1, ret_query,
	     errno_query);

	CU_ASSERT_EQUAL(ret_query, -1);
	CU_ASSERT_EQUAL(errno_query, EINVAL);

	memset(&queryctrl2, 0xff, sizeof(queryctrl2));
	queryctrl2.id = V4L2_CID_PRIVATE_BASE - 1;
	CU_ASSERT_EQUAL(memcmp(&queryctrl, &queryctrl2, sizeof(queryctrl)), 0);

}

void test_VIDIOC_QUERYCTRL_private_last_1()
{
	int ret_query, errno_query;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;
	__u32 i;

	i = V4L2_CID_PRIVATE_BASE;
	do {
		memset(&queryctrl, 0xff, sizeof(queryctrl));
		queryctrl.id = i;
		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
		errno_query = errno;

		i++;
	} while (ret_query == 0);

	memset(&queryctrl, 0xff, sizeof(queryctrl));
	queryctrl.id = i;
	ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);

	dprintf
	    ("\t%s:%u: VIDIOC_QUERYCTRL, id=%u (V4L2_CID_PRIVATE_BASE+%u), ret_query=%i, errno_query=%i\n",
	     __FILE__, __LINE__, i, i - V4L2_CID_PRIVATE_BASE, ret_query,
	     errno_query);

	CU_ASSERT_EQUAL(ret_query, -1);
	CU_ASSERT_EQUAL(errno_query, EINVAL);

	memset(&queryctrl2, 0xff, sizeof(queryctrl2));
	queryctrl2.id = i;
	CU_ASSERT_EQUAL(memcmp(&queryctrl, &queryctrl2, sizeof(queryctrl)), 0);

}

void test_VIDIOC_QUERYCTRL_NULL()
{
	int ret_query, errno_query;
	int ret_null, errno_null;
	struct v4l2_queryctrl queryctrl;
	__u32 i;
	unsigned int count_ctrl;

	count_ctrl = 0;

	i = V4L2_CID_BASE;
	for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {
		memset(&queryctrl, 0xff, sizeof(queryctrl));
		queryctrl.id = i;
		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
		errno_query = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_QUERYCTRL, id=%u (V4L2_CID_BASE+%i), ret_query=%i, errno_query=%i\n",
		     __FILE__, __LINE__, i, i - V4L2_CID_BASE, ret_query,
		     errno_query);

		if (ret_query == 0) {
			CU_ASSERT_EQUAL(ret_query, 0);
			count_ctrl++;
		} else {
			CU_ASSERT_EQUAL(ret_query, -1);
			CU_ASSERT_EQUAL(errno_query, EINVAL);
		}
	}

	i = V4L2_CID_PRIVATE_BASE;
	do {
		memset(&queryctrl, 0xff, sizeof(queryctrl));
		queryctrl.id = i;
		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
		errno_query = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_QUERYCTRL, id=%u (V4L2_CID_PRIVATE_BASE+%i), ret_query=%i, errno_query=%i\n",
		     __FILE__, __LINE__, i, i - V4L2_CID_PRIVATE_BASE,
		     ret_query, errno_query);

		if (ret_query == 0) {
			CU_ASSERT_EQUAL(ret_query, 0);
			count_ctrl++;
		} else {
			CU_ASSERT_EQUAL(ret_query, -1);
			CU_ASSERT_EQUAL(errno_query, EINVAL);
		}

		i++;
	} while (ret_query == 0 && V4L2_CID_PRIVATE_BASE <= i);

	ret_null = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_QUERYCTRL, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (0 < count_ctrl) {
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}
