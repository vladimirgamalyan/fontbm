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


def text_to_value(v):
    return v


def read_xml(path):
    font_info = {
        'pages': [],
        'chars': [],
        'kernings': [],
    }
    tree = ET.parse(path)
    info_tag = tree.find('info')
    font_info['info'] = {
        'face': info_tag.attrib['face'],
        'size': int(info_tag.attrib['size']),
        'bold': int(info_tag.attrib['bold']),
        'italic': int(info_tag.attrib['italic']),
        'charset': info_tag.attrib['charset'],
        'unicode': int(info_tag.attrib['unicode']),
        'stretchH': int(info_tag.attrib['stretchH']),
        'smooth': int(info_tag.attrib['smooth']),
        'aa': int(info_tag.attrib['aa']),
        'padding': [int(n) for n in info_tag.attrib['padding'].split(',')],
        'spacing': [int(n) for n in info_tag.attrib['spacing'].split(',')],
        'outline': int(info_tag.attrib['outline'])
    }
    common_tag = tree.find('common')
    font_info['common'] = {
        'lineHeight': int(common_tag.attrib['lineHeight']),
        'base': int(common_tag.attrib['base']),
        'scaleW': int(common_tag.attrib['scaleW']),
        'scaleH': int(common_tag.attrib['scaleH']),
        'pages': int(common_tag.attrib['pages']),
        'packed': int(common_tag.attrib['packed']),
        'alphaChnl': int(common_tag.attrib['alphaChnl']),
        'redChnl': int(common_tag.attrib['redChnl']),
        'greenChnl': int(common_tag.attrib['greenChnl']),
        'blueChnl': int(common_tag.attrib['blueChnl'])
    }

    for elem in tree.iterfind('pages/page'):
        font_info['pages'].append({
            'id': int(elem.attrib['id']),
            'file': elem.attrib['file']
        })

    for char in tree.iterfind('chars/char'):
        font_info['chars'].append({
            'id': int(char.attrib['id']),
            'x': int(char.attrib['x']),
            'y': int(char.attrib['y']),
            'width': int(char.attrib['width']),
            'height': int(char.attrib['height']),
            'xoffset': int(char.attrib['xoffset']),
            'yoffset': int(char.attrib['yoffset']),
            'xadvance': int(char.attrib['xadvance']),
            'page': int(char.attrib['page']),
            'chnl': int(char.attrib['chnl'])
        })

    for char in tree.iterfind('kernings/kerning'):
        font_info['kernings'].append({
            'first': int(char.attrib['first']),
            'second': int(char.attrib['second']),
            'amount': int(char.attrib['amount'])
        })

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
                font_info[tag] = {
                    'face': text_dict['face'],
                    'size': int(text_dict['size']),
                    'bold': int(text_dict['bold']),
                    'italic': int(text_dict['italic']),
                    'charset': text_dict['charset'],
                    'unicode': int(text_dict['unicode']),
                    'stretchH': int(text_dict['stretchH']),
                    'smooth': int(text_dict['smooth']),
                    'aa':  int(text_dict['aa']),
                    'padding': [int(n) for n in text_dict['padding'].split(',')],
                    'spacing': [int(n) for n in text_dict['spacing'].split(',')],
                    'outline': int(text_dict['outline'])
                }
            if tag == 'common':
                font_info[tag] = {
                    'lineHeight': int(text_dict['lineHeight']),
                    'base': int(text_dict['base']),
                    'scaleW': int(text_dict['scaleW']),
                    'scaleH': int(text_dict['scaleH']),
                    'pages': int(text_dict['pages']),
                    'packed': int(text_dict['packed']),
                    'alphaChnl': int(text_dict['alphaChnl']),
                    'redChnl': int(text_dict['redChnl']),
                    'greenChnl': int(text_dict['greenChnl']),
                    'blueChnl': int(text_dict['blueChnl'])
                }
            elif tag == 'page':
                font_info['pages'].append({
                    'id': int(text_dict['id']),
                    'file': text_dict['file']
                })
            elif tag == 'char':
                font_info['chars'].append({
                    'id': int(text_dict['id']),
                    'x': int(text_dict['x']),
                    'y': int(text_dict['y']),
                    'width': int(text_dict['width']),
                    'height': int(text_dict['height']),
                    'xoffset': int(text_dict['xoffset']),
                    'yoffset': int(text_dict['yoffset']),
                    'xadvance': int(text_dict['xadvance']),
                    'page': int(text_dict['page']),
                    'chnl': int(text_dict['chnl'])
                })
            elif tag == 'kerning':
                font_info['kernings'].append({
                    'first': int(text_dict['first']),
                    'second': int(text_dict['second']),
                    'amount': int(text_dict['amount']),
                })
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
    font_name = ''
    i = 0
    while b[i] != 0:
        font_name += chr(b[i])
        i += 1
    d['face'] = font_name
    d['charset'] = '' if d['unicode'] else get_charset_name(d['charset'])
    result['info'] = d
    b = b[i + 1:]
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
    assert block_type == 3
    return result


def test_fnt_formats(font_exe, env):
    # clear_work_dir()
    args = [font_exe, '--font-file', 'fonts/FreeSans.ttf', '--output', 'generated/format_test',
            '--chars', '32-126', '--font-size', '16',
            '--include-kerning-pairs',
            '--padding-up', '1', '--padding-right', '2', '--padding-down', '3', '--padding-left', '4',
            '--spacing-horiz', '5', '--spacing-vert', '6']

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


if __name__ == '__main__':
    main(sys.argv)
