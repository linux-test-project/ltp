'''
	Access Control Lists testing based on newpynfs framework
	Aurelien Charbon - Bull SA
'''
from random_gen import *
from optparse import OptionParser
import commands
import os
import threading
import time
import random

alphabet='abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789_-() ~'
t_alphabet=len(alphabet)



def test_longacl(l,path):
	# mesures sur le getfacl
	test = RandomGen()

	u = commands.getoutput('rm ' + path + "/*")	# clean directory
	print "test acl getfacl\n"
	for i in range(l):
		test.getUserList()
		testfile = 'testfile' + str(i)
		u = commands.getoutput('touch ' + path + "/" + testfile)
		print "setfacl with " + str(i) + " entries\n " + u
		for j in range(i):
			user = test.uList.pop()
			mode = test.createRandomMode()
                        u = commands.getoutput('setfacl -m u:' + user + ':' + mode + " " + path + "/" + testfile)
	                if u != "":
                                print "setfacl -m u:" + user + ':' + mode + " " + path + "/" + testfile
                                print u
def main():
	parser = OptionParser()
	parser.add_option("-l", "--length", dest="length",type="int",help="max lentgh of ACL")
	parser.add_option("-p", "--path", dest="path",help="path of test file")
	(options, args) = parser.parse_args()
	test_longacl(options.length,options.path)
main()

