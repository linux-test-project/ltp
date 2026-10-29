// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2026
 */

#ifndef _RANDOM_RANGE_H_
#define _RANDOM_RANGE_H_

int       parse_ranges     ( char *, int, int, int, int (*)(), char **, char ** );
int       range_min        ( char *, int );
int       range_max        ( char *, int );
int       range_mult       ( char *, int );
long      random_range     ( int, int, int, char ** );
long      random_rangel    ( long, long, long, char ** );
long long random_rangell   ( long long, long long, long long, char ** );
void      random_range_seed( long );
long      random_bit       ( long );

#endif
