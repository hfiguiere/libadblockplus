#!/usr/bin/env python3
# coding: utf-8

import logging
import os
import urllib.request
import subprocess
import sys
import time

def download_file(url, dest_file_path):
    downloading_started_at = time.time()
    show_interval = 5 # seconds
    last_shown_interval = 0
    def report_download_progress(chunk_number, chunk_size, total_size):
        nonlocal last_shown_interval
        current_interval = (time.time() - downloading_started_at) // show_interval
        if current_interval <= last_shown_interval:
            return
        last_shown_interval = current_interval
        if total_size > 0:
             logging.info('Download progress: {:>3d}%'.format(100 * chunk_number * chunk_size // total_size))
    urllib.request.urlretrieve(url, dest_file_path, report_download_progress)

def install_git_lfs():
    """
    """
    url = 'https://github.com/github/git-lfs/releases/download/v1.2.1/git-lfs-linux-amd64-1.2.1.tar.gz'
    tmp_dir_path = '/tmp'
    user_bin_path = os.path.join(os.path.expanduser('~'), 'bin')
    if not os.path.exists(user_bin_path):
        os.makedirs(user_bin_path)
    if sys.platform == 'win32':
        url = 'https://github.com/github/git-lfs/releases/download/v1.2.1/git-lfs-windows-amd64-1.2.1.zip'
        tmp_dir_path = os.environ['TEMP']
    dest_file_path = os.path.join(tmp_dir_path, os.path.basename(url))
    logging.info('{}\n{}'.format(url, dest_file_path))
    download_file(url, dest_file_path)

    env = os.environ.copy()
    if sys.platform == 'win32':
        git_lfs_exe = 'git-lfs.exe'
        path_to_7zip = os.path.join(env['PROGRAMFILES'], '7-Zip', '7z.exe')
        cmd = [path_to_7zip, 'e', dest_file_path, 'git-lfs-windows-amd64-1.2.1\\' + git_lfs_exe, '-o{}'.format(user_bin_path), '-y']
        subprocess.run(cmd, check = True)
        subprocess.run([os.path.join(user_bin_path, git_lfs_exe), 'install'], check = True)
    else:
        cmd = ['tar', 'xzf', dest_file_path, '-C', user_bin_path, '--strip-components', '1', 'git-lfs-1.2.1/git-lfs', 'git-lfs-1.2.1/install.sh']
        subprocess.run(cmd, check = True)
        env['PREFIX'] = user_bin_path
        install_sh = os.path.join(user_bin_path, 'install.sh')
        subprocess.run([install_sh], cwd = user_bin_path, env = env, check = True, shell = True)
        os.remove(install_sh)

if __name__ == '__main__':
    logging.basicConfig(format = '%(levelname)s:%(message)s', level=logging.DEBUG)
    install_git_lfs()
