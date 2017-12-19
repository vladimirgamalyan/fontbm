#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import subprocess
import difflib
import os.path
import filecmp
import shutil

font_exe = ''

def diff(expected, generated):
    with open(expected, 'r') as a:
        with open(generated, 'r') as b:
            d = list(difflib.unified_diff(
                a.readlines(),
                b.readlines(),
                fromfile=expected,
                tofile=generated,
                n=0
            ))
            for line in d:
                sys.stdout.write(line)
            return bool(d)


def check_diff(expected, generated, binary=False):
    if not os.path.isfile(expected):
        raise RuntimeError('expected file not found')
    if not os.path.isfile(generated):
        raise RuntimeError('generated file not found')
    different = (not filecmp.cmp(expected, generated)) if binary else diff(expected, generated)
    if different:
        raise RuntimeError('generated data not equal expected for ' + generated)


def fontbm(*args):
    global font_exe
    call_args = [font_exe] + list(args)
    if subprocess.call(call_args) != 0:
        raise RuntimeError('fontbm error')


def main(argv):
    global font_exe
    if len(argv) != 2:
        raise RuntimeError('invalid test arguments')
    font_exe = argv[1]
    if not os.path.isfile(font_exe):
        raise RuntimeError('not found fontbm executable')

    shutil.rmtree('generated', ignore_errors=True)
    os.makedirs('generated')

    fontbm('--font-file', 'fonts/FreeSans.ttf', '--chars', '32-126',
        '--padding-up', '8', '--padding-right', '7',
        '--padding-down', '6', '--padding-left', '5',
        '--output', 'generated/test0', '--include-kerning-pairs')
    check_diff('expected/test0.fnt', 'generated/test0.fnt')

    fontbm('--font-file', 'fonts/FreeSans.ttf', '--chars', '32-126',
        '--output', 'generated/test1', '--include-kerning-pairs',
        '--padding-up', '8', '--padding-right', '7',
        '--data-format', 'xml')
    check_diff('expected/test1.fnt', 'generated/test1.fnt')

    fontbm('--font-file', 'fonts/FreeSans.ttf', '--chars', '32-126',
        '--output', 'generated/test2', '--include-kerning-pairs',
        '--spacing-vert', '4', '--spacing-horiz', '5',
        '--data-format', 'bin')
    check_diff('expected/test2.fnt', 'generated/test2.fnt', True)

    fontbm('--font-file', 'fonts/FreeSans.ttf', '--chars', '32-126',
        '--output', 'generated/test3', '--include-kerning-pairs',
        '--texture-width', '32', '--texture-height', '32')
    check_diff('expected/test3.fnt', 'generated/test3.fnt')


if __name__ == "__main__":
    try:
        main(sys.argv)
    except RuntimeError, arg:
        print arg
        sys.exit(1)
