// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2026
 * Copyright (c) 2026 Li Wang <li.wang@linux.dev>
 */

#ifndef TST_PATH_DEFS__
#define TST_PATH_DEFS__

/* KERNEL */
#define PATH_KERN_HOSTNAME			"/proc/sys/kernel/hostname"
#define PATH_KERN_OSRELEASE			"/proc/sys/kernel/osrelease"
#define PATH_KERN_VERSION			"/proc/sys/kernel/version"
#define PATH_KERN_DOMAINNAME			"/proc/sys/kernel/domainname"
#define PATH_KERN_PRINTK			"/proc/sys/kernel/printk"
#define PATH_KERN_PID_MAX			"/proc/sys/kernel/pid_max"
#define PATH_KERN_SHMMAX			"/proc/sys/kernel/shmmax"
#define PATH_KERN_SHMMNI			"/proc/sys/kernel/shmmni"
#define PATH_KERN_SHMALL			"/proc/sys/kernel/shmall"
#define PATH_KERN_SHM_NEXT_ID			"/proc/sys/kernel/shm_next_id"
#define PATH_KERN_MSGMNI			"/proc/sys/kernel/msgmni"
#define PATH_KERN_MSG_NEXT_ID			"/proc/sys/kernel/msg_next_id"
#define PATH_KERN_SEM				"/proc/sys/kernel/sem"
#define PATH_KERN_CORE_PATTERN			"/proc/sys/kernel/core_pattern"
#define PATH_KERN_CAP_LAST_CAP			"/proc/sys/kernel/cap_last_cap"
#define PATH_KERN_NUMA_BALANCING		"/proc/sys/kernel/numa_balancing"
#define PATH_KERN_IO_URING_DISABLED		"/proc/sys/kernel/io_uring_disabled"
#define PATH_KERN_OVERFLOWUID			"/proc/sys/kernel/overflowuid"
#define PATH_KERN_OVERFLOWGID			"/proc/sys/kernel/overflowgid"
#define PATH_KERN_PERF_EVENT_PARANOID		"/proc/sys/kernel/perf_event_paranoid"
#define PATH_KERN_PERF_EVENT_MLOCK_KB		"/proc/sys/kernel/perf_event_mlock_kb"
#define PATH_KERN_PERF_EVENT_MAX_SAMPLE_RATE	"/proc/sys/kernel/perf_event_max_sample_rate"
#define PATH_KERN_SCHED_RT_PERIOD_US		"/proc/sys/kernel/sched_rt_period_us"
#define PATH_KERN_SCHED_RT_RUNTIME_US		"/proc/sys/kernel/sched_rt_runtime_us"
#define PATH_KERN_SCHED_RR_TIMESLICE_MS		"/proc/sys/kernel/sched_rr_timeslice_ms"
#define PATH_KERN_UNPRIVILEGED_USERNS_CLONE	"/proc/sys/kernel/unprivileged_userns_clone"

/* USER */
#define PATH_USER_MAX_USER_NAMESPACES		"/proc/sys/user/max_user_namespaces"
#define PATH_USER_MAX_FANOTIFY_GROUPS		"/proc/sys/user/max_fanotify_groups"
#define PATH_USER_MAX_FANOTIFY_MARKS		"/proc/sys/user/max_fanotify_marks"

/* FS */
#define PATH_FS_NR_OPEN				"/proc/sys/fs/nr_open"
#define PATH_FS_NR_AIO_MAX_NR			"/proc/sys/fs/aio-max-nr"
#define PATH_FS_PIPE_MAX_SIZE			"/proc/sys/fs/pipe-max-size"
#define PATH_FS_PIPE_MAX_PAGES			"/proc/sys/fs/pipe-max-pages"
#define PATH_FS_MAX_USER_GROUPS			"/proc/sys/fs/fanotify/max_user_groups"
#define PATH_FS_MAX_USER_MARKS			"/proc/sys/fs/fanotify/max_user_marks"

/* VM */
#define PATH_VM_DROP_CACHES			"/proc/sys/vm/drop_caches"
#define PATH_VM_COMPACT_MEMORY			"/proc/sys/vm/compact_memory"
#define PATH_VM_PANIC_ON_OOM			"/proc/sys/vm/panic_on_oom"
#define PATH_VM_OVERCOMMIT_MEMORY		"/proc/sys/vm/overcommit_memory"
#define PATH_VM_OVERCOMMIT_RATIO		"/proc/sys/vm/overcommit_ratio"
#define PATH_VM_MIN_FREE_KBYTES			"/proc/sys/vm/min_free_kbytes"
#define PATH_VM_VFS_CACHE_PRESSURE		"/proc/sys/vm/vfs_cache_pressure"

/* HUGETLB */
#define PATH_MM_HUGEPAGES			"/sys/kernel/mm/hugepages"
#define PATH_MM_THP				"/sys/kernel/mm/transparent_hugepage"
#define PATH_VM_NR_HPAGES			"/proc/sys/vm/nr_hugepages"
#define PATH_VM_OVERCOMMIT_HPAGES		"/proc/sys/vm/nr_overcommit_hugepages"

/* KSM */
#define PATH_MM_KSM				"/sys/kernel/mm/ksm"
#define PATH_MM_KSM_RUN				"/sys/kernel/mm/ksm/run"
#define PATH_MM_KSM_FULL_SCANS			"/sys/kernel/mm/ksm/full_scans"
#define PATH_MM_KSM_SLEEP_MILLISECS		"/sys/kernel/mm/ksm/sleep_millisecs"
#define PATH_MM_KSM_MAX_PAGE_SHARING		"/sys/kernel/mm/ksm/max_page_sharing"
#define PATH_MM_KSM_MERGE_ACROSS_NODES		"/sys/kernel/mm/ksm/merge_across_nodes"
#define PATH_MM_KSM_SMART_SCAN			"/sys/kernel/mm/ksm/smart_scan"
#define PATH_MM_KSM_PAGES_TO_SCAN		"/sys/kernel/mm/ksm/pages_to_scan"
#define PATH_MM_KSM_PAGES_SKIPPED		"/sys/kernel/mm/ksm/pages_skipped"

/* NETWORK */
#define PATH_IPV4_ICMP_RATELIMIT		"/proc/sys/net/ipv4/icmp_ratelimit"
#define PATH_IPV4_ICMP_RATEMASK			"/proc/sys/net/ipv4/icmp_ratemask"
#define PATH_IPV4_ICMP_ECHO_IGNORE_ALL		"/proc/sys/net/ipv4/icmp_echo_ignore_all"
#define PATH_IPV4_ICMP_MSGS_BURST		"/proc/sys/net/ipv4/icmp_msgs_burst"
#define PATH_IPV4_TCP_PROBE_INTERVAL		"/proc/sys/net/ipv4/tcp_probe_interval"
#define PATH_IPV4_TCP_KEEPALIVE_TIME		"/proc/sys/net/ipv4/tcp_keepalive_time"
#define PATH_IPV4_TCP_NOTSENT_LOWAT		"/proc/sys/net/ipv4/tcp_notsent_lowat"
#define PATH_IPV4_IP_LOCAL_RESERVED_PORTS	"/proc/sys/net/ipv4/ip_local_reserved_ports"

/* MEMINFO */
#define MEMINFO_HPAGE_TOTAL	"HugePages_Total:"
#define MEMINFO_HPAGE_FREE	"HugePages_Free:"
#define MEMINFO_HPAGE_RSVD	"HugePages_Rsvd:"
#define MEMINFO_HPAGE_SURP	"HugePages_Surp:"
#define MEMINFO_HPAGE_SIZE	"Hugepagesize:"

#endif /* TST_PATH_DEFS__ */
