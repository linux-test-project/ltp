/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define ACPI_TEST_NAME		"ltp_acpi_test"

enum ACPI_TEST_CASES {
	ACPI_INIT = 0,

	/*
	 * Test-case will stop traversing if it finds _STR.
	 * To continue, please trigger it again.
	 */
	ACPI_TRAVERSE,
	ACPI_NOTIFY_HANDLER,
	ACPI_EVENT_HANDLER,
	ACPI_GLOBAL_LOCK,
	ACPI_TEST_BUS,
	ACPI_TEST_RESOURCES,
	ACPI_SLEEP_TEST,
	ACPI_TEST_REGISTER,
	ACPI_TEST_DEV_CALLBACK,
	ACPI_TC_NUM
};
