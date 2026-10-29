#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2025 Cyril Hrubis <chrubis@suse.cz>
# Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@suse.com>
"""
This script parses JSON results from kirk and LTP metadata in order to
calculate timeouts for tests based on the results file.
It can also patch tests automatically and replace the calculated timeout.
"""

import re
import os
import json
import argparse

# The test runtime is multiplied by this to get a timeout
TIMEOUT_MUL = 1.2


def _sed(fname, expr, replace):
    """
    Pythonic version of sed command.
    """
    content = []
    matcher = re.compile(expr)

    with open(fname, 'r', encoding="utf-8") as data:
        for line in data:
            match = matcher.search(line)
            if not match:
                content.append(line)
            else:
                content.append(replace)

    with open(fname, 'w', encoding="utf-8") as data:
        data.writelines(content)


def _patch(ltp_dir, fname, new_timeout, override):
    """
    If `override` is True, it patches a test file, searching for timeout and
    replacing it with `new_timeout`.
    """
    orig_timeout = None
    file_path = os.path.join(ltp_dir, fname)

    with open(file_path, 'r', encoding="utf-8") as c_source:
        matcher = re.compile(r'\s*.timeout\s*=\s*(\d+).')
        for line in c_source:
            match = matcher.search(line)
            if not match:
                continue

            timeout = match.group(1)
            orig_timeout = int(timeout)

    if orig_timeout:
        if orig_timeout < new_timeout or override:
            print(f"CHANGE {fname} timeout {orig_timeout} -> {new_timeout}")
            _sed(file_path, r".timeout = [0-9]*,\n",
                 f"\t.timeout = {new_timeout},\n")
        else:
            print(f"KEEP   {fname} timeout {orig_timeout} (new {new_timeout})")
    else:
        print(f"ADD    {fname} timeout {new_timeout}")
        _sed(file_path,
             "static struct tst_test test = {",
             "static struct tst_test test = {\n"
             f"\t.timeout = {new_timeout},\n")


def _patch_all(ltp_dir, timeouts, override):
    """
    Patch all tests.
    """
    for timeout in timeouts:
        if timeout['path']:
            _patch(ltp_dir, timeout['path'], timeout['timeout'], override)


def _print_table(timeouts):
    """
    Print the timeouts table.
    """
    timeouts.sort(key=lambda x: x['timeout'], reverse=True)

    total = 0

    print("Old library tests\n-----------------\n")
    for timeout in timeouts:
        if not timeout['newlib']:
            print(f"{timeout['name']:30s} {timeout['timeout']}")
            total += 1

    print(f"\n\t{total} tests in total")

    total = 0

    print("\nNew library tests\n-----------------\n")
    for timeout in timeouts:
        if timeout['newlib']:
            print(f"{timeout['name']:30s} {timeout['timeout']}")
            total += 1

    print(f"\n\t{total} tests in total")


def _parse_data(ltp_dir, results_path):
    """
    Parse results data and metadata, then it generates timeouts data.
    """
    timeouts = []
    results = None
    metadata = None

    with open(results_path, 'r', encoding="utf-8") as file:
        results = json.load(file)

    metadata_path = os.path.join(ltp_dir, 'metadata', 'ltp.json')
    with open(metadata_path, 'r', encoding="utf-8") as file:
        metadata = json.load(file)

    for test in results['results']:
        name = test['test_fqn']
        duration = test['test']['duration']

        # if test runs for all_filesystems, normalize runtime to one filesystem
        filesystems = max(1, test['test']['log'].count('TINFO: Formatting /'))

        # check if test is new library test
        test_is_newlib = name in metadata['tests']

        # store test file path
        path = None
        if test_is_newlib:
            path = metadata['tests'][name]['fname']

        test_has_runtime = False
        if test_is_newlib:
            # filter out tests with runtime
            test_has_runtime = 'runtime' in metadata['tests'][name]

            # timer tests define runtime dynamically in timer library
            test_has_runtime = 'sample' in metadata['tests'][name]

        # select tests that does not have runtime and which are executed
        # for a long time
        if not test_has_runtime and duration >= 0.5:
            data = {}
            data["name"] = name
            data["timeout"] = int(TIMEOUT_MUL * duration/filesystems + 0.5)
            data["newlib"] = test_is_newlib
            data["path"] = path

            timeouts.append(data)

    return timeouts


def _file_exists(filepath):
    """
    Check if the given file path exists.
    """
    if not os.path.isfile(filepath):
        raise argparse.ArgumentTypeError(
            f"The file '{filepath}' does not exist.")
    return filepath


def _dir_exists(dirpath):
    """
    Check if the given directory path exists.
    """
    if not os.path.isdir(dirpath):
        raise argparse.ArgumentTypeError(
            f"The directory '{dirpath}' does not exist.")
    return dirpath


def run():
    """
    Entry point of the script.
    """
    parser = argparse.ArgumentParser(
        description="Script to calculate LTP tests timeouts")

    parser.add_argument(
        '-l',
        '--ltp-dir',
        type=_dir_exists,
        help='LTP source code directory',
        default='..')

    parser.add_argument(
        '-r',
        '--results',
        type=_file_exists,
        required=True,
        help='kirk results.json file location')

    parser.add_argument(
        '-o',
        '--override',
        default=False,
        action='store_true',
        help='Always override test timeouts')

    parser.add_argument(
        '-p',
        '--patch',
        default=False,
        action='store_true',
        help='Patch tests with updated timeout')

    parser.add_argument(
        '-t',
        '--print-table',
        default=True,
        action='store_true',
        help='Print table with suggested timeouts')

    args = parser.parse_args()

    timeouts = _parse_data(args.ltp_dir, args.results)

    if args.print_table:
        _print_table(timeouts)

    if args.patch:
        _patch_all(args.ltp_dir, timeouts, args.override)


if __name__ == "__main__":
    run()
