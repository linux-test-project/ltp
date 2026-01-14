// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) Linux Test Project, 2026
 */

#ifndef TST_MODULE
#define TST_MODULE

#include <stdbool.h>

void tst_module_exists_(void (cleanup_fn)(void), const char *mod_name,
					 char **mod_path);

void tst_module_load_(void (cleanup_fn)(void), const char *mod_name,
					char *const argv[]);

void tst_module_unload_(void (cleanup_fn)(void), const char *mod_name);

bool tst_module_signature_enforced_(void);
void tst_requires_module_signature_disabled_(void);

/*
 * Check module existence.
 *
 * @mod_name: module's file name.
 * @mod_path: if it is not NULL, then tst_module_exists places the found
 * module's path into the location pointed to by *mod_path. It must be freed
 * with free() when it is no longer needed.
 *
 * In case of failure, test'll call cleanup_fn and exit with TCONF return value.
 */
static inline void tst_module_exists(void (cleanup_fn)(void),
				     const char *mod_name, char **mod_path)
{
	tst_module_exists_(cleanup_fn, mod_name, mod_path);
}

/*
 * Load a module using insmod program.
 *
 * @mod_name: module's file name.
 * @argv: an array of pointers to null-terminated strings that represent the
 * additional parameters to the module. The array of pointers  must be
 * terminated by a NULL pointer. If argv points to NULL, it will be ignored.
 *
 * In case of insmod failure, test will call cleanup_fn and exit with TBROK
 * return value.
 */
static inline void tst_module_load(void (cleanup_fn)(void),
				   const char *mod_name, char *const argv[])
{
	tst_module_load_(cleanup_fn, mod_name, argv);
}

/*
 * Unload a module using rmmod program. In case of failure, test will call
 * cleanup_fn and exit with TBROK return value.
 *
 * @mod_name: can be module name or module's file name.
 */
static inline void tst_module_unload(void (cleanup_fn)(void), const char *mod_name)
{
	tst_module_unload_(cleanup_fn, mod_name);
}

/**
 * tst_requires_module_signature_disabled() - Check if enforced module signature.
 *
 * Module signature is enforced if module.sig_enforce=1 kernel parameter or
 * CONFIG_MODULE_SIG_FORCE=y.
 *
 * return: Returns true if module signature is enforced false otherwise.
 *
 */
static inline bool tst_module_signature_enforced(void)
{
	return tst_module_signature_enforced_();
}

/**
 * tst_requires_module_signature_disabled() - Check if test needs to be skipped due
 * enforced module signature.
 *
 * Skip test with tst_brk(TCONF) due module signature enforcement if
 * module.sig_enforce=1 kernel parameter or CONFIG_MODULE_SIG_FORCE=y.
 */

static inline void tst_requires_module_signature_disabled(void)
{
	tst_requires_module_signature_disabled_();
}

#endif /* TST_MODULE */
