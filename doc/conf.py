# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import re
import json
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
ltp_repo_base_url = f"{ltp_repo}/tree/master"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'linuxdoc.rstKernelDoc',
    'sphinxcontrib.spelling',
    'sphinx.ext.autosectionlabel',
    'sphinx.ext.extlinks',
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
        'getmsg',
        'getpmsg',
        'mq_getsetattr',
        'nfsservctl',
        'putmsg',
        'putpmsg',
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

        # kernel.org doesn't always allow to download syscalls file, so we need
        # to emulate a different client in order to download it. Browser
        # emulation might fail due to captcha request and for this reason we
        # use the 'curl' command instead
        req = urllib.request.Request(
            f"{syscalls_url}/syscall_n64.tbl",
            headers={'User-Agent': 'curl/8.6.0'})

        with urllib.request.urlopen(req) as response:
            with open("syscalls.tbl", 'wb') as out_file:
                out_file.write(response.read())
    except urllib.error.URLError as err:
        error = True
        logger = sphinx.util.logging.getLogger(__name__)
        msg = f"Can't download syscall_n64.tbl from kernel sources ({err})"
        logger.warning(msg)

        with open(output, 'w+', encoding='utf-8') as stats:
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

            ltp_syscalls[name] = f'{ltp_repo_base_url}/{path}'

    # compare kernel syscalls with LTP tested syscalls
    syscalls = {}
    for kersc in ker_syscalls:
        if kersc in black_list:
            continue

        if kersc not in syscalls:
            if kersc in white_list:
                syscalls[kersc] = f'{ltp_repo_base_url}/{white_list[kersc]}'
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


def _generate_tags_table(tags):
    """
    Generate the tags table from tags hash.
    """
    supported_url_ref = {
        "linux-git": "https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=",
        "linux-stable-git": "https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/commit/?id=",
        "glibc-git": "https://sourceware.org/git/?p=glibc.git;a=commit;h=",
        "musl-git": "https://git.musl-libc.org/cgit/musl/commit/src/linux/clone.c?id=",
        "CVE": "https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-",
    }

    table = [
        '.. list-table::',
        '   :header-rows: 1',
        '',
        '   * - Tag',
        '     - Info',
    ]

    for tag in tags:
        tag_key = tag[0]
        tag_val = tag[1]

        tag_url = supported_url_ref.get(tag_key, None)
        if tag_url:
            tag_val = f'`{tag_val} <{tag_url}{tag_val}>`_'

        table.extend([
            f'   * - :c:struct:`{tag_key} <tst_tag>`',
            f'     - {tag_val}',
        ])

    return table


def _generate_options_table(options):
    """
    Generate the options table from the options hash.
    """
    table = [
        '.. list-table::',
        '   :header-rows: 1',
        '',
        '   * - Option',
        '     - Description',
    ]

    for opt in options:
        if not isinstance(opt, list):
            table.clear()
            break

        key = opt[0]
        val = opt[2]

        if key.endswith(':'):
            key = key[:-1] if key.endswith(':') else key

        key = f'-{key}'

        table.extend([
            f'   * - {key}',
            f'     - {val}',
        ])

    return table


def _generate_table_cell(key, values):
    """
    Generate a cell which can be multiline if value is a list.
    """
    cell = []
    key = f' :c:struct:`{key} <tst_test>`'

    if len(values) > 1:
        cell.extend([
            f'   * - {key}',
            f'     - | {values[0]}',
        ])

        for item in values[1:]:
            cell.append(f'       | {item}')
    else:
        cell.extend([
            f'   * - {key}',
            f'     - {values[0]}',
        ])

    return cell


def _generate_setup_table(keys):
    """
    Generate the table with test setup configuration.
    """
    exclude = [
        # following keys are already handled
        'options',
        'runtime',
        'timeout',
        'fname',
        'doc',
        # following keys don't need to be shown
        'child_needs_reinit',
        'needs_checkpoints',
        'forks_child',
        'tags',
    ]
    my_keys = {k: v for k, v in keys.items() if k not in exclude}
    if len(my_keys) == 0:
        return []

    table = [
        '.. list-table::',
        '   :header-rows: 1',
        '',
        '   * - Key',
        '     - Value',
    ]

    values = []

    for key, value in my_keys.items():
        if key in exclude:
            continue

        values.clear()

        if key == 'ulimit':
            for item in value:
                values.append(f'{item[0]} : {item[1]}')
        elif key == 'hugepages':
            if len(value) == 1:
                values.append(f'{value[0]}')
            else:
                values.append(f'{value[0]}, {value[1]}')
        elif key == 'filesystems':
            for params in value:
                for pkey, pval in params.items():
                    if pkey == "type":
                        values.append(f"- {pval}")
                    else:
                        values.append(f" {pkey}: {pval}")
        elif key == "save_restore":
            for item in value:
                values.append(item[0])
        else:
            if isinstance(value, list):
                values.extend(value)
            else:
                values.append(value)

        if values:
            table.extend(_generate_table_cell(key, values))

    return table


def generate_test_catalog(_):
    """
    Generate the test catalog from ltp.json metadata file.
    """
    output = '_static/tests.rst'
    metadata_file = '../metadata/ltp.json'
    text = [
        '.. warning::',
        '    The following catalog has been generated using LTP metadata',
        '    which is including only tests using the new :ref:`LTP C API`.',
        ''
    ]

    metadata = None
    with open(metadata_file, 'r', encoding='utf-8') as data:
        metadata = json.load(data)

    timeout_def = metadata['defaults']['timeout']
    regexp = re.compile(r'^\[([A-Za-z][\w\W]+)\]')

    for test_name, conf in sorted(metadata['tests'].items()):
        text.extend([
            f'{test_name}',
            len(test_name) * '-'
        ])

        # source url location
        test_fname = conf.get('fname', None)
        if test_fname:
            text.extend([
                '',
                f"`source <{ltp_repo_base_url}/{test_fname}>`__",
                ''
            ])

        # test description
        desc = conf.get('doc', None)
        if desc:
            desc_text = []
            for line in desc:
                line = regexp.sub(r'**\1**', line)
                desc_text.append(line)

            text.extend([
                '\n'.join(desc_text),
            ])

        # timeout information
        timeout = conf.get('timeout', None)
        if timeout:
            text.extend([
                '',
                f'Test timeout is {timeout} seconds.',
            ])
        else:
            text.extend([
                '',
                f'Test timeout defaults is {timeout_def} seconds.',
            ])

        # runtime information
        runtime = conf.get('runtime', None)
        if runtime:
            text.extend([
                f'Maximum runtime is {runtime} seconds.',
                ''
            ])
        else:
            text.append('')

        # options information
        opts = conf.get('options', None)
        if opts:
            text.append('')
            text.extend(_generate_options_table(opts))
            text.append('')

        # tags information
        tags = conf.get('tags', None)
        if tags:
            text.append('')
            text.extend(_generate_tags_table(tags))
            text.append('')

        # parse struct tst_test content
        text.append('')
        text.extend(_generate_setup_table(conf))
        text.append('')

        # small separator between tests
        text.extend([
            '',
            '.. raw:: html',
            '',
            '    <hr>',
            '',
        ])

    with open(output, 'w+', encoding='utf-8') as new_tests:
        new_tests.write('\n'.join(text))


def setup(app):
    """
    Setup the current documentation, using self generated data and graphics
    customizations.
    """
    app.add_css_file('custom.css')
    app.connect('builder-inited', generate_syscalls_stats)
    app.connect('builder-inited', generate_test_catalog)
