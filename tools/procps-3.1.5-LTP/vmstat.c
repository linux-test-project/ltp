// old: "Copyright 1994 by Henry Ware <al172@yfn.ysu.edu>. Copyleft same year."
// most code copyright 2002 Albert Cahalan

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/dir.h>
#include <dirent.h>

#include "proc/sysinfo.h"
#include "proc/version.h"

#define BUFFSIZE 8192
#define FALSE 0
#define TRUE 1

static char buff[BUFFSIZE]; /* used in the procedures */

typedef unsigned long long jiff;

static int a_option; /* "-a" means "show active/inactive" */

static unsigned sleep_time = 1;
static unsigned long num_updates;

static unsigned int height;   // window height
static unsigned int moreheaders=TRUE;


/////////////////////////////////////////////////////////////////////////

static void usage(void) NORETURN;
static void usage(void) {
  fprintf(stderr,"usage: vmstat [-V] [-n] [delay [count]]\n");
  fprintf(stderr,"              -V prints version.\n");
  fprintf(stderr,"              -n causes the headers not to be reprinted regularly.\n");
  fprintf(stderr,"              -a print inactive/active page stats.\n");
  fprintf(stderr,"              delay is the delay between updates in seconds. \n");
  fprintf(stderr,"              count is the number of updates.\n");
  exit(EXIT_FAILURE);
}

static void crash(const char *filename) NORETURN;
static void crash(const char *filename) {
    perror(filename);
    exit(EXIT_FAILURE);
}


////////////////////////////////////////////////////////////////////////

static void getrunners(unsigned int *restrict running, unsigned int *restrict blocked) {
  static struct direct *ent;
  DIR *proc;

  *running=0;
  *blocked=0;

  if((proc=opendir("/proc"))==NULL) crash("/proc");

  while(( ent=readdir(proc) )) {
    unsigned size;
    int fd;
    char filename[80];
    char c;
    if (!isdigit(ent->d_name[0])) continue;
    sprintf(filename, "/proc/%s/stat", ent->d_name);
    fd = open(filename, O_RDONLY, 0);
    if (fd == -1) continue;
    read(fd,buff,BUFFSIZE-1);
    sscanf(
      buff,
      "%*d %*s %c "
      "%*d %*d %*d %*d %*d %*u %*u"
      " %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %*u %*d %*u %u"
      /*  " %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u\n"  */ ,
      &c,
      &size
    );
    close(fd);

    if (c=='R') {
      (*running)++;
      continue;
    }
    if (c=='D') {
      (*blocked)++;
      continue;
    }
  }
  closedir(proc);
}

static void getstat(jiff *restrict cuse, jiff *restrict cice, jiff *restrict csys, jiff *restrict cide, jiff *restrict ciow,
	     unsigned *restrict pin, unsigned *restrict pout, unsigned *restrict s_in, unsigned *restrict sout,
	     unsigned *restrict intr, unsigned *restrict ctxt,
	     unsigned int *restrict running, unsigned int *restrict blocked,
	     unsigned int *restrict btime, unsigned int *restrict processes) {
  static int fd;
  int need_vmstat_file = 0;
  int need_proc_scan = 0;
  const char* b;
  buff[BUFFSIZE-1] = 0;  /* ensure null termination in buffer */

  if(fd){
    lseek(fd, 0L, SEEK_SET);
  }else{
    fd = open("/proc/stat", O_RDONLY, 0);
    if(fd == -1) crash("/proc/stat");
  }
  read(fd,buff,BUFFSIZE-1);
  *intr = 0; 
  *ciow = 0;  /* not separated out until the 2.5.41 kernel */

  b = strstr(buff, "cpu ");
  if(b) sscanf(b,  "cpu  %Lu %Lu %Lu %Lu %Lu", cuse, cice, csys, cide, ciow);

  b = strstr(buff, "page ");
  if(b) sscanf(b,  "page %u %u", pin, pout);
  else need_vmstat_file = 1;

  b = strstr(buff, "swap ");
  if(b) sscanf(b,  "swap %u %u", s_in, sout);
  else need_vmstat_file = 1;

  b = strstr(buff, "intr ");
  if(b) sscanf(b,  "intr %u", intr);

  b = strstr(buff, "ctxt ");
  if(b) sscanf(b,  "ctxt %u", ctxt);

  b = strstr(buff, "btime ");
  if(b) sscanf(b,  "btime %u", btime);

  b = strstr(buff, "processes ");
  if(b) sscanf(b,  "processes %u", processes);

  b = strstr(buff, "procs_running ");
  if(b) sscanf(b,  "procs_running %u", running);
  else need_proc_scan = 1;

  b = strstr(buff, "procs_blocked ");
  if(b) sscanf(b,  "procs_blocked %u", blocked);
  else need_proc_scan = 1;

  if(need_proc_scan){   /* Linux 2.5.46 (approximately) and below */
    getrunners(running, blocked);
  }

  (*running)--;   // exclude vmstat itself

  if(need_vmstat_file){  /* Linux 2.5.40-bk4 and above */
    vminfo();
    *pin  = vm_pgpgin;
    *pout = vm_pgpgout;
    *s_in = vm_pswpin;
    *sout = vm_pswpout;
  }
}


//////////////////////////////////////////////////////////////////////////////////////

#if 0
// produce:  "  6  ", "123  ", "123k ", etc.
static int format_1024(unsigned long long val64, char *restrict dst){
  unsigned oldval;
  const char suffix[] = " kmgtpe";
  unsigned level = 0;
  unsigned val32;

  if(val64 < 1000){   // special case to avoid "6.0  " when plain "  6  " would do
    val32 = val64;
    return sprintf(dst,"%3u  ",val32);
  }

  while(val64 > 0xffffffffull){
    level++;
    val64 /= 1024;
  }

  val32 = val64;

  while(val32 > 999){
    level++;
    oldval = val32;
    val32 /= 1024;
  }

  if(val32 < 10){
    unsigned fract = (oldval % 1024) * 10 / 1024;
    return sprintf(dst, "%u.%u%c ", val32, fract, suffix[level]);
  }
  return sprintf(dst, "%3u%c ", val32, suffix[level]);
}


// produce:  "  6  ", "123  ", "123k ", etc.
static int format_1000(unsigned long long val64, char *restrict dst){
  unsigned oldval;
  const char suffix[] = " kmgtpe";
  unsigned level = 0;
  unsigned val32;

  if(val64 < 1000){   // special case to avoid "6.0  " when plain "  6  " would do
    val32 = val64;
    return sprintf(dst,"%3u  ",val32);
  }

  while(val64 > 0xffffffffull){
    level++;
    val64 /= 1000;
  }

  val32 = val64;

  while(val32 > 999){
    level++;
    oldval = val32;
    val32 /= 1000;
  }

  if(val32 < 10){
    unsigned fract = (oldval % 1000) / 100;
    return sprintf(dst, "%u.%u%c ", val32, fract, suffix[level]);
  }
  return sprintf(dst, "%3u%c ", val32, suffix[level]);
}
#endif

static void new_header(void){
  printf("procs -----------memory---------- ---swap-- -----io---- --system-- ----cpu----\n");
  printf(
    "%2s %2s %6s %6s %6s %6s %4s %4s %5s %5s %4s %5s %2s %2s %2s %2s\n",
    "r","b",
    "swpd", "free", a_option?"inact":"buff", a_option?"active":"cache",
    "si","so",
    "bi","bo",
    "in","cs",
    "us","sy","id","wa"
  );
}

static void new_format(void) {
  const char format[]="%2u %2u %6u %6u %6u %6u %4u %4u %5u %5u %4u %5u %2u %2u %2u %2u\n";
  unsigned int tog=0; /* toggle switch for cleaner code */
  unsigned int i;
  unsigned int hz = Hertz;
  unsigned int running,blocked,dummy_1,dummy_2;
  jiff cpu_use[2], cpu_nic[2], cpu_sys[2], cpu_idl[2], cpu_iow[2];
  jiff duse, dsys, didl, diow, Div, divo2;
  unsigned int pgpgin[2], pgpgout[2], pswpin[2], pswpout[2];
  unsigned int intr[2], ctxt[2];
  unsigned int sleep_half; 
  unsigned int kb_per_page = sysconf(_SC_PAGESIZE) / 1024;
  int debt = 0;  // handle idle ticks running backwards

  sleep_half=(sleep_time/2);
  new_header();

  meminfo();
  getstat(cpu_use,cpu_nic,cpu_sys,cpu_idl,cpu_iow,
	  pgpgin,pgpgout,pswpin,pswpout,
	  intr,ctxt,
	  &running,&blocked,
	  &dummy_1, &dummy_2);
  duse= *cpu_use + *cpu_nic; 
  dsys= *cpu_sys;
  didl= *cpu_idl;
  diow= *cpu_iow;
  Div= duse+dsys+didl+diow;
  divo2= Div/2UL;
  printf(format,
	 running, blocked,
	 kb_swap_used, kb_main_free,
	 a_option?kb_inactive:kb_main_buffers,
	 a_option?kb_active:kb_main_cached,
	 (unsigned)( (*pswpin  * kb_per_page * hz + divo2) / Div ),
	 (unsigned)( (*pswpout * kb_per_page * hz + divo2) / Div ),
	 (unsigned)( (*pgpgin                * hz + divo2) / Div ),
	 (unsigned)( (*pgpgout               * hz + divo2) / Div ),
	 (unsigned)( (*intr                  * hz + divo2) / Div ),
	 (unsigned)( (*ctxt                  * hz + divo2) / Div ),
	 (unsigned)( (100*duse                    + divo2) / Div ),
	 (unsigned)( (100*dsys                    + divo2) / Div ),
	 (unsigned)( (100*didl                    + divo2) / Div ),
	 (unsigned)( (100*diow                    + divo2) / Div )
  );

  for(i=1;i<num_updates;i++) { /* \\\\\\\\\\\\\\\\\\\\ main loop ////////////////// */
    sleep(sleep_time);
    if (moreheaders && ((i%height)==0)) new_header();
    tog= !tog;

    meminfo();
    getstat(cpu_use+tog,cpu_nic+tog,cpu_sys+tog,cpu_idl+tog,cpu_iow+tog,
	  pgpgin+tog,pgpgout+tog,pswpin+tog,pswpout+tog,
	  intr+tog,ctxt+tog,
	  &running,&blocked,
	  &dummy_1,&dummy_2);
    duse= cpu_use[tog]-cpu_use[!tog] + cpu_nic[tog]-cpu_nic[!tog];
    dsys= cpu_sys[tog]-cpu_sys[!tog];
    didl= cpu_idl[tog]-cpu_idl[!tog];
    diow= cpu_iow[tog]-cpu_iow[!tog];

    /* idle can run backwards for a moment -- kernel "feature" */
    if(debt){
      didl = (int)didl + debt;
      debt = 0;
    }
    if( (int)didl < 0 ){
      debt = (int)didl;
      didl = 0;
    }

    Div= duse+dsys+didl+diow;
    divo2= Div/2UL;
    printf(format,
           running, blocked,
	   kb_swap_used,kb_main_free,
	   a_option?kb_inactive:kb_main_buffers,
	   a_option?kb_active:kb_main_cached,
	   (unsigned)( ( (pswpin [tog] - pswpin [!tog])*kb_per_page+sleep_half )/sleep_time ),
	   (unsigned)( ( (pswpout[tog] - pswpout[!tog])*kb_per_page+sleep_half )/sleep_time ),
	   (unsigned)( (  pgpgin [tog] - pgpgin [!tog]             +sleep_half )/sleep_time ),
	   (unsigned)( (  pgpgout[tog] - pgpgout[!tog]             +sleep_half )/sleep_time ),
	   (unsigned)( (  intr   [tog] - intr   [!tog]             +sleep_half )/sleep_time ),
	   (unsigned)( (  ctxt   [tog] - ctxt   [!tog]             +sleep_half )/sleep_time ),
	   (unsigned)( (100*duse+divo2)/Div ),
	   (unsigned)( (100*dsys+divo2)/Div ),
	   (unsigned)( (100*didl+divo2)/Div ),
	   (unsigned)( (100*diow+divo2)/Div )
    );
  }
}


//////////////////////////////////////////////////////////////////////////////////////

static void sum_format(void) {
  unsigned int running, blocked, btime, processes;
  jiff cpu_use, cpu_nic, cpu_sys, cpu_idl, cpu_iow;
  unsigned int pgpgin, pgpgout, pswpin, pswpout;
  unsigned int intr, ctxt;

  meminfo();
  getstat(&cpu_use, &cpu_nic, &cpu_sys, &cpu_idl, &cpu_iow,
	  &pgpgin, &pgpgout, &pswpin, &pswpout,
	  &intr, &ctxt,
	  &running, &blocked,
	  &btime, &processes);
  printf("%13u kB total memory\n", kb_main_total);
  printf("%13u kB used memory\n", kb_main_used);
  printf("%13u kB active memory\n", kb_active);
  printf("%13u kB inactive memory\n", kb_inactive);
  printf("%13u kB free memory\n", kb_main_free);
  printf("%13u kB buffer memory\n", kb_main_buffers);
  printf("%13u kB swap cache\n", kb_main_cached);
  printf("%13u kB total swap\n", kb_swap_total);
  printf("%13u kB used swap\n", kb_swap_used);
  printf("%13u kB free swap\n", kb_swap_free);
  printf("%13Lu non-nice user cpu ticks\n", cpu_use);
  printf("%13Lu nice user cpu ticks\n", cpu_nic);
  printf("%13Lu system cpu ticks\n", cpu_sys);
  printf("%13Lu idle cpu ticks\n", cpu_idl);
  printf("%13Lu IO-wait cpu ticks\n", cpu_iow);
  printf("%13u pages paged in\n", pgpgin);
  printf("%13u pages paged out\n", pgpgout);
  printf("%13u pages swapped in\n", pswpin);
  printf("%13u pages swapped out\n", pswpout);
  printf("%13u interrupts\n", intr);
  printf("%13u CPU context switches\n", ctxt);
  printf("%13u boot time\n", btime);
  printf("%13u forks\n", processes);
}

static void fork_format(void) {
  unsigned int running, blocked, btime, processes;
  jiff cpu_use, cpu_nic, cpu_sys, cpu_idl, cpu_iow;
  unsigned int pgpgin, pgpgout, pswpin, pswpout;
  unsigned int intr, ctxt;

  getstat(&cpu_use, &cpu_nic, &cpu_sys, &cpu_idl, &cpu_iow,
	  &pgpgin, &pgpgout, &pswpin, &pswpout,
	  &intr, &ctxt,
	  &running, &blocked,
	  &btime, &processes);
  printf("%13u forks\n", processes);
}


static int winhi(void) {
    struct winsize win;
    int rows = 24;
 
    if (ioctl(1, TIOCGWINSZ, &win) != -1 && win.ws_row > 0)
      rows = win.ws_row;
 
    return rows;
}


int main(int argc, char *argv[]) {
  argc=0; /* redefined as number of integer arguments */
  for (argv++;*argv;argv++) {
    if ('-' ==(**argv)) {
      switch (*(++(*argv))) {
      case 'V':
	display_version();
	exit(0);
      case 'a':
	/* active/inactive mode */
	a_option=1;
        break;
      case 'f':
        // FIXME: check for conflicting args
	fork_format();
        exit(0);
      case 'n':
	/* print only one header */
	moreheaders=FALSE;
        break;
      case 's':
        // FIXME: check for conflicting args
	sum_format();
        exit(0);
      default:
	/* no other aguments defined yet. */
	usage();
      }
    } else {
      argc++;
      switch (argc) {
      case 1:
        if ((sleep_time = atoi(*argv)) == 0)
         usage();
       num_updates = ULONG_MAX;
       break;
      case 2:
        num_updates = atol(*argv);
       break;
      default:
       usage();
      } /* switch */
    }
  }

  if (moreheaders) {
      int tmp=winhi()-3;
      height=((tmp>0)?tmp:22);
  }    

  setlinebuf(stdout);

  new_format();
  return 0;
}


