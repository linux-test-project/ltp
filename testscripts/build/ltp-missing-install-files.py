#!/usr/bin/env python
#
#    Script for determining items missing from LTP install based on the output
#    log provided by runltp[lite.sh].
#
#    Copyright (C) 2009, Cisco Systems Inc.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Ngie Cooper, July 2009
#
# Please invoke this script with --help to determine usage.
#

from optparse import OptionParser
import os, re, sys

parser = OptionParser(usage='usage: %prog [options] logfile ...')

parser.add_option('-v', '--verbose', action='store_true', default=False,
                  dest='verbose', help=('print out successful results as '
                                        'well as failures'))

opts, logfiles = parser.parse_args()

if not len(logfiles):
    parser.print_help()

for logfile in logfiles:
    if not os.access(logfile, os.R_OK):
        sys.exit("%s not readable" % logfile)

todo_res = [
    re.compile("""initiation_status="pan\(\d+\): execvp of '(?P<app>.+)' \(tag (?P<tag>\w+)\) failed.+errno:2\s+No such file or directory"""),
    re.compile("(?P<tag>\S+): line \d+: (?P<app>\S+): No such file or directory"),
    re.compile("(?P<caller>\S+): (?P<app>\s+): No such file or directory"),
    re.compile("""tag=(?P<tag>\w+) [\w=]+
cmdline=.+
contacts=.+
analysis=.+
<<<test_output>>>
(\S+): (?P<app>\w+): command not found
<<<execution_status>>>
initiation_status=.+
.+termination_id=127.+""")
]

for logfile in logfiles:
    fd = open(logfile)

    # Case 1:

    # initiation_status="pan(9908): execvp of 'fs_perms_simpletest.sh' (tag fs_perms) failed.  errno:2  No such file or directory"

    # Case 2:

    # /scratch/ltp-install4/testcases/bin/test_controllers.sh: line 109: /scratch/ltp-install4/testcases/bin/run_cpuset_test.sh: No such file or directory

    # Case 3:

    # gcc: /scratch/ltp-install4/testcases/bin/nmfile2.c: No such file or directory

    # Case 4:

    # <<<test_start>>>
    # tag=iogen01 stime=1248638309
    # cmdline="export LTPROOT; rwtest -N iogen01 -i 120s -s read,write -Da -Dv -n 2 500b:doio.f1.$$ 1000b:doio.f2.$$"
    # contacts=""
    # analysis=exit
    # <<<test_output>>>
    # sh: rwtest: command not found
    # <<<execution_status>>>
    # initiation_status="ok"
    # duration=0 termination_type=exited termination_id=127 corefile=no

    missing_ents = []

    try:

        lines = fd.readlines()

        for line in lines:

            for todo_re in todo_res[:-1]:

                m = todo_re.match(line)
                if m:
                    missing_ent = " ".join([m.group(1), m.group('app')])
                    if missing_ent not in missing_ents:
                        missing_ents.append(missing_ent)
                    break

        for m in todo_res[2].finditer("".join(lines)):
            missing_ent = " ".join([m.group('tag'), m.group('app')])
            if missing_ent not in missing_ents:
                missing_ents.append(missing_ent)

    finally:
        fd.close()

    if len(missing_ents):
        print "\n".join(["%s: %s" % (os.path.basename(logfile), i) for i in ["Tag | App"] + missing_ents])
    elif opts.verbose:
        print "%s: CONGRATULATIONS -- no missing files found!" % os.path.basename(logfile)
