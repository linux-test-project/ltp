import string
import runtest

# Test object contains a test identifier and an associated command
# line.  The constructor is passed a string which contains an id
# token followed by the tokens of the command line of the test.
# TestNoObjectError is raised when the line is blank or contains only
# comments.
# TestBadObjectError is raised when there are not two tokens in the
# line: id & cmd

class test:
  TestNoObjectError = 'Failed attempt to create test object with no information'
  TestBadObjectError = 'Failed attempt to create test object with incomplete information'
  def __init__(self, t_line):
    l_line = string.strip(t_line)

    # look for comments
    cmt_start=string.find(l_line,'#')
    if cmt_start != -1:
      l_line = l_line[:cmt_start]
      l_line = string.strip(l_line)
    if l_line == '':
      raise test.TestNoObjectError 

    self.d_name = string.split(l_line)[0]
    self.d_cmd = string.join(string.split(l_line)[1:])

    if self.d_name == '' or self.d_cmd == '':
      raise test.TestBadObjectError 

  def name(self):
    return self.d_name
  
  def command(self):
    return self.d_cmd

  def __cmp__(self,other):
    if type(other) == type(self.d_name):
      return cmp(self.d_name,other)
    else:
      return cmp(self.d_name,other.d_name)

# test_list is a class which is intended to contain a list of the test objects.
# The test object ids must be unique, otherwise a TestNameExistsError exception
# is thrown.

class test_list:
  TestNameExistsError = 'Attempt to add test object with duplicate id'

  def __init__(self):
    self.d_testlist = []

  def add(self, a_tobj):
    for obj in self.d_testlist:
      if obj == a_tobj:
        raise test_list.TestNameExistsError, a_tobj.name()
    self.d_testlist.append(a_tobj)

  def echo(self):
    for obj in self.d_testlist:
      print "%s %s" % \
      (obj.name(),obj.command())

  def run(self):
    exit_report=0
    for obj in self.d_testlist:
      exit_report = exit_report | runtest.execute(obj.name(),obj.command())
    return exit_report
