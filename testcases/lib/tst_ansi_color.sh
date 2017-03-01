#!/bin/sh
#
# Copyright (c) Linux Test Project, 2017
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Written by  Petr Vorel <pvorel@suse.cz>

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
