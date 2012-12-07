/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 20 Apr 2009  0.3  Added string content validation
 * 18 Apr 2009  0.2  More strict check for strings
 *  5 Apr 2009  0.1  First release
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

static void do_check_menu(__u32 id, __u32 index,
			  int ret_query, int errno_query,
			  struct v4l2_querymenu *menu)
{
	struct v4l2_querymenu menu2;

	if (ret_query == 0) {
		CU_ASSERT_EQUAL(ret_query, 0);

		dprintf("\tmenu = {.id=%u, .index=%i, .name=\"%s\", "
			".reserved=0x%X }\n",
			menu->id, menu->index, menu->name, menu->reserved);

		CU_ASSERT_EQUAL(menu->id, id);
		CU_ASSERT_EQUAL(menu->index, index);

		CU_ASSERT(0 < strlen((char *)menu->name));
		CU_ASSERT(valid_string((char *)menu->name, sizeof(menu->name)));

		CU_ASSERT_EQUAL(menu->reserved, 0);

		/* Check if the unused bytes of the name string is also filled
		 * with zeros. Also check if there is any padding byte between
		 * any two fields then this padding byte is also filled with
		 * zeros.
		 */
		memset(&menu2, 0, sizeof(menu2));
		menu2.id = id;
		menu2.index = index;
		strncpy((char *)menu2.name, (char *)menu->name,
			sizeof(menu2.name));
		CU_ASSERT_EQUAL(memcmp(menu, &menu2, sizeof(*menu)), 0);

	} else {
		CU_ASSERT_EQUAL(ret_query, -1);
		CU_ASSERT_EQUAL(errno_query, EINVAL);

		memset(&menu2, 0xff, sizeof(menu2));
		menu2.id = id;
		menu2.index = index;
		CU_ASSERT(memcmp(&menu, &menu2, sizeof(menu)));
	}
}

static void do_query_menu(__u32 id)
{
	int ret_query, errno_query;
	__u32 i;
	struct v4l2_querymenu menu;

	i = 0;
	do {
		memset(&menu, 0xff, sizeof(menu));
		menu.id = id;
		menu.index = i;

		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYMENU, &menu);
		errno_query = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_QUERYMENU, id=%u, (V4L2_CID_BASE+%i), index=%u, ret_query=%i, errno_query=%i\n",
		     __FILE__, __LINE__, id, id - V4L2_CID_BASE, i, ret_query,
		     errno_query);

		do_check_menu(id, i, ret_query, errno_query, &menu);

		i++;
	} while (ret_query == 0);

}

static void do_query_menu_invalid(__u32 id)
{
	int ret_query, errno_query;
	unsigned int i;
	struct v4l2_querymenu menu;
	const __u32 test_index[] = {
		U32_MIN,
		U32_MIN + 1,
		(__u32) S32_MIN,
		(__u32) S32_MAX,
		U32_MAX - 1,
		U32_MAX
	};

	for (i = 0; i < sizeof(test_index) / sizeof(*test_index); i++) {
		memset(&menu, 0xff, sizeof(menu));
		menu.id = id;
		menu.index = test_index[i];

		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYMENU, &menu);
		errno_query = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_QUERYMENU, id=%u, (V4L2_CID_BASE+%i), index=%u, ret_query=%i, errno_query=%i\n",
		     __FILE__, __LINE__, id, id - V4L2_CID_BASE, test_index[i],
		     ret_query, errno_query);

		CU_ASSERT_EQUAL(ret_query, -1);
		CU_ASSERT_EQUAL(errno_query, EINVAL);

		do_check_menu(id, test_index[i], ret_query, errno_query, &menu);

	}

}

void test_VIDIOC_QUERYMENU()
{
	int ret_query, errno_query;
	struct v4l2_queryctrl queryctrl;
	__u32 i;

	for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {

		memset(&queryctrl, 0, sizeof(queryctrl));
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

			switch (queryctrl.type) {
			case V4L2_CTRL_TYPE_MENU:
				do_query_menu(i);
				break;

			case V4L2_CTRL_TYPE_INTEGER:
			case V4L2_CTRL_TYPE_BOOLEAN:
			case V4L2_CTRL_TYPE_BUTTON:
			case V4L2_CTRL_TYPE_INTEGER64:
			case V4L2_CTRL_TYPE_CTRL_CLASS:
			default:
				do_query_menu_invalid(i);
			}

		} else {
			CU_ASSERT_EQUAL(ret_query, -1);
			CU_ASSERT_EQUAL(errno_query, EINVAL);

			do_query_menu_invalid(i);

		}
	}

}

void test_VIDIOC_QUERYMENU_invalid()
{
	do_query_menu_invalid(0);
	do_query_menu_invalid(V4L2_CID_BASE - 1);
	do_query_menu_invalid(V4L2_CID_LASTP1);
	do_query_menu_invalid(V4L2_CID_LASTP1 + 1);
	do_query_menu_invalid(V4L2_CID_PRIVATE_BASE - 1);
}

void test_VIDIOC_QUERYMENU_private()
{
	int ret_query, errno_query;
	struct v4l2_queryctrl queryctrl;
	__u32 i;

	i = V4L2_CID_PRIVATE_BASE;
	do {
		memset(&queryctrl, 0, sizeof(queryctrl));
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

			switch (queryctrl.type) {
			case V4L2_CTRL_TYPE_MENU:
				do_query_menu(i);
				break;

			case V4L2_CTRL_TYPE_INTEGER:
			case V4L2_CTRL_TYPE_BOOLEAN:
			case V4L2_CTRL_TYPE_BUTTON:
			case V4L2_CTRL_TYPE_INTEGER64:	/* fallthrough */
			case V4L2_CTRL_TYPE_CTRL_CLASS:
			default:
				do_query_menu_invalid(i);

			}

		} else {
			CU_ASSERT_EQUAL(ret_query, -1);
			CU_ASSERT_EQUAL(errno_query, EINVAL);

			do_query_menu_invalid(i);

		}
	} while (ret_query == 0);

}

void test_VIDIOC_QUERYMENU_private_last_1()
{
	int ret_query, errno_query;
	struct v4l2_queryctrl queryctrl;
	__u32 i;

	i = V4L2_CID_PRIVATE_BASE;
	do {
		memset(&queryctrl, 0xff, sizeof(queryctrl));
		queryctrl.id = i;
		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
		errno_query = errno;

		i++;
	} while (ret_query == 0);

	do_query_menu_invalid(i);
}

void test_VIDIOC_QUERYMENU_NULL()
{
	int ret_query, errno_query;
	int ret_menu, errno_menu;
	int ret_null, errno_null;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_querymenu menu;
	__u32 i;
	unsigned int count_menu;

	count_menu = 0;

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
			if (queryctrl.type == V4L2_CTRL_TYPE_MENU) {

				memset(&menu, 0, sizeof(menu));
				menu.id = i;
				menu.index = 0;

				ret_menu =
				    ioctl(get_video_fd(), VIDIOC_QUERYMENU,
					  &menu);
				errno_menu = errno;

				dprintf
				    ("\t%s:%u: VIDIOC_QUERYMENU, id=%u, (V4L2_CID_BASE+%i), index=%u, ret_query=%i, errno_query=%i\n",
				     __FILE__, __LINE__, i, i - V4L2_CID_BASE,
				     0, ret_query, errno_query);

				if (ret_menu == 0) {
					CU_ASSERT_EQUAL(ret_menu, 0);
					count_menu++;
				} else {
					CU_ASSERT_EQUAL(ret_menu, -1);
					CU_ASSERT_EQUAL(errno_menu, EINVAL);
				}
			}
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
			if (queryctrl.type == V4L2_CTRL_TYPE_MENU) {

				memset(&menu, 0, sizeof(menu));
				menu.id = i;
				menu.index = 0;

				ret_menu =
				    ioctl(get_video_fd(), VIDIOC_QUERYMENU,
					  &menu);
				errno_menu = errno;

				dprintf
				    ("\t%s:%u: VIDIOC_QUERYMENU, id=%u, (V4L2_CID_BASE+%i), index=%u, ret_query=%i, errno_query=%i\n",
				     __FILE__, __LINE__, i, i - V4L2_CID_BASE,
				     0, ret_query, errno_query);

				if (ret_menu == 0) {
					CU_ASSERT_EQUAL(ret_menu, 0);
					count_menu++;
				} else {
					CU_ASSERT_EQUAL(ret_menu, -1);
					CU_ASSERT_EQUAL(errno_menu, EINVAL);
				}
			}
		} else {
			CU_ASSERT_EQUAL(ret_query, -1);
			CU_ASSERT_EQUAL(errno_query, EINVAL);
		}

		i++;
	} while (ret_query == 0 && V4L2_CID_PRIVATE_BASE <= i);

	ret_null = ioctl(get_video_fd(), VIDIOC_QUERYMENU, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_QUERYMENU, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (0 < count_menu) {
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}
