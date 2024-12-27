// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) Linux Test Project, 2016-2024
 * Alexey Kodanev <alexey.kodanev@oracle.com>
 */

#ifndef TST_MODULE_H
#define TST_MODULE_H

#include <stdbool.h>

void tst_module_exists_(void (cleanup_fn)(void), const char *mod_name,
					 char **mod_path);

static inline void tst_module_exists(const char *mod_name, char **mod_path)
{
	tst_module_exists_(NULL, mod_name, mod_path);
}

void tst_module_load_(void (cleanup_fn)(void), const char *mod_name,
					char *const argv[]);

static inline void tst_module_load(const char *mod_name, char *const argv[])
{
	tst_module_load_(NULL, mod_name, argv);
}

void tst_module_unload_(void (cleanup_fn)(void), const char *mod_name);

static inline void tst_module_unload(const char *mod_name)
{
	tst_module_unload_(NULL, mod_name);
}

bool tst_module_signature_enforced_(void);

static inline bool tst_module_signature_enforced(void)
{
	return tst_module_signature_enforced_();
}

void tst_requires_module_signature_disabled_(void);

static inline void tst_requires_module_signature_disabled(void)
{
	tst_requires_module_signature_disabled_();
}

#endif /* TST_MODULE_H */
