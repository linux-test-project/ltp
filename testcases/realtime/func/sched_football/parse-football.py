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
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##																			  ##
## NAME: parse-football.py                                                    ##
##                                                                            ##
## DESCRIPTION: Log Parser for the sched_football.c test                      ##
##                                                                            ##
## AUTHOR: Chirag <chirag@linux.vnet.ibm.com                                  ##
##                                                                            ##
################################################################################

from scripts.parser import *
import re
class SchedFootballLog(Log):
	def __init__(self,filename):
		Log.__init__(self,filename)

	def eval(self):
		exp1= "Final ball position"
		flag=False
		for line in self.read():
			if exp1 in line:
				list=line.split(" ")
				if(list[3]=="0"):
					return True
				else:
					return False


	
def main():
	if len(sys.argv) < 2:
		sys.exit("Usage : ./%s <logname>" % sys.argv[0])
	else:
		log_file=sys.argv[1]

	log = SchedFootballLog(log_file)
	sys.exit("Result: %s " % (["FAIL", "PASS"][log.eval()]))

if __name__ == "__main__":
    main()
