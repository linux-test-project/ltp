/* 
 * Print table of MCA status bit combinations with results in HTML.
 * Author: Andi Kleen
 */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#define __KERNEL__ 1
#include <asm/types.h>
#include <asm/mce.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))

typedef unsigned long long u64;


#define MCI_STATUS_S	 (1ULL<<56)  /* Signaled machine check */
#define MCI_STATUS_AR	 (1ULL<<55)  /* Action required */

int tolerant = 1;
int panic_on_oops = 0;
int mce_ser = 1;

#include "mce-severity.c"

int disable_opt = 0;

struct rname { 
	char *name;
	unsigned color;
	char *desc;
} rnames[] = {
#define R(x,col,d) [MCE_ ## x ## _SEVERITY] = { #x, col, d }
	R(NO, 0xc0c0c0, "Ignored"),
	R(KEEP, 0x800080, "Ignore. Keep for CMC"),
	R(SOME, 0x808080, "Log & Clear"),
	R(AO, 0xffff00, "Kill address owner"),
	R(UC, 0x700000, "Kill or panic"),
	R(AR, 0x00ff00, "Kill current context"),
	R(PANIC, 0xff0000, "Shutdown"),
#undef R
};

struct bit { 
	char *name;
	unsigned offset;
	u64 bit;
} bits[] = { 
#define O(x) offsetof(struct mce, x)
#define S(x) { #x, O(status), MCI_STATUS_ ## x }
	{ "RIPV", O(mcgstatus), MCG_STATUS_RIPV },
	{ "EIPV", O(mcgstatus), MCG_STATUS_EIPV },
	{ "MCIP", O(mcgstatus), MCG_STATUS_MCIP },
	S(EN),
	S(VAL),
	S(UC),
	S(S),
	S(AR),
	S(PCC),
	S(OVER),
	{ "SCRB-ERR", O(status), 0xc0 },
#undef S
#undef O
};

struct mce basem;

#define bit_for_each(i,v) for (i = 0; i < 64; i++) if ((v) & (1ULL << i)) 

struct result { 
	int res;
	unsigned dontcare;
	char *msg;
};

void genstate(struct mce *m, unsigned num)
{
	int i;
	*m = basem;
	
	bit_for_each (i, num)
		*(u64 *)((char *)m + bits[i].offset) |= bits[i].bit;
}

// find don't care bits
// brute force version because andi is not clever enough to make the clever 
// version work. luckily the tables are small

#define for_rr(start, i) for (i = start; i < num; i++) if (rr[i].res >= 0) 
#define mask_of(x) ((1U << (x))-1)

static void disable(struct result *rr, int i, int src)
{
	//fprintf(stderr, "disabling %d from %d\n", i, src);
	rr[i].res = -1;
}

// handle case: one bit set always the same outcome
static void one_bit_all(struct result *rr, int num, int mask)
{
	int first, k;
	if (mask >= num)
		return;
	first = mask;
	for_rr (first, k) { 
		if (!(k & mask))
			continue;
		if (rr[k].res != rr[first].res)
			return;
	}
	rr[first].dontcare = mask_of(ARRAY_SIZE(bits)) & ~mask;
	for_rr (first + 1, k) { 
		if (k & mask)
			disable(rr, k, k);
	}
}

// check if toggling one bit gives the same outcome
static void neighbour_same(struct result *rr, int num, int mask)
{
	int k, other;
	for_rr (mask, k) { 
		if (!(k & mask) || (rr[k].dontcare & mask))
			continue;
		other = k ^ mask;
		if (other >= num)
			continue;
		if (rr[other].res == rr[k].res && rr[other].msg == rr[k].msg) { 
			disable(rr, other, k);
			rr[k].dontcare |= mask;
		}
	}
}

void optimizer(struct result *rr, int num)
{
	int i;

	for (i = 1; i <= 1 << ARRAY_SIZE(bits); i <<= 1)
		one_bit_all(rr, num, i);
	for (i = 1; i <= 1 << ARRAY_SIZE(bits); i <<= 1)
		neighbour_same(rr, num, i);
}

int bitcount(u64 v)
{
	int num = 0;
	while (v) {
		if (v & 1)
			num++;
		v >>= 1;
	}
	return num;
}

void table(char *title)
{
	struct mce m;
	int i, w, num; 
	
	struct result *rr = calloc(sizeof(struct result), 1U << ARRAY_SIZE(bits));

	num = 0;
	for (i = 0; i < 1U << ARRAY_SIZE(bits); i++) {
		genstate(&m, i);
		rr[num].res = mce_severity(&m, tolerant, &rr[num].msg);
		num++;
	}

	if (!disable_opt)
		optimizer(rr, num);
	
	printf("<p><table border=1>\n");
	printf("<chaption>%s</chaption>\n", title);

	printf("<tr>\n");	
	for (i = 0; i < ARRAY_SIZE(bits); i++) { 
		printf("<th>%s</th>", bits[i].name);
	}
	printf("<th>Result</th><th>Rule</th><th>Action</th>\n");
	printf("</tr>\n");

	for_rr (0, i) { 
		printf("<tr>");
		for (w = 0; w < ARRAY_SIZE(bits); w++) { 
			char *p = "0"; 
			char *col = "";
			unsigned mask = 1U << w;

			if (mask & rr[i].dontcare) {
				p = "x";
				col = " bgcolor=\"888888\"";
			} else if (mask & i) { 
				if (bitcount(bits[w].bit) > 1) 
					asprintf(&p, "%llx", bits[w].bit);
				else
					p = "1";
 				col = " bgcolor=\"ffffff\"";
			}
			printf("<td%s>%s</td>", col, p);
		}
		struct rname *rname = &rnames[rr[i].res];
		if ((unsigned)rr[i].res >= ARRAY_SIZE(rnames))
			rname = &((struct rname) { .name = "out of bounds", .color = 0xff00ff });
		assert(rname->name != NULL);
		printf("<td bgcolor=\"%06x\">%s</td>", rname->color, rname->name);
		assert(rr[i].msg != NULL);
		printf("<td>%s</td>", rr[i].msg);
		printf("<td>%s</td>", rname->desc);
		printf("</tr>\n");	
	}
	printf("</table>\n");
}

void usage(void)
{
	fprintf(stderr, "ttable [-a]\n"
			"-a don't print don't care bits, but all states\n");
	exit(1);
}

int main(int ac, char **av)
{
	int opt;
	while ((opt = getopt(ac, av, "a")) != -1) { 
		switch (opt) { 
		case 'a': 
			disable_opt = 1;
			break;
		default:
			usage();
		}
	}

	printf("<html><body>\n");
	printf("<!-- Auto generated. Changes will be overwritten -->\n");
	basem.ip = 1;
	printf("<h1>Linux kernel machine check grading</h1>\n");
	printf("Caveats: Only scrubber error AO MCACOD. Only applies to exceptions.\n");
	mce_ser = 1;
	basem.cs = 0;
	table("With MCA recovery ring 0");
	tolerant = 0;
	table("With MCA recovery ring 0 tolerant = 0");
	tolerant = 1;
	basem.cs = 3;
	table("With MCA recovery ring 3");
	basem.cs = 0;
	mce_ser = 0;
	table("Without MCA recovery ring 0");
	basem.cs = 3;
	table("Without MCA recovery ring 3");
	printf("</body></html>\n");
	return 0;
}
