import os
import sys
import string
import popen2

def execute(id,cmd):
    exit_report=0
    print "%s: running cmdline '%s'"%(id,cmd)
    subproc = popen2.Popen3(cmd,1)
    exit_status = subproc.wait()>>8
    stdout_lines = subproc.fromchild.readlines()
    stderr_lines = subproc.childerr.readlines()
    if len(stdout_lines) > 0:
      print "%s: stdout:" % (id)
      for line in stdout_lines:
        line = string.strip(line)
        print line
    if len(stderr_lines) > 0:
      print "%s: stderr:" % (id)
      for line in stderr_lines:
        line = string.strip(line)
        print line

    if exit_status != 0:
      print '%s: exit: %d' %(id,exit_status)
      result = "FAIL"
      exit_report=1
    else:
      result = "PASS"

    print "%s: result: %s" % (id,result)
    return exit_report
    
