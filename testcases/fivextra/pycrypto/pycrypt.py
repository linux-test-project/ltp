#!/usr/bin/env python
# Test script for the Python Cryptography Toolkit.
#
# Modified from the source for LTP
#	- Robb Romans <robb@austin.ibm.com>

import os, sys

# Add the current directory to the front of sys.path
from distutils.util import get_platform
s = os.path.join(os.getcwd(), 'pycrypttest')
s = os.path.join(os.getcwd(), s)
sys.path.insert(0, s)
s = os.path.join(os.getcwd(), 'testcases/bin/pycrypttest')
sys.path.insert(0, s)

from Crypto.Util import test

args = sys.argv[1:]
quiet = "--quiet" in args
if quiet: args.remove('--quiet')

if not quiet:
    print '\nStream Ciphers:'
    print '==============='

if args: test.TestStreamModules(args, verbose= not quiet)
else: test.TestStreamModules(verbose= not quiet)

if not quiet:
    print '\nBlock Ciphers:'
    print '=============='

if args: test.TestBlockModules(args, verbose= not quiet)
else: test.TestBlockModules(verbose= not quiet)


