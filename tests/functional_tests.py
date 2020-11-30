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
        raise RuntimeError('generated data is not equal for expected ' + generated)


def test_expected(font_exe, env):
    subprocess.run([font_exe, '--font-file', 'fonts/FreeSans.ttf', '--chars', '32-126',
        '--padding-up', '8', '--padding-right', '7', '--padding-down', '6', '--padding-left', '5',
        '--output', 'generated/test0', '--include-kerning-pairs'], check=True, env=env)
    check_diff('expected/test0.fnt', 'generated/test0.fnt')

    subprocess.run([font_exe, '--font-file', 'fonts/FreeSans.ttf', '--chars', '32-126',
        '--output', 'generated/test1', '--include-kerning-pairs',
        '--padding-up', '8', '--padding-right', '7',
        '--data-format', 'xml'], check=True, env=env)
    check_diff('expected/test1.fnt', 'generated/test1.fnt')

    subprocess.run([font_exe, '--font-file', 'fonts/FreeSans.ttf', '--chars', '32-126',
        '--output', 'generated/test2', '--include-kerning-pairs',
        '--spacing-vert', '4', '--spacing-horiz', '5',
        '--data-format', 'bin'], check=True, env=env)
    check_diff('expected/test2.fnt', 'generated/test2.fnt', True)


def test_too_many_textures(font_exe, env):
    process = subprocess.Popen([font_exe, '--font-file', 'fonts/FreeSans.ttf', '--chars', '32-126',
                                '--output', 'generated/test3', '--include-kerning-pairs',
                                '--texture-width', '128', '--texture-height', '128',
                                '--max-texture-count', '1'],
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                               env=env, text=True)
    out, err = process.communicate()
    assert out == ''
    assert err.strip() == 'too many generated textures (more than --max-texture-count)'
    assert process.returncode == 1


def main(argv):
    assert len(argv) == 3
    font_exe = argv[1]
    runtime_lib_dir = argv[2]
    assert os.path.isfile(font_exe)
    assert os.path.isdir(runtime_lib_dir)

    shutil.rmtree('generated', ignore_errors=True)
    os.makedirs('generated')

    env = os.environ.copy()
    env['PATH'] = os.pathsep.join((runtime_lib_dir, env.get('PATH', '')))

    test_expected(font_exe, env)
    test_too_many_textures(font_exe, env)


if __name__ == '__main__':
    main(sys.argv)
