#!/usr/bin/env python
# coding: utf-8

import os
import sys

base_dir = os.path.abspath(os.path.dirname(__file__))
sys.path.append(os.path.join(base_dir, 'third_party', 'gyp', 'pylib'))
import gyp
import gyp.generator.msvs

orig_fix_path = gyp.generator.msvs._FixPath

dont_expand = [
# js2c
  'CORE', 'EXPERIMENTAL', 'off', 'EXTRAS', 'EXPERIMENTAL_EXTRAS',
# build-v8
  'ia32', 'x64'
]

def _FixPath(path):
    if path in dont_expand:
        # Don't touch js2c and build-v8 parameters
        return path
    return orig_fix_path(path)
gyp.generator.msvs._FixPath = _FixPath

if __name__ == '__main__':
    gyp.main(sys.argv[1:])
