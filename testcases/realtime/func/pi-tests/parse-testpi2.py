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
## NAME: parse-testpi2.py                                                     ##
##                                                                            ##
## DESCRIPTION: Log Parser for the testpi-2.c test                            ##
##                                                                            ##
## AUTHOR: Chirag <chirag@linux.vnet.ibm.com                                  ##
##                                                                            ##
################################################################################


from scripts.parser import *
import re
class TestPi2(Log):
	def __init__(self,filename):
		Log.__init__(self,filename)

	def eval(self):
		exp1= re.compile("pthread pol 2 pri 10")
		exp2= re.compile(r'^Noise Thread')
		exp3=re.compile("[1-9]\d{2,3}")
		prev_line="temp"
		count=0
		flag=True
		for line in self.read():
			if exp1.search(line) and exp2.search(prev_line) and exp3.search(prev_line):
				list=prev_line.split(" ")
				if int(list[4])<= 9900:
					count+=1
					flag=True
				elif count == 0:
					return False



		prev_line=line
		if count>=2:
			return True
		else:
			return False

def main():
	if len(sys.argv) < 2:
		sys.exit("Usage : ./%s <logname>" % sys.argv[0])
	else:
		log_file=sys.argv[1]
	log = TestPi2(log_file)
	sys.exit("Result: %s " % (["FAIL", "PASS"][log.eval()]))

if __name__ == "__main__":
	main()
