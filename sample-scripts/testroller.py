#!/usr/bin/python3

# This app will take the parameters "on" and "off", and will basically
# shade or unshade all the windows on the current desktop and area.

import os, sys, re
import subprocess


def EeshExec(cmd):
    subprocess.run(["eesh", cmd])


def EeshCall(cmd):
    sp = subprocess.run(["eesh", "-e", cmd], capture_output=True, encoding='utf8')
    out = sp.stdout.strip()
#   print(f'out = "{out}"')
    return out


argv = sys.argv[1:]

shade = ''
if len(argv) > 0:
    if argv[0] in ['0', 'off']:
        shade = 'off'
    elif argv[0] in ['1', 'on']:
        shade = 'on'

# Get the current desk we're on
out = EeshCall('desk')
re1 = re.compile(r'.+:\s+(\d+)/(\d+)')          # Current Desktop: 0/3
match = re1.match(out)
if match:
    desk_cur = int(match.group(1))
    desk_cnt = int(match.group(2))
#print(f'Desk: {desk_cur} of {desk_cnt}')

# Get the current area we're on
out = EeshCall('area')
re1 = re.compile(r'.+:\s+(\d+)\s+(\d+)')        # Current Area: 2 0
match = re1.match(out)
if match:
    area_x = int(match.group(1))
    area_y = int(match.group(2))
area_cur = f'{area_x} {area_y}'
#print(f'Area: {area_x}, {area_y}: "{area_cur}"')

# Get the old shadespeed so that we can set it back later
# because we want this to happen fairly quickly, we'll set
# the speed to something really high
out = EeshCall('show misc.shading.speed')       # misc.shading.speed = 8000
shadespeed = int(out.split('=')[1])
#print(f'shadespeed = {shadespeed}')

# Get the window list
winlist = EeshCall('wl a')

p = os.popen('eesh', 'w')

p.write(f'set misc.shading.speed 10000000\n')
p.flush()

# Now walk through each of the windows and shade/unshade them

for win in winlist.split('\n'):
    tok = win.split(':')
    tok = [t.strip() for t in tok]
    if len(tok) < 6:
        continue
    window = tok[0]
    desk = int(tok[3])
    area = tok[4]
    name = tok[5]
#   print([window, desk, area, name, ':', desk_cur, area_cur])

    # Skip pagers, iconboxes, systrays, and epplets
    if name.startswith('Pager-') or name.startswith('E-') or \
            name.startswith('Iconbox') or name.startswith('Systray'):
        continue
    if desk != desk_cur:
        continue
    if area != area_cur:
        continue
    print(f'win_op {window} shade {shade}')
    p.write(f'win_op {window} shade {shade}\n')

# Set the shade speed back to what it was originally
p.write(f'set misc.shading.speed {shadespeed}\n')
#p.flush()
p.close()

# that's it!
