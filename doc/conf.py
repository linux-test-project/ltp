# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import re
import sphinx
import socket
import urllib.request

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Linux Test Project'
copyright = '2024, Linux Test Project'
author = 'Linux Test Project'
release = '1.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'linuxdoc.rstKernelDoc',
    'sphinxcontrib.spelling',
    'sphinx.ext.extlinks'
]

exclude_patterns = ["html*", '_static*']
extlinks = {
    'repo': ('https://github.com/linux-test-project/ltp/%s', '%s'),
    'master': ('https://github.com/linux-test-project/ltp/blob/master/%s', '%s'),
    'git_man': ('https://git-scm.com/docs/git-%s', 'git %s'),
    # TODO: allow 2nd parameter to show page description instead of plain URL
    'kernel_doc': ('https://docs.kernel.org/%s.html', 'https://docs.kernel.org/%s.html'),
    'kernel_tree': ('https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/%s', '%s'),
}

spelling_lang = "en_US"
spelling_warning = True
spelling_exclude_patterns=['users/stats.rst']
spelling_word_list_filename = "spelling_wordlist"

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']


def generate_syscalls_stats(_):
    """
    Generate statistics for syscalls. We fetch the syscalls list from the kernel
    sources, then we compare it with testcases/kernel/syscalls folder and
    generate a file that is included in the statistics documentation section.
    """
    output = '_static/syscalls.rst'

    # sometimes checking testcases/kernel/syscalls file names are not enough,
    # because in some cases (i.e. io_ring) syscalls are tested, but they are
    # part of a more complex scenario. In the following list, we define syscalls
    # which we know they are 100% tested already.
    white_list = [
        'rt_sigpending',
        'sethostname',
        'lsetxattr',
        'inotify_add_watch',
        'inotify_rm_watch',
        'newfstatat',
        'pselect6',
        'fanotify_init',
        'fanotify_mark',
        'prlimit64',
        'getdents64',
        'pkey_mprotect',
        'pkey_alloc',
        'pkey_free',
        'io_uring_setup',
        'io_uring_enter',
        'io_uring_register',
        'epoll_pwait2',
        'quotactl_fd',
        'pread64',
        'pwrite64',
        'fadvise64',
        'getmsg',
        'getpmsg',
        'putmsg',
        'putpmsg',
    ]

    # populate with not implemented, reserved, unmaintained syscalls defined
    # inside the syscalls file
    black_list = [
        'reserved177',
        'reserved193',
        'rseq',
        '_newselect',
        '_sysctl',
        'create_module',
        'get_kernel_syms',
        'query_module',
        'nfsservctl',
        'afs_syscall',
        'sysmips',
        'mq_getsetattr',
        'vserver',
    ]

    # fetch syscalls file
    error = False
    try:
        socket.setdefaulttimeout(3)
        urllib.request.urlretrieve(
            "https://raw.githubusercontent.com/torvalds/linux/master/arch/mips/kernel/syscalls/syscall_n64.tbl",
            "syscalls.tbl")
    except Exception as err:
        error = True
        logger = sphinx.util.logging.getLogger(__name__)
        msg = "Can't download syscall_n64.tbl from kernel sources"
        logger.warning(msg)

        with open(output, 'w+') as stats:
            stats.write(f".. warning::\n\n    {msg}")

    if error:
        return

    text = [
        'Syscalls\n',
        '--------\n\n',
    ]

    # collect all available kernel syscalls
    regexp = re.compile(r'\d+\s+n64\s+(?P<syscall>\w+)\s+\w+')
    ker_syscalls = []
    with open("syscalls.tbl", 'r') as data:
        for line in data:
            match = regexp.search(line)
            if match:
                ker_syscalls.append(match.group('syscall'))

    # collect all LTP tested syscalls
    ltp_syscalls = []
    for root, _, files in os.walk('../testcases/kernel/syscalls'):
        for myfile in files:
            if myfile.endswith('.c'):
                ltp_syscalls.append(myfile)

    # compare kernel syscalls with LTP tested syscalls
    syscalls = {}
    for kersc in ker_syscalls:
        if kersc in black_list:
            continue

        if kersc not in syscalls:
            if kersc in white_list:
                syscalls[kersc] = True
                continue

            syscalls[kersc] = False

        for ltpsc in ltp_syscalls:
            if ltpsc.startswith(kersc):
                syscalls[kersc] = True

    # generate the statistics file
    tested_syscalls = [key for key, val in syscalls.items() if val]
    text.append('syscalls which are tested under :master:`testcases/kernel/syscalls`:\n\n')
    text.append(f'* kernel syscalls: {len(ker_syscalls)}\n')
    text.append(f'* tested syscalls: {len(tested_syscalls)}\n\n')

    # create tested/untested syscalls table
    index_tested = 0
    table_tested = [
        'Tested syscalls\n',
        '~~~~~~~~~~~~~~~\n\n',
        '.. list-table::\n',
        '    :header-rows: 0\n\n',
    ]

    index_untest = 0
    table_untest = [
        'Untested syscalls\n',
        '~~~~~~~~~~~~~~~~~\n\n',
        '.. list-table::\n',
        '    :header-rows: 0\n\n',
    ]

    for sysname, tested in syscalls.items():
        if tested:
            if (index_tested % 3) == 0:
                table_tested.append(f'    * - {sysname}\n')
            else:
                table_tested.append(f'      - {sysname}\n')

            index_tested += 1
        else:
            if (index_untest % 3) == 0:
                table_untest.append(f'    * - {sysname}\n')
            else:
                table_untest.append(f'      - {sysname}\n')

            index_untest += 1

    left = index_tested % 3
    if left > 0:
        for index in range(0, 3 - left):
            table_tested.append(f'      -\n')

    left = index_untest % 3
    if left > 0:
        for index in range(0, 3 - left):
            table_untest.append(f'      -\n')

    text.extend(table_tested)
    text.append('\n')
    text.extend(table_untest)

    # write the file
    with open(output, 'w+') as stats:
        stats.writelines(text)


def setup(app):
    app.add_css_file('custom.css')
    app.connect('builder-inited', generate_syscalls_stats)
