// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Cyril Hrubis <chrubis@suse.cz>
 */
#include <sys/mount.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "ujson.h"

static char *shell_filename;

static void run_shell(void)
{
	tst_run_script(shell_filename, NULL);
}

static void run_shell_tcnt(unsigned int n)
{
	char buf[128];
	char *const params[] = {buf, NULL};

	snprintf(buf, sizeof(buf), "%u", n);

	tst_run_script(shell_filename, params);
}

static struct tst_test test = {
	.runs_script = 1,
};

static void print_help(void)
{
	printf("Usage: tst_shell_loader ltp_shell_test.sh ...\n");
}

static char *metadata;
static size_t metadata_size;
static size_t metadata_used;

static void metadata_append(const char *line)
{
	size_t linelen = strlen(line);

	if (metadata_size - metadata_used < linelen + 1) {
		metadata_size += 4096;
		metadata = SAFE_REALLOC(metadata, metadata_size);
	}

	strcpy(metadata + metadata_used, line);
	metadata_used += linelen;
}

enum test_attr_ids {
	ALL_FILESYSTEMS,
	DEV_MIN_SIZE,
	FILESYSTEMS,
	FORMAT_DEVICE,
	MIN_CPUS,
	MIN_MEM_AVAIL,
	MIN_KVER,
	MIN_RUNTIME,
	MIN_SWAP_AVAIL,
	MNTPOINT,
	MOUNT_DEVICE,
	NEEDS_ABI_BITS,
	NEEDS_CMDS,
	NEEDS_DEVFS,
	NEEDS_DEVICE,
	NEEDS_DRIVERS,
	NEEDS_HUGETLBFS,
	NEEDS_KCONFIGS,
	NEEDS_ROFS,
	NEEDS_ROOT,
	NEEDS_TMPDIR,
	RESTORE_WALLCLOCK,
	RUNTIME,
	SAVE_RESTORE,
	SKIP_FILESYSTEMS,
	SKIP_IN_COMPAT,
	SKIP_IN_LOCKDOWN,
	SKIP_IN_SECUREBOOT,
	SUPPORTED_ARCHS,
	TAGS,
	TAINT_CHECK,
	TCNT,
};

static ujson_obj_attr test_attrs[] = {
	UJSON_OBJ_ATTR_IDX(ALL_FILESYSTEMS, "all_filesystems", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(DEV_MIN_SIZE, "dev_min_size", UJSON_INT),
	UJSON_OBJ_ATTR_IDX(FILESYSTEMS, "filesystems", UJSON_ARR),
	UJSON_OBJ_ATTR_IDX(FORMAT_DEVICE, "format_device", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(MIN_CPUS, "min_cpus", UJSON_INT),
	UJSON_OBJ_ATTR_IDX(MIN_MEM_AVAIL, "min_mem_avail", UJSON_INT),
	UJSON_OBJ_ATTR_IDX(MIN_KVER, "min_kver", UJSON_STR),
	UJSON_OBJ_ATTR_IDX(MIN_RUNTIME, "min_runtime", UJSON_INT),
	UJSON_OBJ_ATTR_IDX(MIN_SWAP_AVAIL, "min_swap_avail", UJSON_INT),
	UJSON_OBJ_ATTR_IDX(MNTPOINT, "mntpoint", UJSON_STR),
	UJSON_OBJ_ATTR_IDX(MOUNT_DEVICE, "mount_device", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(NEEDS_ABI_BITS, "needs_abi_bits", UJSON_INT),
	UJSON_OBJ_ATTR_IDX(NEEDS_CMDS, "needs_cmds", UJSON_ARR),
	UJSON_OBJ_ATTR_IDX(NEEDS_DEVFS, "needs_devfs", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(NEEDS_DEVICE, "needs_device", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(NEEDS_DRIVERS, "needs_drivers", UJSON_ARR),
	UJSON_OBJ_ATTR_IDX(NEEDS_HUGETLBFS, "needs_hugetlbfs", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(NEEDS_KCONFIGS, "needs_kconfigs", UJSON_ARR),
	UJSON_OBJ_ATTR_IDX(NEEDS_ROFS, "needs_rofs", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(NEEDS_ROOT, "needs_root", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(NEEDS_TMPDIR, "needs_tmpdir", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(RESTORE_WALLCLOCK, "restore_wallclock", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(RUNTIME, "runtime", UJSON_INT),
	UJSON_OBJ_ATTR_IDX(SAVE_RESTORE, "save_restore", UJSON_ARR),
	UJSON_OBJ_ATTR_IDX(SKIP_FILESYSTEMS, "skip_filesystems", UJSON_ARR),
	UJSON_OBJ_ATTR_IDX(SKIP_IN_COMPAT, "skip_in_compat", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(SKIP_IN_LOCKDOWN, "skip_in_lockdown", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(SKIP_IN_SECUREBOOT, "skip_in_secureboot", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(SUPPORTED_ARCHS, "supported_archs", UJSON_ARR),
	UJSON_OBJ_ATTR_IDX(TAGS, "tags", UJSON_ARR),
	UJSON_OBJ_ATTR_IDX(TAINT_CHECK, "taint_check", UJSON_BOOL),
	UJSON_OBJ_ATTR_IDX(TCNT, "tcnt", UJSON_INT)
};

static ujson_obj test_obj = {
	.attrs = test_attrs,
	.attr_cnt = UJSON_ARRAY_SIZE(test_attrs),
};

static const char *const *parse_strarr(ujson_reader *reader, ujson_val *val)
{
	unsigned int cnt = 0, i = 0;
	char **ret;

	ujson_reader_state state = ujson_reader_state_save(reader);

	UJSON_ARR_FOREACH(reader, val) {
		if (val->type != UJSON_STR) {
			ujson_err(reader, "Expected string!");
			return NULL;
		}

		cnt++;
	}

	ujson_reader_state_load(reader, state);

	ret = SAFE_MALLOC(sizeof(char *) * (cnt + 1));

	UJSON_ARR_FOREACH(reader, val) {
		ret[i++] = strdup(val->val_str);
	}

	ret[i] = NULL;

	return (const char *const *)ret;
}

enum fs_ids {
	FS_MIN_KVER,
	MKFS_OPTS,
	MKFS_SIZE_OPT,
	MKFS_VER,
	MNT_FLAGS,
	TYPE,
};

static ujson_obj_attr fs_attrs[] = {
	UJSON_OBJ_ATTR_IDX(FS_MIN_KVER, "min_kver", UJSON_STR),
	UJSON_OBJ_ATTR_IDX(MKFS_OPTS, "mkfs_opts", UJSON_ARR),
	UJSON_OBJ_ATTR_IDX(MKFS_SIZE_OPT, "mkfs_size_opt", UJSON_STR),
	UJSON_OBJ_ATTR_IDX(MKFS_VER, "mkfs_ver", UJSON_STR),
	UJSON_OBJ_ATTR_IDX(MNT_FLAGS, "mnt_flags", UJSON_ARR),
	UJSON_OBJ_ATTR_IDX(TYPE, "type", UJSON_STR),
};

static ujson_obj fs_obj = {
	.attrs = fs_attrs,
	.attr_cnt = UJSON_ARRAY_SIZE(fs_attrs),
};

static int parse_mnt_flags(ujson_reader *reader, ujson_val *val)
{
	int ret = 0;

	UJSON_ARR_FOREACH(reader, val) {
		if (val->type != UJSON_STR) {
			ujson_err(reader, "Expected string!");
			return ret;
		}

		if (!strcmp(val->val_str, "RDONLY"))
			ret |= MS_RDONLY;
		else if (!strcmp(val->val_str, "NOATIME"))
			ret |= MS_NOATIME;
		else if (!strcmp(val->val_str, "NOEXEC"))
			ret |= MS_NOEXEC;
		else if (!strcmp(val->val_str, "NOSUID"))
			ret |= MS_NOSUID;
		else
			ujson_err(reader, "Invalid mount flag");
	}

	return ret;
}

static struct tst_fs *parse_filesystems(ujson_reader *reader, ujson_val *val)
{
	unsigned int i = 0, cnt = 0;
	struct tst_fs *ret;

	ujson_reader_state state = ujson_reader_state_save(reader);

	UJSON_ARR_FOREACH(reader, val) {
		if (val->type != UJSON_OBJ) {
			ujson_err(reader, "Expected object!");
			return NULL;
		}
		ujson_obj_skip(reader);
		cnt++;
	}

	ujson_reader_state_load(reader, state);

	ret = SAFE_MALLOC(sizeof(struct tst_fs) * (cnt + 1));
	memset(ret, 0, sizeof(*ret) * (cnt+1));

	UJSON_ARR_FOREACH(reader, val) {
		UJSON_OBJ_FOREACH_FILTER(reader, val, &fs_obj, ujson_empty_obj) {
			switch ((enum fs_ids)val->idx) {
			case MKFS_OPTS:
				ret[i].mkfs_opts = parse_strarr(reader, val);
			break;
			case MKFS_SIZE_OPT:
				ret[i].mkfs_size_opt = strdup(val->val_str);
			break;
			case MKFS_VER:
				ret[i].mkfs_ver = strdup(val->val_str);
			break;
			case MNT_FLAGS:
				ret[i].mnt_flags = parse_mnt_flags(reader, val);
			break;
			case TYPE:
				ret[i].type = strdup(val->val_str);
			break;
			case FS_MIN_KVER:
				ret[i].min_kver = strdup(val->val_str);
			break;
			}

		}

		i++;
	}

	return ret;
}

static struct tst_tag *parse_tags(ujson_reader *reader, ujson_val *val)
{
	unsigned int i = 0, cnt = 0;
	struct tst_tag *ret;

	ujson_reader_state state = ujson_reader_state_save(reader);

	UJSON_ARR_FOREACH(reader, val) {
		if (val->type != UJSON_ARR) {
			ujson_err(reader, "Expected array!");
			return NULL;
		}
		ujson_arr_skip(reader);
		cnt++;
	}

	ujson_reader_state_load(reader, state);

	ret = SAFE_MALLOC(sizeof(struct tst_tag) * (cnt + 1));
	memset(&ret[cnt], 0, sizeof(ret[cnt]));

	UJSON_ARR_FOREACH(reader, val) {
		char *name = NULL;
		char *value = NULL;

		UJSON_ARR_FOREACH(reader, val) {
			if (val->type != UJSON_STR) {
				ujson_err(reader, "Expected string!");
				return NULL;
			}

			if (!name) {
				name = strdup(val->val_str);
			} else if (!value) {
				value = strdup(val->val_str);
			} else {
				ujson_err(reader, "Expected only two members!");
				return NULL;
			}
		}

		ret[i].name = name;
		ret[i].value = value;
		i++;
	}

	return ret;
}

static struct tst_path_val *parse_save_restore(ujson_reader *reader, ujson_val *val)
{
	unsigned int i = 0, cnt = 0;
	struct tst_path_val *ret;

	ujson_reader_state state = ujson_reader_state_save(reader);

	UJSON_ARR_FOREACH(reader, val) {
		if (val->type != UJSON_ARR) {
			ujson_err(reader, "Expected array!");
			return NULL;
		}
		ujson_arr_skip(reader);
		cnt++;
	}

	ujson_reader_state_load(reader, state);

	ret = SAFE_MALLOC(sizeof(struct tst_path_val) * (cnt + 1));
	memset(&ret[cnt], 0, sizeof(ret[cnt]));

	UJSON_ARR_FOREACH(reader, val) {
		char *path = NULL;
		char *pval = NULL;
		int flags_set = 0;
		int val_set = 0;
		unsigned int flags = 0;

		UJSON_ARR_FOREACH(reader, val) {
			if (!path) {
				if (val->type != UJSON_STR) {
					ujson_err(reader, "Expected string!");
					return NULL;
				}

				path = strdup(val->val_str);
			} else if (!val_set) {
				if (val->type == UJSON_STR) {
					pval = strdup(val->val_str);
				} else if (val->type != UJSON_NULL) {
					ujson_err(reader, "Expected string or NULL!");
					return NULL;
				}
				val_set = 1;
			} else if (!flags_set) {
				if (val->type != UJSON_STR) {
					ujson_err(reader, "Expected string!");
					return NULL;
				}

				if (!strcmp(val->val_str, "TCONF")) {
					flags = TST_SR_TCONF;
				} else if (!strcmp(val->val_str, "TBROK")) {
					flags = TST_SR_TBROK;
				} else if (!strcmp(val->val_str, "SKIP")) {
					flags = TST_SR_SKIP;
				} else {
					ujson_err(reader, "Invalid flags!");
					return NULL;
				}

				flags_set = 1;
			} else {
				ujson_err(reader, "Expected only two members!");
				return NULL;
			}
		}

		if (!path || !flags_set) {
			ujson_err(reader, "Expected [\"/{proc,sys}/path\", {\"TCONF\", \"TBROK\", \"TSKIP\"}]!");
			return NULL;
		}

		ret[i].path = path;
		ret[i].val = pval;
		ret[i].flags = flags;
		i++;
	}

	return ret;
}

static void parse_metadata(void)
{
	ujson_reader reader = UJSON_READER_INIT(metadata, metadata_used, UJSON_READER_STRICT);
	char str_buf[128];
	ujson_val val = UJSON_VAL_INIT(str_buf, sizeof(str_buf));

	UJSON_OBJ_FOREACH_FILTER(&reader, &val, &test_obj, ujson_empty_obj) {
		switch ((enum test_attr_ids)val.idx) {
		case ALL_FILESYSTEMS:
			test.all_filesystems = val.val_bool;
		break;
		case DEV_MIN_SIZE:
			if (val.val_int <= 0)
				ujson_err(&reader, "Device size must be > 0");
			else
				test.dev_min_size = val.val_int;
		break;
		case FILESYSTEMS:
			test.filesystems = parse_filesystems(&reader, &val);
		break;
		case FORMAT_DEVICE:
			test.format_device = val.val_bool;
		break;
		case MIN_CPUS:
			if (val.val_int <= 0)
				ujson_err(&reader, "Minimal number of cpus must be > 0");
			else
				test.min_cpus = val.val_int;
		break;
		case MIN_MEM_AVAIL:
			if (val.val_int <= 0)
				ujson_err(&reader, "Minimal available memory size must be > 0");
			else
				test.min_mem_avail = val.val_int;
		break;
		case MIN_KVER:
			test.min_kver = strdup(val.val_str);
		break;
		case MIN_RUNTIME:
			if (val.val_int <= 0)
				ujson_err(&reader, "Minimal runtime must be > 0");
			else
				test.min_runtime = val.val_int;
		break;
		case MIN_SWAP_AVAIL:
			if (val.val_int <= 0)
				ujson_err(&reader, "Minimal available swap size must be > 0");
			else
				test.min_swap_avail = val.val_int;
		break;
		case MNTPOINT:
			test.mntpoint = strdup(val.val_str);
		break;
		case MOUNT_DEVICE:
			test.mount_device = val.val_bool;
		break;
		case NEEDS_ABI_BITS:
			if (val.val_int == 32 || val.val_int == 64)
				test.needs_abi_bits = val.val_int;
			else
				ujson_err(&reader, "ABI bits must be 32 or 64");
		break;
		case NEEDS_CMDS:
			test.needs_cmds = parse_strarr(&reader, &val);
		break;
		case NEEDS_DEVFS:
			test.needs_devfs = val.val_bool;
		break;
		case NEEDS_DEVICE:
			test.needs_device = val.val_bool;
		break;
		case NEEDS_DRIVERS:
			test.needs_drivers = parse_strarr(&reader, &val);
		break;
		case NEEDS_HUGETLBFS:
			test.needs_hugetlbfs = val.val_bool;
		break;
		case NEEDS_KCONFIGS:
			test.needs_kconfigs = parse_strarr(&reader, &val);
		break;
		case NEEDS_ROFS:
			test.needs_rofs = val.val_bool;
		break;
		case NEEDS_ROOT:
			test.needs_root = val.val_bool;
		break;
		case NEEDS_TMPDIR:
			test.needs_tmpdir = val.val_bool;
		break;
		case RESTORE_WALLCLOCK:
			test.restore_wallclock = val.val_bool;
		break;
		case RUNTIME:
			if (val.val_int <= 0)
				ujson_err(&reader, "Runtime must be > 0");
			else
				test.runtime = val.val_int;
		break;
		case SAVE_RESTORE:
			test.save_restore = parse_save_restore(&reader, &val);
		break;
		case SKIP_FILESYSTEMS:
			test.skip_filesystems = parse_strarr(&reader, &val);
		break;
		case SKIP_IN_COMPAT:
			test.skip_in_compat = val.val_bool;
		break;
		case SKIP_IN_LOCKDOWN:
			test.skip_in_lockdown = val.val_bool;
		break;
		case SKIP_IN_SECUREBOOT:
			test.skip_in_secureboot = val.val_bool;
		break;
		case SUPPORTED_ARCHS:
			test.supported_archs = parse_strarr(&reader, &val);
		break;
		case TAGS:
			test.tags = parse_tags(&reader, &val);
		break;
		case TAINT_CHECK:
			test.taint_check = val.val_bool;
		break;
		case TCNT:
			if (val.val_int <= 0)
				ujson_err(&reader, "Number of tests must be > 0");
			else
				test.tcnt = val.val_int;
		break;
		}
	}

	ujson_reader_finish(&reader);

	if (ujson_reader_err(&reader))
		tst_brk(TBROK, "Invalid metadata");
}

enum parser_state {
	PAR_NONE,
	PAR_ESC,
	PAR_DOC,
	PAR_ENV,
};

static void extract_metadata(void)
{
	FILE *f;
	char line[4096];
	char path[4096];
	enum parser_state state = PAR_NONE;
	unsigned int lineno = 1;

	if (tst_get_path(shell_filename, path, sizeof(path)) == -1)
		tst_brk(TBROK, "Failed to find %s in $PATH", shell_filename);

	f = SAFE_FOPEN(path, "r");

	while (fgets(line, sizeof(line), f)) {
		switch (state) {
		case PAR_NONE:
			if (!strcmp(line, "# ---\n"))
				state = PAR_ESC;
		break;
		case PAR_ESC:
			if (!strcmp(line, "# env\n")) {
				state = PAR_ENV;
			} else if (!strcmp(line, "# doc\n")) {
				state = PAR_DOC;
			} else {
				tst_brk(TBROK, "%s: %u: Unknown comment block %s",
				        path, lineno, line);
			}
		break;
		case PAR_ENV:
			if (line[0] != '#') {
				tst_brk(TBROK,
					"%s: %u: Unexpected end of comment block!",
					path, lineno);
			}

			if (!strcmp(line, "# ---\n"))
				state = PAR_NONE;
			else
				metadata_append(line + 2);
		break;
		case PAR_DOC:
			if (line[0] != '#') {
				tst_brk(TBROK,
					"%s: %u: Unexpected end of comment block!",
					path, lineno);
			}

			if (!strcmp(line, "# ---\n"))
				state = PAR_NONE;
		break;
		}

		lineno++;
	}

	fclose(f);
}

static void prepare_test_struct(void)
{
	extract_metadata();

	if (metadata)
		parse_metadata();
	else
		tst_brk(TBROK, "No metadata found!");
}

int main(int argc, char *argv[])
{
	if (argc < 2)
		goto help;

	shell_filename = argv[1];

	prepare_test_struct();

	if (test.tcnt)
		test.test = run_shell_tcnt;
	else
		test.test_all = run_shell;

	tst_run_tcases(argc - 1, argv + 1, &test);
help:
	print_help();
	return 1;
}
