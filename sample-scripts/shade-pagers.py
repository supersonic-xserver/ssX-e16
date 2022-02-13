#!/usr/bin/python3

# This is a hack of mandrake's "testroller.pl" that shades/unshades all your
# pager windows

import os, sys
import subprocess

argv = sys.argv[1:]

shade = ''
if len(argv) > 0:
    if argv[0] in ['0', 'off']:
        shade = 'off'
    elif argv[0] in ['1', 'on']:
        shade = 'on'

cmd = f'win_op Pager* shade {shade}'
#print(cmd)

if True:
    p = os.popen('eesh', 'w')
    p.write(f'{cmd}\n')
    p.close()

if False:
    subprocess.run(['eesh', '-c', f'{cmd}\n'])

# Alternatively, simply do
#$ eesh wop Pager* shade [on|off]
