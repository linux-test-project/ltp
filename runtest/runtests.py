#!/usr/bin/python

import string
import testobj
import sys
import os

fargs=sys.argv[1:]
lineset=[]

# read from multiple files all the lines in the files

for file in fargs:
  try:
    testfile = open(file,'r')
  except IOError, otherdata:
    print 'IOError:', otherdata 
    sys.exit()
  else:
    lineset=lineset+testfile.readlines()
    testfile.close()

tlist = testobj.test_list()

# for each line, try to build a test object.  Add each test
# object to the test list.

for line in lineset:
  try:
    new_test_obj = testobj.test(line)
    tlist.add(new_test_obj)
  except testobj.test.TestNoObjectError:
    None
  except testobj.test.TestBadObjectError:
    print "bad test record '%s'" % (string.strip(line))
  except testobj.test_list.TestNameExistsError, tname:
    print "Test name '%s' already exists, ignoring" % (tname)

# run all the tests in the test list
sys.exit(tlist.run())
