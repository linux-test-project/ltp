#!/bin/sh
#
# Copyright (C) 2009, Cisco Systems Inc.
#  Ngie Cooper, August 2009
# Copyright (C) 2012-2014 Linux Test Project
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

# running under systemd?
if command -v systemctl >/dev/null 2>&1; then
	HAVE_SYSTEMCTL=1
else
	HAVE_SYSTEMCTL=0
fi

# Check to see if syslogd, syslog-ng or rsyslogd exists
SYSLOG_DAEMON=""
if command -v syslogd >/dev/null 2>&1; then
	SYSLOG_DAEMON="syslog"
elif command -v syslog-ng >/dev/null 2>&1; then
	SYSLOG_DAEMON="syslog-ng"
elif command -v rsyslogd >/dev/null 2>&1; then
	SYSLOG_DAEMON="rsyslog"
fi

# Check to see if cron or crond exists
CROND_DAEMON=""
if command -v crond >/dev/null 2>&1; then
	CROND_DAEMON="crond"
elif command -v cron >/dev/null 2>&1; then
	CROND_DAEMON="cron"
fi

start_daemon()
{
	if [ $HAVE_SYSTEMCTL -eq 1 ]; then
		systemctl start $1.service > /dev/null 2>&1
	elif command -v service >/dev/null 2>&1; then
		service $1 start > /dev/null 2>&1
	else
		/etc/init.d/$1 start > /dev/null 2>&1
	fi
}

stop_daemon()
{
	if [ $HAVE_SYSTEMCTL -eq 1 ]; then
		systemctl stop $1.service > /dev/null 2>&1
	elif command -v service >/dev/null 2>&1; then
		service $1 stop > /dev/null 2>&1
	else
		/etc/init.d/$1 stop > /dev/null 2>&1
	fi
}

status_daemon()
{
	if [ $HAVE_SYSTEMCTL -eq 1 ]; then
		systemctl is-active $1.service > /dev/null 2>&1
	elif command -v service >/dev/null 2>&1; then
		service $1 status > /dev/null 2>&1
	else
		/etc/init.d/$1 status > /dev/null 2>&1
	fi
}

restart_daemon()
{
	if [ $HAVE_SYSTEMCTL -eq 1 ]; then
		systemctl restart $1.service > /dev/null 2>&1
	elif command -v service >/dev/null 2>&1; then
		service $1 restart > /dev/null 2>&1
	else
		/etc/init.d/$1 restart > /dev/null 2>&1
	fi
}
