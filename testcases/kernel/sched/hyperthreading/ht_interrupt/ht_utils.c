
#include "ht_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <linux/unistd.h>
#include "ltp_cpuid.h"

#define PROC_PATH	"/proc"
#define BUFF_SIZE	8192
#define PROCESSOR_STR	"processor"
#define PACKAGE_STR	"cpu_package"
#define HT_FLAG "ht"
#define FLAG_STR "flags"

#define MAX_CPU_NUM 128

char buffer[BUFF_SIZE];

int is_cmdline_para(const char *para)
{
	FILE *fp;

	if ((fp = fopen("/proc/cmdline", "r")) != NULL && para != NULL) {
		while (fgets(buffer, BUFF_SIZE - 1, fp) != NULL) {
			if (strstr(buffer, para) != NULL) {
				fclose(fp);
				return 1;
			}
		}
	}
	/* If fopen succeeds and the pointer para is NULL,
	 * It won't enter the above if-block.
	 * so need to close fp here.
	 */
	if (fp != NULL)
		fclose(fp);

	return 0;
}

int is_ht_kernel()
{
	FILE *fp;

	if ((fp = fopen("/proc/cpuinfo", "r")) != NULL) {
		while (fgets(buffer, BUFF_SIZE - 1, fp) != NULL) {
			if (strncmp(buffer, PACKAGE_STR, strlen(PACKAGE_STR)) ==
			    0) {
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}

	return 0;
}

int is_ht_cpu()
{
	/*Number of logic processor in a physical processor */
	int smp_num_siblings = -1;
	/*ht flag */
	int ht = -1;
	unsigned int eax, ebx, ecx, edx;
	cpuid(1, &eax, &ebx, &ecx, &edx);
	smp_num_siblings = (ebx & 0xff0000) >> 16;
	ht = (edx & 0x10000000) >> 28;

	if (ht == 1 && smp_num_siblings == 2) {
//              printf("The processor in this system supports HT\n");
		return 1;
	} else {
//              printf("The processor in this system does not support HT\n");
		return 0;
	}
}

int is_ht_enabled()
{
	int cpu_map[MAX_CPU_NUM];
	/*A bit-map shows whether a 'logic' processor has ht flag */
	int ht_cpu[MAX_CPU_NUM];
	int logic_cpu_num = 0;
	int package = -1;
	int cpu_id = -1;
	char *ht_flag = NULL;
	int i = 0;
	int j = 0;
	int k = 0;

	FILE *fp;
	char *proc_cpuinfo =
	    (char *)alloca(strlen(PROC_PATH) + sizeof("/cpuinfo"));
	strcat(strcpy(proc_cpuinfo, PROC_PATH), "/cpuinfo");

	if ((fp = fopen(proc_cpuinfo, "r")) != NULL) {
		while (fgets(buffer, BUFF_SIZE - 1, fp) != NULL) {
			if (strncmp
			    (buffer, PROCESSOR_STR,
			     strlen(PROCESSOR_STR)) == 0) {
				sscanf(buffer, PROCESSOR_STR "\t: %d", &cpu_id);
				ht_cpu[cpu_id] = 0;
				while (fgets(buffer, BUFF_SIZE - 1, fp) != NULL) {
					if (strncmp
					    (buffer, PACKAGE_STR,
					     strlen(PACKAGE_STR)) == 0) {
						sscanf(buffer,
						       PACKAGE_STR "\t: %d",
						       &package);
						cpu_map[cpu_id] = package;
						printf("cpu_map[%d]=%d\n",
						       cpu_id, package);
					}
					if (strncmp
					    (buffer, FLAG_STR,
					     strlen(FLAG_STR)) == 0) {
						ht_flag = buffer;
						while (*ht_flag != '\0') {
							/*printf("ht_flag=%s",ht_flag); */
							if (strncmp
							    (ht_flag, HT_FLAG,
							     strlen(HT_FLAG)) ==
							    0) {
								ht_cpu[cpu_id] =
								    1;
								break;
							}
							ht_flag++;
						}
						printf("ht_cpu[%d]=%d\n",
						       cpu_id, ht_cpu[cpu_id]);
						logic_cpu_num += 1;
						break;
					}
				}
			}
		}
	} else
		return 0;

	fclose(fp);

	for (i = 0; i < logic_cpu_num; i++) {
		if (ht_cpu[i] == 1) {
			for (j = i + 1; j < logic_cpu_num; j++) {
				if (cpu_map[i] == cpu_map[j]) {
					for (k = j + 1; k < logic_cpu_num; k++) {
						if (cpu_map[j] == cpu_map[k]) {
							/* Not proper HT support, with 3 logic processor in 1 cpu package */
							return 0;
						}
					}
					if (ht_cpu[j] == 1) {
						return 1;
					} else
						return 0;
				}
			}
			/* in this case, processor[i] has ht flag, but is not ht enabled */
			if (j == logic_cpu_num) {
				return 0;
			}
		}
	}
	if (i == logic_cpu_num) {
		return 0;
	}
	return 0;
}

// return 0 means Pass,
// return 1 means ht is not enabled,
// return 2 means CPU is not support ht,
// return 3 mean kernel is not support ht.
int check_ht_capability()
{
	int result;

	if (is_ht_kernel()) {
		if (is_ht_cpu()) {
			if (is_ht_enabled())
				result = 0;	//HT is enabled by default in this system.
			else
				result = 1;	//HT is not enabled by default in this system.
		} else
			result = 2;	//This processor does not support HT.
	} else
		result = 3;	//HT feature is not included in this Linux Kernel.

	return result;
}

#define PROCFS_PATH "/proc/"
#define CPUINFO_PATH "/proc/cpuinfo"
#define CPU_NAME "processor"
#define STAT_NAME "stat"

char buf[256];

int get_cpu_count()
{
	FILE *pfile;
	int count;

	if ((pfile = fopen(CPUINFO_PATH, "r")) == NULL)
		return 0;

	count = 0;

	while (fgets(buf, 255, pfile) != NULL) {
		if (strncmp(buf, CPU_NAME, strlen(CPU_NAME)) == 0)
			count++;
	}

	fclose(pfile);

	return count;
}

int get_current_cpu(pid_t pid)
{
	int cpu = -1;
	int da;
	char str[100];
	char ch;

	FILE *pfile;

	sprintf(buf, "%s%d/%s%c", PROCFS_PATH, pid, STAT_NAME, 0);

	if ((pfile = fopen(buf, "r")) == NULL)
		return -1;

	if (fscanf
	    (pfile,
	     "%d %s %c %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
	     &da, str, &ch, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da,
	     &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da,
	     &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da,
	     &cpu) <= 0) {
		fclose(pfile);
		return -1;
	}

	fclose(pfile);

	return cpu;
}
