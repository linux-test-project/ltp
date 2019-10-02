#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2017
# Written by Petr Vorel <pvorel@suse.cz>

tst_flag2color()
{
	# NOTE: these colors should match colors defined in include/tst_ansi_color.h
	local ansi_color_blue='\033[1;34m'
	local ansi_color_green='\033[1;32m'
	local ansi_color_magenta='\033[1;35m'
	local ansi_color_red='\033[1;31m'
	local ansi_color_yellow='\033[1;33m'

	case "$1" in
	TPASS) printf $ansi_color_green;;
	TFAIL) printf $ansi_color_red;;
	TBROK) printf $ansi_color_red;;
	TWARN) printf $ansi_color_magenta;;
	TINFO) printf $ansi_color_blue;;
	TCONF) printf $ansi_color_yellow;;
	esac
}

tst_color_enabled()
{
	[ "$LTP_COLORIZE_OUTPUT" = "n" ] || [ "$LTP_COLORIZE_OUTPUT" = "0" ] && return 0
	[ "$LTP_COLORIZE_OUTPUT" = "y" ] || [ "$LTP_COLORIZE_OUTPUT" = "1" ] && return 1
	[ -t 1 ] || return 0
	return 1
}

tst_print_colored()
{
	tst_color_enabled
	local color=$?

	[ "$color" = "1" ] && tst_flag2color "$1"
	printf "$2"
	[ "$color" = "1" ] && printf '\033[0m'
}
