// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Jinhui huang <huangjh.jy@cn.fujitsu.com>
 */

#ifndef FCHMOD_H
#define FCHMOD_H

#define FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define DIR_MODE	(S_IRWXU | S_IRWXG | S_IRWXO)
#define PERMS	01777
#define TESTFILE	"testfile"
#define TESTDIR	"testdir"

#endif /* FCHMOD_H */
