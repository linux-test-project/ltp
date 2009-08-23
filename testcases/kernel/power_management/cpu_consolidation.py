#!/usr/bin/python
''' This Python script interprets various sched stats values.
    Validates cpu consolidation for given sched_mc_power_saving value
'''

import os
import sys
LIB_DIR = "%s/testcases/kernel/power_management/lib" % os.environ['LTPROOT']
sys.path.append(LIB_DIR)
from optparse import OptionParser
from sched_mc import *

__author__ = "Poornima Nayak <mpnayak@linux.vnet.ibm.com>"

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

def main(argv=None):
    if argv is None:
        argv = sys.argv

    usage = "-w"
    parser = OptionParser(usage)
    parser.add_option("-c", "--mc_level", dest="mc_level",
        default=0, help="Sched mc power saving value 0/1/2")
    parser.add_option("-t", "--smt_level", dest="smt_level",
        default=0, help="Sched smt power saving value 0/1/2")
    parser.add_option("-w", "--workload", dest="work_ld",
        default="ebizzy", help="Workload can be ebizzy/kernbench")
    parser.add_option("-s", "--stress", dest="stress",
        default="partial", help="Load on system is full/partial [i.e 50%]/thread")
    (options, args) = parser.parse_args()

    try:
        count_num_cpu()
        count_num_sockets()
        if is_multi_socket():
            set_sched_mc_power(options.mc_level)
        if is_hyper_threaded():
            set_sched_smt_power(options.smt_level)
        map_cpuid_pkgid()
        print "INFO: Created table mapping cpu to package"
        background="no"
        duration=60
        trigger_workld(options.work_ld, options.stress, duration, background)
        generate_report()
        status = validate_cpu_consolidation(options.work_ld,options.mc_level, options.smt_level)
        reset_schedmc()
        if is_hyper_threaded():
            reset_schedsmt()
        return(status)        
    except Exception, details:
        print "INFO: CPU consolidation failed", details
        return(1)

if __name__ == "__main__":
    sys.exit(main())
