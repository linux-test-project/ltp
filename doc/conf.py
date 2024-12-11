# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import re
import socket
import urllib.request
import sphinx

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Linux Test Project'
copyright = '2024, Linux Test Project'
author = 'Linux Test Project'
release = '1.0'
ltp_repo = 'https://github.com/linux-test-project/ltp'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'linuxdoc.rstKernelDoc',
    'sphinxcontrib.spelling',
    'sphinx.ext.autosectionlabel',
    'sphinx.ext.extlinks'
]

exclude_patterns = ["html*", '_static*']
extlinks = {
    'repo': (f'{ltp_repo}/%s', '%s'),
    'master': (f'{ltp_repo}/blob/master/%s', '%s'),
    'git_man': ('https://git-scm.com/docs/git-%s', 'git %s'),
    # TODO: allow 2nd parameter to show page description instead of plain URL
    'kernel_doc': ('https://docs.kernel.org/%s.html', 'https://docs.kernel.org/%s.html'),
    'kernel_tree': ('https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/%s', '%s'),
}

spelling_lang = "en_US"
spelling_warning = True
spelling_exclude_patterns = ['users/stats.rst']
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
    ltp_syscalls_path = "testcases/kernel/syscalls"
    white_list = {
        'bpf': f'{ltp_syscalls_path}/bpf',
        'epoll_pwait2': f'{ltp_syscalls_path}/epoll_pwait',
        'fadvise64': f'{ltp_syscalls_path}/fadvise',
        'fanotify_init': f'{ltp_syscalls_path}/fanotify',
        'fanotify_mark': f'{ltp_syscalls_path}/fanotify',
        'futex': f'{ltp_syscalls_path}/futex',
        'getdents64': f'{ltp_syscalls_path}/gettdents',
        'inotify_add_watch': f'{ltp_syscalls_path}/inotify',
        'inotify_init': f'{ltp_syscalls_path}/inotify',
        'inotify_rm_watch': f'{ltp_syscalls_path}/inotify',
        'io_uring_enter': f'{ltp_syscalls_path}/io_uring',
        'io_uring_register': f'{ltp_syscalls_path}/io_uring',
        'io_uring_setup': f'{ltp_syscalls_path}/io_uring',
        'landlock_add_rule': f'{ltp_syscalls_path}/landlock',
        'landlock_create_ruleset': f'{ltp_syscalls_path}/landlock',
        'landlock_restrict_self': f'{ltp_syscalls_path}/landlock',
        'lsetxattr': f'{ltp_syscalls_path}/lgetxattr',
        'newfstatat': f'{ltp_syscalls_path}/fstatat',
        'pkey_alloc': f'{ltp_syscalls_path}/pkeys',
        'pkey_free': f'{ltp_syscalls_path}/pkeys',
        'pkey_mprotect': f'{ltp_syscalls_path}/pkeys',
        'prlimit64': f'{ltp_syscalls_path}/getrlimit',
        'pread64': f'{ltp_syscalls_path}/pread',
        'pselect6': f'{ltp_syscalls_path}/pselect',
        'pwrite64': f'{ltp_syscalls_path}/pwrite',
        'quotactl_fd': f'{ltp_syscalls_path}/quotactl',
        'rt_sigpending': f'{ltp_syscalls_path}/sigpending',
        'semtimedop': f'{ltp_syscalls_path}/ipc/semop',
        'sethostname': f'{ltp_syscalls_path}/sethostname'
    }

    # populate with not implemented, reserved, unmaintained syscalls defined
    # inside the syscalls file
    black_list = [
        '_newselect',
        '_sysctl',
        'afs_syscall',
        'cachectl',
        'create_module',
        'get_kernel_syms',
        'mq_getsetattr',
        'nfsservctl',
        'query_module',
        'reserved177',
        'reserved193',
        'restart_syscall',
        'rseq',
        'sysmips',
        'vserver',
    ]

    # fetch syscalls file
    syscalls_url = "https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/plain/arch/mips/kernel/syscalls"
    error = False
    try:
        socket.setdefaulttimeout(3)
        urllib.request.urlretrieve(
            f"{syscalls_url}/syscall_n64.tbl", "syscalls.tbl")
    except urllib.error.URLError as err:
        error = True
        logger = sphinx.util.logging.getLogger(__name__)
        msg = f"Can't download syscall_n64.tbl from kernel sources ({err})"
        logger.warning(msg)

        with open(output, 'w+', encoding='utf-8') as stats:
            stats.write(f".. warning::\n\n    {msg}")

    if error:
        return

    syscalls_base_url = f"{ltp_repo}/tree/master"
    text = [
        'Syscalls\n',
        '--------\n\n',
    ]

    # collect all available kernel syscalls
    regexp = re.compile(r'\d+\s+n64\s+(?P<syscall>\w+)\s+\w+')
    ker_syscalls = []
    with open("syscalls.tbl", 'r', encoding='utf-8') as data:
        for line in data:
            match = regexp.search(line)
            if not match:
                continue

            ker_syscalls.append(match.group('syscall'))

    # collect all LTP tested syscalls
    name_patterns = [
        re.compile(r'(?P<name>[a-zA-Z_]+[^_])\d{2}\.c'),
        re.compile(r'(?P<name>[a-zA-Z_]+[1-9])_\d{2}\.c'),
    ]
    ltp_syscalls = {}
    for dirpath, _, files in os.walk(f'../{ltp_syscalls_path}'):
        for myfile in files:
            match = None
            for pattern in name_patterns:
                match = pattern.search(myfile)
                if match:
                    break

            if not match:
                continue

            # we need to use relative path from the project root
            path = dirpath.replace('../', '')
            name = match.group('name')

            ltp_syscalls[name] = f'{syscalls_base_url}/{path}'

    # compare kernel syscalls with LTP tested syscalls
    syscalls = {}
    for kersc in ker_syscalls:
        if kersc in black_list:
            continue

        if kersc not in syscalls:
            if kersc in white_list:
                syscalls[kersc] = f'{syscalls_base_url}/{white_list[kersc]}'
                continue

            syscalls[kersc] = None

        for ltpsc, ltpsp in ltp_syscalls.items():
            if ltpsc == kersc:
                syscalls[kersc] = ltpsp

    # generate the statistics file
    tested_syscalls = [key for key, val in syscalls.items() if val is not None]
    text.append('syscalls which are tested under '
                ':master:`testcases/kernel/syscalls`:\n\n')
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

    max_columns = 3

    for sysname, path in syscalls.items():
        if path is not None:
            if (index_tested % max_columns) == 0:
                table_tested.append(f'    * - `{sysname} <{path}>`_\n')
            else:
                table_tested.append(f'      - `{sysname} <{path}>`_\n')

            index_tested += 1
        else:
            if (index_untest % max_columns) == 0:
                table_untest.append(f'    * - {sysname}\n')
            else:
                table_untest.append(f'      - {sysname}\n')

            index_untest += 1

    left = index_tested % max_columns
    if left > 0:
        for _ in range(0, max_columns - left):
            table_tested.append('      -\n')

    left = index_untest % max_columns
    if left > 0:
        for _ in range(0, max_columns - left):
            table_untest.append('      -\n')

    text.extend(table_tested)
    text.append('\n')
    text.extend(table_untest)

    # write the file
    with open(output, 'w+', encoding='utf-8') as stats:
        stats.writelines(text)


def setup(app):
    """
    Setup the current documentation, using self generated data and graphics
    customizations.
    """
    app.add_css_file('custom.css')
    app.connect('builder-inited', generate_syscalls_stats)
