// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*
 * Tests for tst_parse_filesize.
 */

#include "tst_test.h"

static void do_test(void)
{
    long long val = 0;
    int ret = 0;

    if ((ret = tst_parse_filesize("1", &val, LLONG_MIN, LLONG_MAX))) {
        tst_res(TFAIL, "return code %d (%s)", ret, tst_strerrno(ret));
    } else {
        if (val == 1LL)
            tst_res(TPASS, "value is %lli", val);
        else
            tst_res(TFAIL, "%lli != %lli", val, 1LL);
    }

    /* small letters */
    if ((ret = tst_parse_filesize("1k", &val, LLONG_MIN, LLONG_MAX))) {
        tst_res(TFAIL, "return code %d (%s)", ret, tst_strerrno(ret));
    } else {
        if (val == 1024LL)
            tst_res(TPASS, "value is %lli", val);
        else
            tst_res(TFAIL, "%lli != %lli", val, 1024LL);
    }

    if ((ret = tst_parse_filesize("1m", &val, LLONG_MIN, LLONG_MAX))) {
        tst_res(TFAIL, "return code %d (%s)", ret, tst_strerrno(ret));
    } else {
        if (val == 1048576LL)
            tst_res(TPASS, "value is %lli", val);
        else
            tst_res(TFAIL, "%lli != %lli", val, 1048576LL);
    }

    if ((ret = tst_parse_filesize("1g", &val, LLONG_MIN, LLONG_MAX))) {
        tst_res(TFAIL, "return code %d (%s)", ret, tst_strerrno(ret));
    } else {
        if (val == 1073741824LL)
            tst_res(TPASS, "value is %lli", val);
        else
            tst_res(TFAIL, "%lli != %lli", val, 1073741824LL);
    }

    /* big letters */
    if ((ret = tst_parse_filesize("1K", &val, LLONG_MIN, LLONG_MAX))) {
        tst_res(TFAIL, "return code %d (%s)", ret, tst_strerrno(ret));
    } else {
        if (val == 1024LL)
            tst_res(TPASS, "value is %lli", val);
        else
            tst_res(TFAIL, "%lli != %lli", val, 1024LL);
    }

    if ((ret = tst_parse_filesize("1M", &val, LLONG_MIN, LLONG_MAX))) {
        tst_res(TFAIL, "return code %d (%s)", ret, tst_strerrno(ret));
    } else {
        if (val == 1048576LL)
            tst_res(TPASS, "value is %lli", val);
        else
            tst_res(TFAIL, "%lli != %lli", val, 1048576LL);
    }

    if ((ret = tst_parse_filesize("1G", &val, LLONG_MIN, LLONG_MAX))) {
        tst_res(TFAIL, "return code %d (%s)", ret, tst_strerrno(ret));
    } else {
        if (val == 1073741824LL)
            tst_res(TPASS, "value is %lli", val);
        else
            tst_res(TFAIL, "%lli != %lli", val, 1073741824LL);
    }

    /* test errors */
    if ((ret = tst_parse_filesize("k", &val, LLONG_MIN, LLONG_MAX))) {
        if (ret == EINVAL)
            tst_res(TPASS, "return code %d (%s)", ret, tst_strerrno(ret));
        else
            tst_res(TFAIL, "return code %d (%s)", ret, tst_strerrno(ret));
    } else {
        tst_res(TFAIL, "tst_parse_filesize should have failed");
    }

    if ((ret = tst_parse_filesize("100", &val, LLONG_MIN, 10))) {
        if (ret == ERANGE)
            tst_res(TPASS, "return code %d (%s)", ret, tst_strerrno(ret));
        else
            tst_res(TFAIL, "return code %d (%s)", ret, tst_strerrno(ret));
    } else {
        tst_res(TFAIL, "tst_parse_filesize should have failed");
    }

    if ((ret = tst_parse_filesize("10", &val, 100, LLONG_MAX))) {
        if (ret == ERANGE)
            tst_res(TPASS, "return code %d (%s)", ret, tst_strerrno(ret));
        else
            tst_res(TFAIL, "return code %d (%s)", ret, tst_strerrno(ret));
    } else {
        tst_res(TFAIL, "tst_parse_filesize should have failed");
    }

    if ((ret = tst_parse_filesize("10garbage", &val, LLONG_MIN, LLONG_MAX))) {
        if (ret == EINVAL)
            tst_res(TPASS, "return code %d (%s)", ret, tst_strerrno(ret));
        else
            tst_res(TFAIL, "return code %d (%s)", ret, tst_strerrno(ret));
    } else {
        tst_res(TFAIL, "tst_parse_filesize should have failed");
    }
}

static struct tst_test test = {
	.test_all = do_test,
};
