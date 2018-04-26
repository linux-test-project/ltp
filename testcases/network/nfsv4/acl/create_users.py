#!/usr/bin/env python3
'''
	Access Control Lists testing based on newpynfs framework
	Aurelien Charbon - Bull SA
'''

from random_gen import *
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-u", "--users", dest="nu",type="int",help="number of users to create")
parser.add_option("-g", "--group",dest="ng",type="int",help="number of groups to create")

(options, args) = parser.parse_args()

''' Measuring time to add an ACE to a list regarding the number of ACE already in the list'''
''' Doing the measurement on 100 files '''
test=RandomGen()
test.createNGroup(options.ng)
test.getGroupList()
test.createNUser(options.nu)
