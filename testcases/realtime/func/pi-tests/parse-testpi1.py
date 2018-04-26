#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
##                                                                            ##
## Copyright ©  International Business Machines  Corp., 2007, 2008            ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##									      ##
## NAME: parse-testpi1.py                                                     ##
##                                                                            ##
## DESCRIPTION: Log Parser for the testpi-1.c test                            ##
##                                                                            ##
## AUTHOR: Chirag <chirag@linux.vnet.ibm.com                                  ##
##                                                                            ##
################################################################################

from scripts.parser import *
import re
class TestPi1(Log):
	def __init__(self,filename):
		Log.__init__(self,filename)

	def eval(self):
		exp1= re.compile("pthread pol 0 pri 0")
		exp2= re.compile(r'^Noise Thread')
		exp3=re.compile("[1-9]\d{2,3}")
		flag=False
		for line in self.read():
			if exp1.search(line) and  exp2.search(prev_line)and exp3.search(prev_line):
				list=prev_line.split(" ")
				if int(list[4])< 9900:
					flag=True
				else:
					flag=False
			prev_line=line
		return flag

def main():
	if len(sys.argv) < 2:
		sys.exit("Usage : ./%s <logname>" % sys.argv[0])
	else:
		log_file = sys.argv[1]
	log = TestPi1(log_file)
	sys.exit("Result: %s " % (["FAIL", "PASS"][log.eval()]))

if __name__ == "__main__":
    main()
