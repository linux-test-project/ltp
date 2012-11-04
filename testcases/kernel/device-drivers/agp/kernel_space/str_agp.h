/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Remember that you want to seperate your header
 * files between what is needed in kernel space
 * only, and what will also be needed by a user
 * space program that is using this module. For
 * that reason keep all structures that will need
 * kernel space pointers in a seperate header file
 * from where ioctl flags aer kept
 *
 * author: Kai Zhao
 * date:   08/25/2003
 *
 */

struct ltpmod_user {

//put any pointers in here that might be needed by other ioctl calls

};
typedef struct ltpmod_user ltpmod_user_t;
static struct agp_bridge_data	*tmp_bridge = NULL;
//static struct agp_memory	*tmp_agp_memory = NULL;





#include <asm/agp.h>	// for flush_agp_cache()

#define PFX "agpgart: "

extern struct agp_bridge_data *agp_bridge;

enum aper_size_type {
	U8_APER_SIZE,
	U16_APER_SIZE,
	U32_APER_SIZE,
	LVL2_APER_SIZE,
	FIXED_APER_SIZE
};

struct gatt_mask {
	unsigned long mask;
	u32 type;
	// totally device specific, for integrated chipsets that
	// might have different types of memory masks.  For other
	// devices this will probably be ignored
};

struct aper_size_info_8 {
	int size;
	int num_entries;
	int page_order;
	u8 size_value;
};

struct aper_size_info_16 {
	int size;
	int num_entries;
	int page_order;
	u16 size_value;
};

struct aper_size_info_32 {
	int size;
	int num_entries;
	int page_order;
	u32 size_value;
};

struct aper_size_info_lvl2 {
	int size;
	int num_entries;
	u32 size_value;
};

struct aper_size_info_fixed {
	int size;
	int num_entries;
	int page_order;
};

struct agp_bridge_driver {
	struct module *owner;
	void *aperture_sizes;
	int num_aperture_sizes;
	enum aper_size_type size_type;
	int cant_use_aperture;
	int needs_scratch_page;
	struct gatt_mask *masks;
	int (*fetch_size)(void);
	int (*configure)(void);
	void (*agp_enable)(u32);
	void (*cleanup)(void);
	void (*tlb_flush)(struct agp_memory *);
	unsigned long (*mask_memory)(unsigned long, int);
	void (*cache_flush)(void);
	int (*create_gatt_table)(void);
	int (*free_gatt_table)(void);
	int (*insert_memory)(struct agp_memory *, off_t, int);
	int (*remove_memory)(struct agp_memory *, off_t, int);
	struct agp_memory *(*alloc_by_type) (size_t, int);
	void (*free_by_type)(struct agp_memory *);
	void *(*agp_alloc_page)(void);
	void (*agp_destroy_page)(void *);
};

struct agp_bridge_data {
	struct agp_version *version;
	struct agp_bridge_driver *driver;
	struct vm_operations_struct *vm_ops;
	void *previous_size;
	void *current_size;
	void *dev_private_data;
	struct pci_dev *dev;
	u32 *gatt_table;
	u32 *gatt_table_real;
	unsigned long scratch_page;
	unsigned long scratch_page_real;
	unsigned long gart_bus_addr;
	unsigned long gatt_bus_addr;
	u32 mode;
	enum chipset_type type;
	unsigned long *key_list;
	atomic_t current_memory_agp;
	atomic_t agp_in_use;
	int max_memory_agp;	// in number of pages
	int aperture_size_idx;
	int capndx;
	char major_version;
	char minor_version;
};

#define OUTREG64(mmap, addr, val)	__raw_writeq((val), (mmap)+(addr))
#define OUTREG32(mmap, addr, val)	__raw_writel((val), (mmap)+(addr))
#define OUTREG16(mmap, addr, val)	__raw_writew((val), (mmap)+(addr))
#define OUTREG8(mmap, addr, val)	__raw_writeb((val), (mmap)+(addr))

#define INREG64(mmap, addr)		__raw_readq((mmap)+(addr))
#define INREG32(mmap, addr)		__raw_readl((mmap)+(addr))
#define INREG16(mmap, addr)		__raw_readw((mmap)+(addr))
#define INREG8(mmap, addr)		__raw_readb((mmap)+(addr))

#define KB(x)	((x) * 1024)
#define MB(x)	(KB (KB (x)))
#define GB(x)	(MB (KB (x)))

#define A_SIZE_8(x)	((struct aper_size_info_8 *) x)
#define A_SIZE_16(x)	((struct aper_size_info_16 *) x)
#define A_SIZE_32(x)	((struct aper_size_info_32 *) x)
#define A_SIZE_LVL2(x)	((struct aper_size_info_lvl2 *) x)
#define A_SIZE_FIX(x)	((struct aper_size_info_fixed *) x)
#define A_IDX8(bridge)	(A_SIZE_8((bridge)->driver->aperture_sizes) + i)
#define A_IDX16(bridge)	(A_SIZE_16((bridge)->driver->aperture_sizes) + i)
#define A_IDX32(bridge)	(A_SIZE_32((bridge)->driver->aperture_sizes) + i)
#define MAXKEY		(4096 * 32)

#define PGE_EMPTY(b, p)	(!(p) || (p) == (unsigned long) (b)->scratch_page)

// intel register
#define INTEL_APBASE	0x10
#define INTEL_APSIZE	0xb4
#define INTEL_ATTBASE	0xb8
#define INTEL_AGPCTRL	0xb0
#define INTEL_NBXCFG	0x50
#define INTEL_ERRSTS	0x91

// Intel 460GX Registers
#define INTEL_I460_APBASE		0x10
#define INTEL_I460_BAPBASE		0x98
#define INTEL_I460_GXBCTL		0xa0
#define INTEL_I460_AGPSIZ		0xa2
#define INTEL_I460_ATTBASE		0xfe200000
#define INTEL_I460_GATT_VALID		(1UL << 24)
#define INTEL_I460_GATT_COHERENT	(1UL << 25)

// intel i830 registers
#define I830_GMCH_CTRL			0x52
#define I830_GMCH_ENABLED		0x4
#define I830_GMCH_MEM_MASK		0x1
#define I830_GMCH_MEM_64M		0x1
#define I830_GMCH_MEM_128M		0
#define I830_GMCH_GMS_MASK		0x70
#define I830_GMCH_GMS_DISABLED		0x00
#define I830_GMCH_GMS_LOCAL		0x10
#define I830_GMCH_GMS_STOLEN_512	0x20
#define I830_GMCH_GMS_STOLEN_1024	0x30
#define I830_GMCH_GMS_STOLEN_8192	0x40
#define I830_RDRAM_CHANNEL_TYPE		0x03010
#define I830_RDRAM_ND(x)		(((x) & 0x20) >> 5)
#define I830_RDRAM_DDT(x)		(((x) & 0x18) >> 3)

// This one is for I830MP w. an external graphic card
#define INTEL_I830_ERRSTS	0x92

// Intel 855GM/852GM registers
#define I855_GMCH_GMS_STOLEN_0M		0x0
#define I855_GMCH_GMS_STOLEN_1M		(0x1 << 4)
#define I855_GMCH_GMS_STOLEN_4M		(0x2 << 4)
#define I855_GMCH_GMS_STOLEN_8M		(0x3 << 4)
#define I855_GMCH_GMS_STOLEN_16M	(0x4 << 4)
#define I855_GMCH_GMS_STOLEN_32M	(0x5 << 4)
#define I85X_CAPID			0x44
#define I85X_VARIANT_MASK		0x7
#define I85X_VARIANT_SHIFT		5
#define I855_GME			0x0
#define I855_GM				0x4
#define I852_GME			0x2
#define I852_GM				0x5

// intel 815 register
#define INTEL_815_APCONT	0x51
#define INTEL_815_ATTBASE_MASK	~0x1FFFFFFF

// intel i820 registers
#define INTEL_I820_RDCR		0x51
#define INTEL_I820_ERRSTS	0xc8

// intel i840 registers
#define INTEL_I840_MCHCFG	0x50
#define INTEL_I840_ERRSTS	0xc8

// intel i845 registers
#define INTEL_I845_AGPM		0x51
#define INTEL_I845_ERRSTS	0xc8

// intel i850 registers
#define INTEL_I850_MCHCFG	0x50
#define INTEL_I850_ERRSTS	0xc8

// intel i860 registers
#define INTEL_I860_MCHCFG	0x50
#define INTEL_I860_ERRSTS	0xc8

// intel i810 registers
#define I810_GMADDR		0x10
#define I810_MMADDR		0x14
#define I810_PTE_BASE		0x10000
#define I810_PTE_MAIN_UNCACHED	0x00000000
#define I810_PTE_LOCAL		0x00000002
#define I810_PTE_VALID		0x00000001
#define I810_SMRAM_MISCC	0x70
#define I810_GFX_MEM_WIN_SIZE	0x00010000
#define I810_GFX_MEM_WIN_32M	0x00010000
#define I810_GMS		0x000000c0
#define I810_GMS_DISABLE	0x00000000
#define I810_PGETBL_CTL		0x2020
#define I810_PGETBL_ENABLED	0x00000001
#define I810_DRAM_CTL		0x3000
#define I810_DRAM_ROW_0		0x00000001
#define I810_DRAM_ROW_0_SDRAM	0x00000001

// Intel 7505 registers
#define INTEL_I7505_NAPBASELO	0x10
#define INTEL_I7505_APSIZE	0x74
#define INTEL_I7505_NCAPID	0x60
#define INTEL_I7505_NISTAT	0x6c
#define INTEL_I7505_ATTBASE	0x78
#define INTEL_I7505_ERRSTS	0x42
#define INTEL_I7505_AGPCTRL	0x70
#define INTEL_I7505_MCHCFG	0x50

// VIA register
#define VIA_APBASE	0x10
#define VIA_GARTCTRL	0x80
#define VIA_APSIZE	0x84
#define VIA_ATTBASE	0x88

// VIA KT400
#define VIA_AGP3_GARTCTRL	0x90
#define VIA_AGP3_APSIZE	0x94
#define VIA_AGP3_ATTBASE	0x98
#define VIA_AGPSEL	0xfd

// SiS registers
#define SIS_APBASE	0x10
#define SIS_ATTBASE	0x90
#define SIS_APSIZE	0x94
#define SIS_TLBCNTRL	0x97
#define SIS_TLBFLUSH	0x98

// AMD registers
#define AMD_APBASE	0x10
#define AMD_MMBASE	0x14
#define AMD_APSIZE	0xac
#define AMD_MODECNTL	0xb0
#define AMD_MODECNTL2	0xb2
#define AMD_GARTENABLE	0x02	// In mmio region (16-bit register)
#define AMD_ATTBASE	0x04	// In mmio region (32-bit register)
#define AMD_TLBFLUSH	0x0c	// In mmio region (32-bit register)
#define AMD_CACHEENTRY	0x10	// In mmio region (32-bit register)

#define AMD_8151_APSIZE	0xb4
#define AMD_8151_GARTBLOCK	0xb8

#define AMD_X86_64_GARTAPERTURECTL	0x90
#define AMD_X86_64_GARTAPERTUREBASE	0x94
#define AMD_X86_64_GARTTABLEBASE	0x98
#define AMD_X86_64_GARTCACHECTL		0x9c
#define AMD_X86_64_GARTEN	1<<0

#define AMD_8151_VMAPERTURE		0x10
#define AMD_8151_AGP_CTL		0xb0
#define AMD_8151_APERTURESIZE	0xb4
#define AMD_8151_GARTPTR		0xb8
#define AMD_8151_GTLBEN	1<<7
#define AMD_8151_APEREN	1<<8

// ALi registers
#define ALI_APBASE			0x10
#define ALI_AGPCTRL			0xb8
#define ALI_ATTBASE			0xbc
#define ALI_TLBCTRL			0xc0
#define ALI_TAGCTRL			0xc4
#define ALI_CACHE_FLUSH_CTRL		0xD0
#define ALI_CACHE_FLUSH_ADDR_MASK	0xFFFFF000
#define ALI_CACHE_FLUSH_EN		0x100

// Serverworks Registers
#define SVWRKS_APSIZE		0x10
#define SVWRKS_SIZE_MASK	0xfe000000

#define SVWRKS_MMBASE		0x14
#define SVWRKS_CACHING		0x4b
#define SVWRKS_FEATURE		0x68

// func 1 registers
#define SVWRKS_AGP_ENABLE	0x60
#define SVWRKS_COMMAND		0x04

// Memory mapped registers
#define SVWRKS_GART_CACHE	0x02
#define SVWRKS_GATTBASE		0x04
#define SVWRKS_TLBFLUSH		0x10
#define SVWRKS_POSTFLUSH	0x14
#define SVWRKS_DIRFLUSH		0x0c

// HP ZX1 SBA registers
#define HP_ZX1_CTRL		0x200
#define HP_ZX1_IBASE		0x300
#define HP_ZX1_IMASK		0x308
#define HP_ZX1_PCOM		0x310
#define HP_ZX1_TCNFG		0x318
#define HP_ZX1_PDIR_BASE	0x320
#define HP_ZX1_CACHE_FLUSH	0x428

struct agp_device_ids {
	unsigned short device_id; // first, to make table easier to read
	enum chipset_type chipset;
	const char *chipset_name;
	int (*chipset_setup) (struct pci_dev *pdev);	// used to override generic
};

// Driver registration
struct agp_bridge_data *agp_alloc_bridge(void);
void agp_put_bridge(struct agp_bridge_data *bridge);
int agp_add_bridge(struct agp_bridge_data *bridge);
void agp_remove_bridge(struct agp_bridge_data *bridge);

// Frontend routines.
int agp_frontend_initialize(void);
void agp_frontend_cleanup(void);

// Generic routines.
void agp_generic_enable(u32 mode);
int agp_generic_create_gatt_table(void);
int agp_generic_free_gatt_table(void);
struct agp_memory *agp_create_memory(int scratch_pages);
int agp_generic_insert_memory(struct agp_memory *mem, off_t pg_start, int type);
int agp_generic_remove_memory(struct agp_memory *mem, off_t pg_start, int type);
struct agp_memory *agp_generic_alloc_by_type(size_t page_count, int type);
void agp_generic_free_by_type(struct agp_memory *curr);
void *agp_generic_alloc_page(void);
void agp_generic_destroy_page(void *addr);
void agp_free_key(int key);
int agp_num_entries(void);
u32 agp_collect_device_status(u32 mode, u32 command);
void agp_device_command(u32 command, int agp_v3);
int agp_3_5_enable(struct agp_bridge_data *bridge);
void global_cache_flush(void);
void get_agp_version(struct agp_bridge_data *bridge);
unsigned long agp_generic_mask_memory(unsigned long addr, int type);

// Standard agp registers
#define AGPSTAT			0x4
#define AGPCMD			0x8
#define AGPNISTAT		0xc
#define AGPNEPG			0x16
#define AGPNICMD		0x20

#define AGP_MAJOR_VERSION_SHIFT	(20)
#define AGP_MINOR_VERSION_SHIFT	(16)

#define AGPSTAT_RQ_DEPTH	(0xff000000)

#define AGPSTAT_CAL_MASK	(1<<12|1<<11|1<<10)
#define AGPSTAT_ARQSZ		(1<<15|1<<14|1<<13)
#define AGPSTAT_ARQSZ_SHIFT	13

#define AGPSTAT_SBA		(1<<9)
#define AGPSTAT_AGP_ENABLE	(1<<8)
#define AGPSTAT_FW		(1<<4)
#define AGPSTAT_MODE_3_0	(1<<3)

#define AGPSTAT2_1X		(1<<0)
#define AGPSTAT2_2X		(1<<1)
#define AGPSTAT2_4X		(1<<2)

#define AGPSTAT3_RSVD		(1<<2)
#define AGPSTAT3_8X		(1<<1)
#define AGPSTAT3_4X		(1)


