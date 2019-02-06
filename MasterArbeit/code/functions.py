import os
import re
import sys
from contextlib import contextmanager


def SubDirPath(root):
    return filter(os.path.isdir, [os.path.join(root, dir) for dir in os.listdir(root)])


def FilesInDir(dirpath, pattern=r'([0-9a-zA-Z._-]*)'):
    file_list = []
    for f in os.listdir(dirpath):
        f = re.search(pattern, f)
        if f is not None:
            file_list.append(os.path.join(dirpath, f.group(0)))
    return file_list


def fileno(file_or_fd):
    fd = getattr(file_or_fd, 'fileno', lambda: file_or_fd)()
    if not isinstance(fd, int):
        raise ValueError("Expected a file (`.fileno()`) or a file descriptor")
    return fd


@contextmanager
def stdout_redirected(to=os.devnull, stdout=None):
    if stdout is None:
        stdout = sys.stdout

    stdout_fd = fileno(stdout)
    # copy stdout_fd before it is overwritten
    # NOTE: `copied` is inheritable on Windows when duplicating a standard stream
    with os.fdopen(os.dup(stdout_fd), 'wb') as copied:
        stdout.flush()  # flush library buffers that dup2 knows nothing about
        try:
            os.dup2(fileno(to), stdout_fd)  # $ exec >&to
        except ValueError:  # filename
            with open(to, 'wb') as to_file:
                os.dup2(to_file.fileno(), stdout_fd)  # $ exec > to
        try:
            yield stdout  # allow code to be run with the redirected stdout
        finally:
            # restore stdout to its previous value
            # NOTE: dup2 makes stdout_fd inheritable unconditionally
            stdout.flush()
            os.dup2(copied.fileno(), stdout_fd)  # $ exec >&copied


def merged_stderr_stdout():  # $ exec 2>&1
    return stdout_redirected(to=sys.stdout, stdout=sys.stderr)


def cal_ext_size(original_size):
    for t in range(0, 4):
        original_size = (original_size + 4) / 2
    original_size = (original_size + 4)
    for t in range(0, 4):
        original_size = original_size * int(2) + int(4)
    return original_size


def cal_seg_mask_width_stride(max, img_ext_size):
    for i in range(1, max):
        for j in range(1, i):
            if (img_ext_size - i) % j == 0:
                N = (img_ext_size - i)/j
                tmp = i
                for t in range(0, 4):
                    tmp = (tmp - 4) / 2
                tmp = (tmp - 4)
                for t in range(0, 4):
                    tmp = tmp * int(2) - int(4)
                if tmp==j:
                    print i, j, N


def my_range(start, end, step):
    while start < end:
        yield start
        start += step

