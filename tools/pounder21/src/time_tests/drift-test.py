#!/usr/bin/python

# Time Drift Script
#		Periodically checks and displays time drift
#		by john stultz (jstultz@us.ibm.com)

# Copyright (C) 2003-2006 IBM
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.


# Usage: drift-test.py [-s] [ntp_server [sleep_time]]

import commands
import sys
import string
import time

server_default = "bvrgsa.ibm.com"
sleep_time_default  = 60

server = ""
sleep_time = 0
set_time = 0

#parse args
for arg in sys.argv[1:]:
	if arg == "-s":
		set_time = 1
	elif server == "":
		server = arg
	elif sleep_time == 0:
		sleep_time = string.atoi(arg)

if server == "":
	server = server_default
if sleep_time == 0:
	sleep_time = sleep_time_default

#set time
if (set_time == 1):
	cmd = commands.getoutput('/usr/sbin/ntpdate -ub ' + server)

cmd = commands.getoutput('/usr/sbin/ntpdate -uq ' + server)
line = string.split(cmd)

#parse original offset
start_offset = string.atof(line[-2]);
#parse original time
start_time = time.localtime(time.time())
datestr = time.strftime("%d %b %Y %H:%M:%S", start_time)

time.sleep(1)
while 1:
	cmd = commands.getoutput('/usr/sbin/ntpdate -uq ' + server)
	line = string.split(cmd)

	#parse offset
	now_offset = string.atof(line[-2]);

	#parse time
	now_time = time.localtime(time.time())
	datestr = time.strftime("%d %b %Y %H:%M:%S", now_time)

	# calculate drift
	delta_time = time.mktime(now_time) - time.mktime(start_time)
	delta_offset = now_offset - start_offset
	drift =  delta_offset / delta_time * 1000000

	#print output
	print time.strftime("%d %b %H:%M:%S",now_time),
	print "	offset:", now_offset ,
	print "	drift:", drift ,"ppm"
	sys.stdout.flush()

	#sleep
	time.sleep(sleep_time)
