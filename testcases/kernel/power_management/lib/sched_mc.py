#!/usr/bin/python
''' Reusable functions related to sched mc FVT are put together
'''

import os
import sys

__author__ = "Vaidyanathan Srinivasan <svaidy@linux.vnet.ibm.com>"
__author__ = "Poornima Nayak <mpnayak@linux.vnet.ibm.com>"

cpu_map = {}
stats_start = {}
stats_stop = {}
stats_percentage = {}
cpu_count = 0

def clear_dmesg():
    '''
       Clears dmesg
    '''
    try:
        os.system('dmesg -c >/dev/null')
    except OSError, e:
        print 'Clearing dmesg failed', e
        sys.exit(1)

def get_proc_data(stats_list):
    ''' Read /proc/stat info and store in dictionary
    '''
    try:
        file_procstat = open("/proc/stat", 'r')
        for line in file_procstat:
            if line.startswith('cpu'):
                data = line.split()
                stats_list[data[0]] = data
        file_procstat.close()
    except OSError, e:
        print "Could not read statistics", e
        sys.exit(1)

def set_sched_mc_power(sched_mc_level):
    ''' Routine to set sched_mc_power_savings to required level
    '''
    try:
        os.system('echo %s > \
            /sys/devices/system/cpu/sched_mc_power_savings'
            % sched_mc_level)
    except OSError, e:
        print "Could not set sched_mc_power_savings to", e
        sys.exit(1)

def count_num_cpu():
    ''' Returns number of cpu's in system
    '''
    try:
        cpuinfo = open('/proc/cpuinfo', 'r')
        global cpu_count
        for line in cpuinfo:
            if line.startswith('processor'):
                cpu_count += 1
        cpuinfo.close()
    except IOError, e:
        print "Could not get cpu count", e
        sys.exit(1)

def map_cpuid_pkgid():
    ''' Routine to map physical package id to cpu id
    '''
    for i in range(0, cpu_count):
        try:
            phy_pkg_file = '/sys/devices/system/cpu/cpu%s' %i
            phy_pkg_file += '/topology/physical_package_id' 
            cpu_phy_id = open(phy_pkg_file).read().rstrip()
            try:
                cpu_map[cpu_phy_id].append(i)
            except KeyError:
                cpu_map[cpu_phy_id] = [i]
        except IOError, e:
            print "Mapping of CPU to pkg id failed", e
            sys.exit(1)

def trigger_workld(workload):
    ''' Based on value in argument passed triggers workloads.
        Triggers workload with number of threads same as number
        of cores in package. 
    '''
    threads = cpu_count / len(cpu_map)
    duration = 60 # let the test run for 1 minute 
    path = '%s/utils/benchmark' % os.environ['LTPROOT']
    try:
        olddir = os.getcwd()
        os.chdir(path)
        wklds_avlbl = list()
        workload_dir = ""
        # Use the latest version of similar workload available
        for file_name in os.listdir('.'):
            if file_name.find(workload) != -1:
                wklds_avlbl.append(file_name)
        wklds_avlbl.sort()
        workload_dir = wklds_avlbl[len(wklds_avlbl)-1]
           
        if workload_dir != "":
            new_path = os.path.join(path,"%s" % workload_dir)
            os.chdir(new_path)
            get_proc_data(stats_start)
            if workload == "ebizzy":
                try:
                    os.system('./ebizzy -t%s -s4096 -S %s >/dev/null'
                        % (threads, duration))
                    print "INFO: ebizzy workload triggerd"
                    os.chdir(olddir)
                    get_proc_data(stats_stop)
                except OSError, e: 
                    print "Workload trigger failed", e
                    sys.exit(1)
        else:
            print "Benchmark/Workload is missing in LTP utils"
            os.chdir(olddir)
            sys.exit(1)
    except OSError, e:
        print "Benchmark/Workload trigger failed ", e
        sys.exit(1)

def generate_report():
    ''' Generate report of CPU utilization
    '''
    cpu_labels = ('cpu', 'user', 'nice', 'system', 'idle', 'iowait', 'irq',
	'softirq', 'x', 'y')
    if (not os.path.exists('/procstat')):
        os.mkdir('/procstat')

    reportfile = open('/procstat/cpu-utilisation', 'a')
    debugfile = open('/procstat/cpu-utilisation.debug', 'a')
    for l in stats_stop:
        percentage_list = []
        total = 0
        for i in range(1, len(stats_stop[l])):
            stats_stop[l][i] =  int(stats_stop[l][i]) - int(stats_start[l][i])
            total += stats_stop[l][i]
        percentage_list.append(l)
        for i in range(1, len(stats_stop[l])):
            percentage_list.append(float(stats_stop[l][i])*100/total)

        stats_percentage[l] = percentage_list

    for i in range(0, len(cpu_labels)):
        print >> debugfile, cpu_labels[i], '\t',
    print >> debugfile
    for l in sorted(stats_stop.keys()):
        print >> debugfile, l, '\t',
        for i in range(1, len(stats_stop[l])):
            print >> debugfile, stats_stop[l][i], '\t',
        print >> debugfile

    for i in range(0, len(cpu_labels)):
        print >> reportfile, cpu_labels[i], '\t',
    print >> reportfile
    for l in sorted(stats_percentage.keys()):
        print >> reportfile, l, '\t',
        for i in range(1, len(stats_percentage[l])):
            print >> reportfile, " %3.4f" % stats_percentage[l][i],
        print >> reportfile

    #Now get the package ID information
    print >> debugfile, "cpu_map: ", cpu_map
    keyvalfile = open('/procstat/keyval', 'a')
    print >> keyvalfile, "nr_packages=%d" % len(cpu_map)
    print >> keyvalfile, "system-idle=%3.4f" % (stats_percentage['cpu'][4])
    for pkg in sorted(cpu_map.keys()):
        total_idle = 0
        total = 0 
        for cpu in cpu_map[pkg]:
            total_idle += stats_stop["cpu%d" % cpu][4]
            for i in range(1, len(stats_stop["cpu%d" % cpu])):
                total += stats_stop["cpu%d" % cpu][i]
        print >> reportfile, "Package: ", pkg, "Idle %3.4f%%" \
		% (float(total_idle)*100/total)
        print >> keyvalfile, "package-%s=%3.4f" % \
		(pkg, (float(total_idle)*100/total))

    #Add record delimiter '\n' before closing these files
    print >> debugfile
    debugfile.close()
    print >> reportfile
    reportfile.close()
    print >> keyvalfile
    keyvalfile.close()

def expand_range(range_val):
    '''
       Expand the range of value into actual numbers
    '''
    ids_list = list()
    sep_comma = range_val.split(",")
    for i in range(0, len(sep_comma)):
        hyphen_values = sep_comma[i].split("-")
        if len(hyphen_values) == 1:
            ids_list.append(int(hyphen_values[0]))
        else:
            for j in range(int(hyphen_values[0]), int(hyphen_values[1])+1):
                ids_list.append(j)
    return(ids_list)

def is_quad_core():
    '''
       Read /proc/cpuinfo and check is system is Quad core
    '''
    try:
        cpuinfo = open('/proc/cpuinfo', 'r')
        for line in cpuinfo:
            if line.startswith('cpu cores'):
                cores = line.split("cpu cores")
                num_cores = cores[1].split(":")
                cpuinfo.close()
                if int(num_cores[1]) == 4:
                    return(1)
                else:
                    return(0)
    except IOError, e:
        print "Failed to get cpu core information", e
        sys.exit(1)

def validate_cpugrp_map(cpu_group, sched_mc_level):
    '''
       Verify if cpugrp belong to same package
    '''
    for pkg in sorted(cpu_map.keys()):
        pkg_cpus = cpu_map[pkg]
        if pkg_cpus == cpu_group:
            return(0)
        else:
            if is_quad_core() and sched_mc_level == 0:
                return(0)

    print "INFO: cpu group does not belong to single package"
    sys.exit(1) 
 
def verify_sched_domain_dmesg(sched_mc_level):
    '''
       Read sched domain information from dmesg.
    '''
    cpu_group = list()
    try:
        dmesg_info = os.popen('dmesg -c').read()
        lines = dmesg_info.split('\n')
        for i in range(0, len(lines)):
            if lines[i].endswith('CPU'):
                groups = lines[i+1].split("groups:")
                subgroup = groups[1].split()
                for j in range(0, len(subgroup)):
                    cpu_group = expand_range(subgroup[j])
                    validate_cpugrp_map(cpu_group, sched_mc_level) 
    except OSError, e:
        print "Reading dmesg failed", e
        sys.exit(1)
    return(0)

def validate_cpu_consolidation(sched_mc_level):
    ''' Verify if cpu's on which threads execiuted belong to same
    package
    '''
    cpus_utilized = list()
    for l in sorted(stats_percentage.keys()):
        if stats_percentage[l][1] > 20:
            cpu_id = stats_percentage[l][0].split("cpu")
            if cpu_id[1] != '':
                cpus_utilized.append(int(cpu_id[1]))
    cpus_utilized.sort()
    print "INFO: CPU's utilized %s" %cpus_utilized

    validate_cpugrp_map(cpus_utilized, sched_mc_level)
    return(0)
