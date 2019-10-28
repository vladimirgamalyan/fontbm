#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import subprocess
import difflib
import os.path
import filecmp
import shutil


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
    assert os.path.isfile(expected)
    assert os.path.isfile(generated)
    different = (not filecmp.cmp(expected, generated)) if binary else diff(expected, generated)
    if different:
        raise RuntimeError('generated data not equal expected for ' + generated)


def main(argv):
    assert len(argv) == 2
    font_exe = argv[1]
    assert os.path.isfile(font_exe)

    shutil.rmtree('generated', ignore_errors=True)
    os.makedirs('generated')

    subprocess.check_call([font_exe, '--font-file', 'fonts/FreeSans.ttf', '--chars', '32-126',
        '--padding-up', '8', '--padding-right', '7', '--padding-down', '6', '--padding-left', '5',
        '--output', 'generated/test0', '--include-kerning-pairs'])
    check_diff('expected/test0.fnt', 'generated/test0.fnt')

    subprocess.check_call([font_exe, '--font-file', 'fonts/FreeSans.ttf', '--chars', '32-126',
        '--output', 'generated/test1', '--include-kerning-pairs',
        '--padding-up', '8', '--padding-right', '7',
        '--data-format', 'xml'])
    check_diff('expected/test1.fnt', 'generated/test1.fnt')

    subprocess.check_call([font_exe, '--font-file', 'fonts/FreeSans.ttf', '--chars', '32-126',
        '--output', 'generated/test2', '--include-kerning-pairs',
        '--spacing-vert', '4', '--spacing-horiz', '5',
        '--data-format', 'bin'])
    check_diff('expected/test2.fnt', 'generated/test2.fnt', True)

    subprocess.check_call([font_exe, '--font-file', 'fonts/FreeSans.ttf', '--chars', '32-126',
        '--output', 'generated/test3', '--include-kerning-pairs',
        '--texture-width', '32', '--texture-height', '32'])
    check_diff('expected/test3.fnt', 'generated/test3.fnt')


if __name__ == "__main__":
    main(sys.argv)
