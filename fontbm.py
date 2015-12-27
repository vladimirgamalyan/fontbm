import subprocess
import os


FONT_BM_EXE = 'fontbm'


# Usage: fontbm.fontbm('FreeSans.ttf', 'test', data_format='xml', font_size=4, color=(255,0,255))

def fontbm(font_file, output, *args, **kwargs):
    arg = [FONT_BM_EXE, '--font-file', font_file, '--output', output]
    for k, v in kwargs.iteritems():
        if k == 'color' or k == 'background_color':
            v = ','.join(map(str,v))
        arg.extend(['--' + k.replace('_', '-'), str(v)])
    env = os.environ.copy()
    env['PATH'] = r'C:\msys64\mingw32\bin;' + env['PATH']
    subprocess.check_call(arg, env=env)
