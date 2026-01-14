
#include "ht_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <linux/unistd.h>
#include "tso_cpuid.h"

#define PROC_PATH	"/proc"
#define BUFF_SIZE	8192
#define PROCESSOR_STR	"processor"

#define MAX_CPU_NUM 128

char buffer[BUFF_SIZE];

int is_ht_cpu(void)
{
	/*Number of logic processor in a physical processor */
	int smp_num_siblings = -1;
	/*ht flag */
	int ht = -1;
	unsigned int eax, ebx, ecx, edx;
	cpuid(1, &eax, &ebx, &ecx, &edx);
	smp_num_siblings = (ebx & 0xff0000) >> 16;
	ht = (edx & 0x10000000) >> 28;

	if (ht == 1 && smp_num_siblings >= 2) {
		/*printf("The processor in this system supports HT\n"); */
		return 1;
	} else {
		/*printf("The processor in this system does not support
		 * HT\n");*/
		return 0;
	}
}

/* return 0 means Pass,
 return 1 means ht is not enabled,*/
int check_ht_capability(void)
{
	int result;
	if (is_ht_cpu())
		result = 0;	/*HT is enabled by default in this system. */
	else
		result = 1;	/*HT is not enabled by default in this system. */

	return result;
}

#define PROCFS_PATH "/proc/"
#define CPUINFO_PATH "/proc/cpuinfo"
#define CPU_NAME "processor"
#define STAT_NAME "stat"

char buf[256];

int get_cpu_count(void)
{
	FILE *pfile;
	int count;

	pfile = fopen(CPUINFO_PATH, "r");
	if (pfile == NULL)
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

	pfile = fopen(buf, "r");
	if (pfile == NULL)
		return -1;

	if (fscanf(pfile, "%d %s %c %d %d %d %d %d %d %d %d %d %d %d %d %d %d\
	 %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &da, str, &ch, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &da, &cpu) <= 0) {
		fclose(pfile);
		return -1;
	}

	fclose(pfile);

	return cpu;
}
