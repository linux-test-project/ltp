/*
 * cpuset header file
 *
 * Copyright (c) 2004-2006 Silicon Graphics, Inc. All rights reserved.
 *
 * Paul Jackson <pj@sgi.com>
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

/*
 * cpusets - basic routines (use cpuset relative numbering of CPUs)
 *
 *   link with -lbitmask -lcpuset
 *
 * cpuset_pin(int relcpu) - Pin current task to one CPU in its cpuset.
 * cpuset_size() - How many CPUs are in current tasks cpuset?
 * cpuset_where() - Most recent CPU in current tasks cpuset that task ran on.
 * cpuset_unpin() - Undo cpuset_pin(), let task run anywhere in its cpuset.
 */

/*
 * cpusets - sets of CPUs and Memory Nodes - advanced routines (use system
 *	     wide numbering of CPUs and Memory Nodes, except as noted)
 *
 *   link with -lbitmask -lcpuset
 *
 * cpuset_version() - [optional] Version (simple integer) of the library.
 *
 * ==== Allocate and free struct cpuset ====
 *
 * cpuset_alloc() - Allocate a new struct cpuset
 * cpuset_free(struct cpuset *cp) - Free struct cpuset *cp
 *
 * ==== Lengths of CPUs and Memory Nodes bitmasks - use to alloc them ====
 *
 * cpuset_cpus_nbits() - Number of bits in a CPU bitmask on current system
 * cpuset_mems_nbits() - Number of bits in a Memory bitmask on current system
 *
 * ==== Set various attributes of a struct cpuset ====
 *
 * cpuset_setcpus(cp, cpus) - Set CPUs in cpuset cp to bitmask cpus
 * cpuset_setmems(cp, mems) - Set Memory Nodes in cpuset cp to bitmask mems
 * cpuset_set_iopt(cp, optname, val) - Set integer value optname of cpuset cp
 * cpuset_set_sopt(cp, optname, val) - [optional] Set string value optname
 *
 * ==== Query various attributes of a struct cpuset ====
 *
 * cpuset_getcpus(cp, cpus) - Write CPUs in cpuset cp to bitmask cpus
 * cpuset_getmems(cp, mems) - Write Memory Nodes in cpuset cp to bitmask mems
 * cpuset_cpus_weight(cp) - Number of CPUs in a cpuset
 * cpuset_mems_weight(cp) - Number of Memory Nodes in a cpuset
 * cpuset_get_iopt(cp, optname) - Return integer value of option optname in cp
 * cpuset_get_sopt(cp, optname) - [optional] Return string value of optname
 *
 * ==== Local CPUs and Memory Nodes ====
 *
 * cpuset_localcpus(mems, cpus) - Set cpus to those local to Memory Nodes mems
 * cpuset_localmems(cpus, mems) - Set mems to those local to CPUs cpus
 * cpuset_cpumemdist(cpu, mem) - [optional] Hardware distance from CPU to Memory Node
 * cpuset_cpu2node(cpu) - [optional] Return Memory Node closest to cpu
 * cpuset_addr2node(addr) - [optional] Return Memory Node holding page at specified addr
 *
 * ==== Create, delete, query, modify, list and examine cpusets ====
 *
 * cpuset_create(path, cp) - Create cpuset 'cp' at location 'path'
 * cpuset_delete(path) - Delete cpuset at location 'path' (if empty)
 * cpuset_query(cp, path) - Set cpuset cp to the cpuset at location 'path'
 * cpuset_modify(path, cp) - Change cpuset at location 'path' to values of 'cp'
 * cpuset_getcpusetpath(pid, buf, buflen) - Get cpuset path of pid into buf
 * cpuset_cpusetofpid(cp, pid) - Get cpuset 'cp' of pid
 * cpuset_mountpoint() - [optional] Cpuset filesystem mount point
 * cpuset_collides_exclusive - [optional] True if would collide exclusive
 * cpuset_nuke(path, unsigned int seconds) - [optional] Remove cpuset anyway possible
 *
 * ==== List tasks (pids) currently attached to a cpuset ====
 *
 * cpuset_init_pidlist(path, recurseflag) - Return list pids in cpuset 'path'
 * cpuset_pidlist_length(pidlist) - Return number of elements in pidlist
 * cpuset_get_pidlist(pidlist, i) - Return i'th element of pidlist
 * cpuset_freepidlist(pidlist) - Free pidlist
 *
 * ==== Attach tasks to cpusets ====
 *
 * cpuset_move(pid, path) - Move task (pid == 0 for current) to a cpuset
 * cpuset_move_all(pidlist, path) - Move all tasks in pidlist to a cpuset
 * cpuset_move_cpuset_tasks(fromrelpath, torelpath) - [optional]
 *		Move all tasks in cpuset 'fromrelpath' to cpuset 'torelpath'
 * cpuset_migrate(pid, path) - [optional] Like cpuset_move - plus migrate memory
 * cpuset_migrate_all(pidlist, path) - [optional] cpuset_move_all plus migrate
 * cpuset_reattach(path) - Rebind cpus_allowed of each task in cpuset 'path'
 *
 * ==== Determine memory pressure ====
 *
 * cpuset_open_memory_pressure(path) - [optional] Open handle to read memory_pressure
 * cpuset_read_memory_pressure(han) - [optional] Read cpuset current memory_pressure
 * cpuset_close_memory_pressure(han) - [optional] Close handle to read memory pressure
 *
 * ==== Map between relative and system-wide CPU and Memory Node numbers ====
 *
 * cpuset_c_rel_to_sys_cpu(cp, cpu) - Map cpuset relative cpu to system wide
 * cpuset_c_sys_to_rel_cpu(cp, cpu) - Map system wide cpu to cpuset relative
 * cpuset_c_rel_to_sys_mem(cp, mem) - Map cpuset relative mem to system wide
 * cpuset_c_sys_to_rel_mem(cp, mem) - Map system wide mem to cpuset relative
 * cpuset_p_rel_to_sys_cpu(pid, cpu) - Map cpuset relative cpu to system wide
 * cpuset_p_sys_to_rel_cpu(pid, cpu) - Map system wide cpu to cpuset relative
 * cpuset_p_rel_to_sys_mem(pid, mem) - Map cpuset relative mem to system wide
 * cpuset_p_sys_to_rel_mem(pid, mem) - Map system wide mem to cpuset relative
 *
 * ==== Placement operations - for detecting cpuset migration ====
 *
 * cpuset_get_placement(pid) - [optional] Return current placement of task pid
 * cpuset_equal_placement(plc1, plc2) - [optional] True if two placements equal
 * cpuset_free_placement(plc) - [optional] Free placement
 *
 * ==== Traverse a cpuset hierarchy ====
 *
 * cpuset_fts_open(path) - [optional] Open cpuset hierarchy ==> cs_tree
 * cpuset_fts_read(cs_tree) - [optional] Next entry in tree ==> cs_entry
 * cpuset_fts_reverse(cs_tree) - [optional] Reverse order of entries in cs_tree
 * cpuset_fts_rewind(cs_tree) - [optional] Rewind cs_tree to beginning
 * cpuset_fts_get_path(cs_entry) - [optional] Get entry's cpuset path
 * cpuset_fts_get_stat(cs_entry) - [optional] Get entry's stat(2) pointer
 * cpuset_fts_get_cpuset(cs_entry) - [optional] Get entry's cpuset pointer
 * cpuset_fts_get_errno(cs_entry) - [optional] Get entry's errno
 * cpuset_fts_get_info(cs_entry) - [optional] Get operation causing error
 * cpuset_fts_close(cs_tree) - [optional] Close cpuset hierarchy
 *
 * ==== Bind to a CPU or Memory Node within the current cpuset ====
 *
 * cpuset_cpubind(cpu) - Bind current task to cpu (uses sched_setaffinity(2))
 * cpuset_latestcpu(pid) - Return most recent CPU on which task pid executed
 * cpuset_membind(mem) - Bind current task to memory (uses set_mempolicy(2))
 *
 * ==== Export cpuset settings to, and import from, a regular file ====
 *
 * cpuset_export(cp, file) - Export cpuset settings to a regular file
 * cpuset_import(cp, file) - Import cpuset settings from a regular file
 *
 * ==== Support calls to [optional] cpuset_* API routines ====
 *
 * cpuset_function - Return pointer to a libcpuset.so function, or NULL
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef _CPUSET_H
#define _CPUSET_H

#ifdef __cplusplus
extern "C" {
#endif

int cpuset_version(void);

int cpuset_pin(int relcpu);
int cpuset_size(void);
int cpuset_where(void);
int cpuset_unpin(void);

struct bitmask;
struct cpuset;
struct cpuset_pidlist;
struct cpuset_placement;
struct cpuset_fts_tree;
struct cpuset_fts_entry;

struct cpuset *cpuset_alloc(void);
void cpuset_free(struct cpuset *cp);

int cpuset_cpus_nbits(void);
int cpuset_mems_nbits(void);

int cpuset_setcpus(struct cpuset *cp, const struct bitmask *cpus);
int cpuset_setmems(struct cpuset *cp, const struct bitmask *mems);
int cpuset_set_iopt(struct cpuset *cp, const char *optionname, int value);
int cpuset_set_sopt(struct cpuset *cp, const char *optionname,
							const char *value);

int cpuset_open_memory_pressure(const char *cpusetpath);
int cpuset_read_memory_pressure(int han);
void cpuset_close_memory_pressure(int han);

int cpuset_getcpus(const struct cpuset *cp, struct bitmask *cpus);
int cpuset_getmems(const struct cpuset *cp, struct bitmask *mems);
int cpuset_cpus_weight(const struct cpuset *cp);
int cpuset_mems_weight(const struct cpuset *cp);
int cpuset_get_iopt(const struct cpuset *cp, const char *optionname);
const char *cpuset_get_sopt(const struct cpuset *cp, const char *optionname);

int cpuset_localcpus(const struct bitmask *mems, struct bitmask *cpus);
int cpuset_localmems(const struct bitmask *cpus, struct bitmask *mems);
unsigned int cpuset_cpumemdist(int cpu, int mem);
int cpuset_cpu2node(int cpu);
int cpuset_addr2node(void *addr);

int cpuset_create(const char *cpusetpath, const struct cpuset *cp);
int cpuset_delete(const char *cpusetpath);
int cpuset_query(struct cpuset *cp, const char *cpusetpath);
int cpuset_modify(const char *cpusetpath, const struct cpuset *cp);
char *cpuset_getcpusetpath(pid_t pid, char *buf, size_t size);
int cpuset_cpusetofpid(struct cpuset *cp, pid_t pid);
const char *cpuset_mountpoint(void);
int cpuset_collides_exclusive(const char *cpusetpath, const struct cpuset *cp);
int cpuset_nuke(const char *cpusetpath, unsigned int seconds);

struct cpuset_pidlist *cpuset_init_pidlist(const char *cpusetpath,
							int recursiveflag);
int cpuset_pidlist_length(const struct cpuset_pidlist *pl);
pid_t cpuset_get_pidlist(const struct cpuset_pidlist *pl, int i);
void cpuset_freepidlist(struct cpuset_pidlist *pl);

int cpuset_move(pid_t pid, const char *cpusetpath);
int cpuset_move_all(struct cpuset_pidlist *pl, const char *cpusetpath);
int cpuset_move_cpuset_tasks(const char *fromrelpath, const char *torelpath);
int cpuset_migrate(pid_t pid, const char *cpusetpath);
int cpuset_migrate_all(struct cpuset_pidlist *pl, const char *cpusetpath);
int cpuset_reattach(const char *cpusetpath);

int cpuset_c_rel_to_sys_cpu(const struct cpuset *cp, int cpu);
int cpuset_c_sys_to_rel_cpu(const struct cpuset *cp, int cpu);
int cpuset_c_rel_to_sys_mem(const struct cpuset *cp, int mem);
int cpuset_c_sys_to_rel_mem(const struct cpuset *cp, int mem);

int cpuset_p_rel_to_sys_cpu(pid_t pid, int cpu);
int cpuset_p_sys_to_rel_cpu(pid_t pid, int cpu);
int cpuset_p_rel_to_sys_mem(pid_t pid, int mem);
int cpuset_p_sys_to_rel_mem(pid_t pid, int mem);

struct cpuset_placement *cpuset_get_placement(pid_t pid);
int cpuset_equal_placement(const struct cpuset_placement *plc1,
					const struct cpuset_placement *plc2);
void cpuset_free_placement(struct cpuset_placement *plc);

struct cpuset_fts_tree *cpuset_fts_open(const char *cpusetpath);
const struct cpuset_fts_entry *cpuset_fts_read(
				struct cpuset_fts_tree *cs_tree);
void cpuset_fts_reverse(struct cpuset_fts_tree *cs_tree);
void cpuset_fts_rewind(struct cpuset_fts_tree *cs_tree);
const char *cpuset_fts_get_path(
				const struct cpuset_fts_entry *cs_entry);
const struct stat *cpuset_fts_get_stat(
				const struct cpuset_fts_entry *cs_entry);
const struct cpuset *cpuset_fts_get_cpuset(
				const struct cpuset_fts_entry *cs_entry);
int cpuset_fts_get_errno(const struct cpuset_fts_entry *cs_entry);
int cpuset_fts_get_info(const struct cpuset_fts_entry *cs_entry);
void cpuset_fts_close(struct cpuset_fts_tree *cs_tree);

int cpuset_cpubind(int cpu);
int cpuset_latestcpu(pid_t pid);
int cpuset_membind(int mem);

int cpuset_export(const struct cpuset *cp, char *buf, int buflen);
int cpuset_import(struct cpuset *cp, const char *buf, int *elinenum,
							char *emsg, int elen);

void *cpuset_function(const char * function_name);

/*
 * cpuset_fts_entry.info values.
 *
 * Because the cpuset_fts_open() call collects all the information
 * at once from an entire cpuset subtree, a simple error return would
 * not provide sufficient information as to what failed, and on what
 * cpuset in the subtree.  So, except for malloc(3) failures, errors
 * are captured in the list of entries.  If an entry has one of the
 * following CPUSET_FTS_ERR_* values in the "info" field, then the "info"
 * field indicates which operation failed, the "err" field captures the
 * failing errno value for that operation, and the other entry fields
 * might not be valid.  If an entry has the value "CPUSET_FTS_CPUSET" for its
 * "info" field, then the err field will have the value "0", and the
 * other fields will be contain valid information about that cpuset.
 *
 */
enum {
	CPUSET_FTS_CPUSET = 0,		/* valid cpuset */
	CPUSET_FTS_ERR_DNR = 1,		/* error - couldn't read directory */
	CPUSET_FTS_ERR_STAT = 2,	/* error - couldn't stat directory */
	CPUSET_FTS_ERR_CPUSET = 3,	/* error - cpuset_query() failed */
};

/*
 * If it necessary to maintain source code compatibility with earlier
 * versions of this header file lacking the above CPUSET_FTS_* values,
 * one can conditionally check that the C preprocessor symbol
 * CPUSET_FTS_INFO_VALUES_DEFINED symbol is not defined and provide
 * alternative coding for that case.
 */
#define CPUSET_FTS_INFO_VALUES_DEFINED 1

#ifdef __cplusplus
}
#endif

#endif
