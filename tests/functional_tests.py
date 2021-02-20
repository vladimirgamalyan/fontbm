#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import subprocess
import difflib
import os.path
import filecmp
import shutil
import shlex
import xml.etree.ElementTree as ET
import json
import struct
# from collections.abc import Iterable
from collections import abc


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


def clear_work_dir():
    shutil.rmtree('generated', ignore_errors=True)
    os.makedirs('generated')


def test_expected(font_exe, env):
    clear_work_dir()
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


def read_xml(path):
    font_info = {
        'pages': [],
        'chars': [],
        'kernings': [],
    }
    tree = ET.parse(path)

    info_tag = tree.find('info')
    font_info['info'] = {k: int(v) for (k, v) in info_tag.attrib.items() if k not in ['face', 'charset',
                                                                                      'padding', 'spacing']}
    font_info['info']['face'] = info_tag.attrib['face']
    font_info['info']['charset'] = info_tag.attrib['charset']
    font_info['info']['padding'] = [int(n) for n in info_tag.attrib['padding'].split(',')]
    font_info['info']['spacing'] = [int(n) for n in info_tag.attrib['spacing'].split(',')]

    font_info['common'] = {k: int(v) for (k, v) in tree.find('common').attrib.items()}

    for elem in tree.iterfind('pages/page'):
        font_info['pages'].append({
            'id': int(elem.attrib['id']),
            'file': elem.attrib['file']
        })

    for char in tree.iterfind('chars/char'):
        font_info['chars'].append({k: int(v) for (k, v) in char.attrib.items()})

    for kerning in tree.iterfind('kernings/kerning'):
        font_info['kernings'].append({k: int(v) for (k, v) in kerning.attrib.items()})

    return font_info


def read_txt(path):
    font_info = {
        'pages': [],
        'chars': [],
        'kernings': [],
    }
    with open(path) as f:
        while True:
            line = f.readline()
            if not line:
                break
            # https://github.com/robhagemans/monobit/blob/7ac7af38f278ccc26eece442027d8aaaa40985a2/monobit/bmfont.py#L270
            tag, text_dict = line.strip().split(' ', 1)
            text_dict = dict(item.split('=') for item in shlex.split(text_dict) if item)
            if tag == 'info':
                font_info[tag] = {k: int(v) for (k, v) in text_dict.items() if k not in ['face', 'charset',
                                                                                         'padding', 'spacing']}
                font_info[tag]['face'] = text_dict['face']
                font_info[tag]['charset'] = text_dict['charset']
                font_info[tag]['padding'] = [int(n) for n in text_dict['padding'].split(',')]
                font_info[tag]['spacing'] = [int(n) for n in text_dict['spacing'].split(',')]
            if tag == 'common':
                font_info[tag] = {k: int(v) for (k, v) in text_dict.items()}
            elif tag == 'page':
                font_info['pages'].append({
                    'id': int(text_dict['id']),
                    'file': text_dict['file']
                })
            elif tag == 'char':
                font_info['chars'].append({k: int(v) for (k, v) in text_dict.items()})
            elif tag == 'kerning':
                font_info['kernings'].append({k: int(v) for (k, v) in text_dict.items()})
    return font_info


def read_json(path):
    with open(path) as f:
        data = json.load(f)
    for i in range(len(data['pages'])):
        data['pages'][i] = {'file': data['pages'][i], 'id': i}
    return data


def get_charset_name(char_set):
    d = {
        0: 'ANSI',
        1: 'DEFAULT',
        2: 'SYMBOL',
        128: 'SHIFTJIS',
        129: 'HANGUL',
        134: 'GB2312',
        136: 'CHINESEBIG5',
        255: 'OEM',
        130: 'JOHAB',
        177: 'HEBREW',
        178: 'ARABIC',
        161: 'GREEK',
        162: 'TURKISH',
        163: 'VIETNAMESE',
        222: 'THAI',
        238: 'EASTEUROPE',
        204: 'RUSSIAN',
        77:  'MAC',
        186: 'BALTIC'
    }
    return d[char_set]


def read_bin_string(b):
    r = ''
    i = 0
    while b[i] != 0:
        r += chr(b[i])
        i += 1
    return b[i + 1:], r


def read_bin(path):
    result = {}
    with open(path, mode='rb') as f:
        b = f.read()
    assert b[:4] == b'BMF\x03'
    b = b[4:]
    block_type, block_size = struct.unpack('<Bi', b[:5])
    assert block_type == 1
    b = b[5:]
    names = ('size', 'bit_field', 'charset', 'stretchH', 'aa', 'paddingUp',
             'paddingRight', 'paddingDown', 'paddingLeft', 'spacingHor', 'spacingVer',
             'outline')
    data = struct.unpack('<hBBHBBBBBBBB', b[:14])
    b = b[14:]
    d = dict(zip(names, data))
    d['padding'] = [d['paddingUp'], d['paddingRight'], d['paddingDown'], d['paddingLeft']]
    del d['paddingUp']
    del d['paddingRight']
    del d['paddingDown']
    del d['paddingLeft']
    d['spacing'] = [d['spacingHor'], d['spacingVer']]
    del d['spacingHor']
    del d['spacingVer']
    d['smooth'] = 1 if d['bit_field'] & (1 << 0) else 0
    d['unicode'] = 1 if d['bit_field'] & (1 << 1) else 0
    d['italic'] = 1 if d['bit_field'] & (1 << 2) else 0
    d['bold'] = 1 if d['bit_field'] & (1 << 3) else 0
    del d['bit_field']
    b, d['face'] = read_bin_string(b)
    d['charset'] = '' if d['unicode'] else get_charset_name(d['charset'])
    result['info'] = d
    block_type, block_size = struct.unpack('<Bi', b[:5])
    assert block_type == 2
    b = b[5:]
    names = ('lineHeight', 'base', 'scaleW', 'scaleH', 'pages', 'bit_field',
             'alphaChnl', 'redChnl', 'greenChnl', 'blueChnl')
    data = struct.unpack('<HHHHHBBBBB', b[:15])
    b = b[15:]
    d = dict(zip(names, data))
    d['packed'] = 1 if d['bit_field'] & (1 << 7) else 0
    del d['bit_field']
    result['common'] = d
    block_type, block_size = struct.unpack('<Bi', b[:5])
    b = b[5:]
    assert block_type == 3
    pages = []
    i = 0
    while True:
        b, file_name = read_bin_string(b)
        # TODO: check if there is \0 when there are no pages
        if not file_name:
            break
        pages.append({
            'id': i,
            'file': file_name
        })
        block_size -= len(file_name)
        if block_size < len(file_name):
            break
        i += 1
    result['pages'] = pages
    block_type, block_size = struct.unpack('<Bi', b[:5])
    b = b[5:]
    # print(block_type, block_size)
    assert block_type == 4
    assert block_size % 20 == 0
    char_count = block_size // 20
    chars = []
    for i in range(char_count):
        names = ('id', 'x', 'y', 'width', 'height', 'xoffset',
                 'yoffset', 'xadvance', 'page', 'chnl')
        data = struct.unpack('<IHHHHhhhBB', b[:20])
        b = b[20:]
        chars.append(dict(zip(names, data)))
    result['chars'] = chars
    assert not b''
    assert b'A'
    if b:
        block_type, block_size = struct.unpack('<Bi', b[:5])
        assert block_type == 5
        b = b[5:]
        assert block_size % 10 == 0
        kerning_count = block_size // 10
        kernings = []
        for i in range(kerning_count):
            names = ('first', 'second', 'amount')
            data = struct.unpack('<IIh', b[:10])
            b = b[10:]
            kernings.append(dict(zip(names, data)))
        result['kernings'] = kernings
        assert not b
    return result


def flatten(path, el):
    if isinstance(el, abc.Mapping):
        for key, value in el.items():
            yield from flatten(path + [key], value)
    elif isinstance(el, abc.Iterable) and not isinstance(el, (str, bytes)):
        for i, value in enumerate(el):
            yield from flatten(path + [i], value)
    else:
        yield path, el


def flatten_data(path):
    with open(path) as f:
        data = json.load(f)
        for i in flatten([], data):
            print(i)


def test_fnt_formats(font_exe, env):
    clear_work_dir()
    args = [font_exe, '--font-file', 'fonts/FreeSans.ttf', '--output', 'generated/format_test',
            '--chars', '32-126', '--font-size', '16',
            '--include-kerning-pairs',
            '--padding-up', '1', '--padding-right', '2', '--padding-down', '3', '--padding-left', '4',
            '--spacing-horiz', '5', '--spacing-vert', '6',
            '--texture-width', '128', '--texture-height', '128']

    subprocess.run(args + ['--data-format', 'txt'], check=True, env=env)
    data_txt = read_txt('generated/format_test.fnt')
    data_txt = json.dumps(data_txt, indent=4, sort_keys=True)
    with open('generated/test_txt.json', 'w') as json_file:
        json_file.write(data_txt)

    subprocess.run(args + ['--data-format', 'xml'], check=True, env=env)
    data_xml = read_xml('generated/format_test.fnt')
    data_xml = json.dumps(data_xml, indent=4, sort_keys=True)
    with open('generated/test_xml.json', 'w') as json_file:
        json_file.write(data_xml)

    subprocess.run(args + ['--data-format', 'json'], check=True, env=env)
    data_json = read_json('generated/format_test.fnt')
    data_json = json.dumps(data_json, indent=4, sort_keys=True)
    with open('generated/test_json.json', 'w') as json_file:
        json_file.write(data_json)

    subprocess.run(args + ['--data-format', 'bin'], check=True, env=env)
    data_bin = read_bin('generated/format_test.fnt')
    data_bin = json.dumps(data_bin, indent=4, sort_keys=True)
    with open('generated/test_bin.json', 'w') as json_file:
        json_file.write(data_bin)

    assert data_txt == data_json
    assert data_txt == data_xml
    assert data_txt == data_bin


def main(argv):
    assert len(argv) == 3
    font_exe = argv[1]
    runtime_lib_dir = argv[2]
    assert os.path.isfile(font_exe)
    assert os.path.isdir(runtime_lib_dir)

    env = os.environ.copy()
    env['PATH'] = os.pathsep.join((runtime_lib_dir, env.get('PATH', '')))

    test_expected(font_exe, env)
    test_too_many_textures(font_exe, env)
    test_fnt_formats(font_exe, env)
    # flatten_data('generated/test_txt.json')


if __name__ == '__main__':
    main(sys.argv)
