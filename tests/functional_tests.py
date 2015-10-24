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
    if not os.path.isfile(expected):
        raise RuntimeError('expected file not found')
    if not os.path.isfile(generated):
        raise RuntimeError('generated file not found')
    different = filecmp.cmp(expected, generated) if binary else diff(expected, generated)
    if different:
        raise RuntimeError('generated data not equal expected')


def run_fontbm(args):
    if subprocess.call(args) != 0:
        raise RuntimeError('fontbm error')


def main(argv):
    if len(argv) != 2:
        raise RuntimeError('invalid test arguments')
    fontbm = argv[1]
    if not os.path.isfile(fontbm):
        raise RuntimeError('not found fontbm executable')

    shutil.rmtree('generated', ignore_errors=True)

    run_fontbm([fontbm, '--font-file', 'fonts/FreeSans.ttf',
               '--output', 'generated/test0', '--include-kerning-pairs'])
    check_diff('expected/test0.fnt', 'generated/test0.fnt')

    run_fontbm([fontbm, '--font-file', 'fonts/FreeSans.ttf',
               '--output', 'generated/test1', '--include-kerning-pairs',
               '--data-format', 'xml'])
    check_diff('expected/test1.fnt', 'generated/test1.fnt')

    run_fontbm([fontbm, '--font-file', 'fonts/FreeSans.ttf',
               '--output', 'generated/test2', '--include-kerning-pairs',
               '--data-format', 'bin'])
    check_diff('expected/test2.fnt', 'generated/test2.fnt', True)

    run_fontbm([fontbm, '--font-file', 'fonts/FreeSans.ttf',
               '--output', 'generated/test3', '--include-kerning-pairs',
               '--texture-width', '32', '--texture-height', '32'])
    check_diff('expected/test3.fnt', 'generated/test3.fnt', True)


if __name__ == "__main__":
    try:
        main(sys.argv)
    except RuntimeError, arg:
        print arg
        sys.exit(1)
