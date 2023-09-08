#!/usr/bin/python3

# This is a little script that will create a window that says
# "Follow the Bouncing Ball" and then drops it to the bottom of the
# screen and slowly bounces it around.
# Then when it's done bouncing it gets rid of it.

import os
import re
import subprocess
import time


def EeshExec(cmd):
    subprocess.run(['eesh'] + cmd.split())


def EeshCall(cmd):
    sp = subprocess.run(['eesh', '-e'] + cmd.split(), capture_output=True, encoding='utf8')
    out = sp.stdout.strip()
#   print(f'out = "{out}"')
    return out


re1 = re.compile(r'.+\s+(\d+)x(\d+)')           # Screen 0  size 1920x1080
out = EeshCall('screen size')
match = re1.match(out)
if match:
    width  = int(match.group(1))
    height = int(match.group(2))

EeshExec('dialog_ok "Follow the Bouncing Ball"')

ball = 'Message'

re1 = re.compile(r'.+:\s+(\d+)\s+(\d+)')        # window location: 868 498
out = EeshCall(f'win_op {ball} move ?')
match = re1.match(out)
if match:
    ballx = int(match.group(1))
    bally = int(match.group(2))

re1 = re.compile(r'.+:\s+(\d+)\s+(\d+)')        # window location: 868 498
out = EeshCall(f'win_op {ball} size ??')
match = re1.match(out)
if match:
    ballw = int(match.group(1))
    ballh = int(match.group(2))

#print(f'x,y={ballx},{bally} wxh={ballw} x {ballh}')

# now for the fun part.  make that baby bounce up and down.
# we're going to open a big pipe for this one and just shove data
# to it.

fallspeeds = [30, 25, 20, 15, 10, 5, 4, 3, 2]
n = len(fallspeeds)
n1 = n - 1

p = os.popen('eesh', 'w')

for i in range(n):
    originalbally = bally
    fallspeed = fallspeeds[i]
#   print(f'loop {i} x,y {ballx},{bally} fall {fallspeed}')
    while bally < height - ballh:
        if bally + fallspeed + ballh < height:
            bally += fallspeed
        else:
            bally = height - ballh
#       print(f'win_op {ball} move {ballx} {bally}')
        p.write(f'win_op {ball} move {ballx} {bally}\n')
        p.flush()
        time.sleep(.020)

#   print('reverse')
    if i < n - 1:
        fallspeed = fallspeeds[i + 1]
    else:
        fallspeed = 1

    while bally > originalbally + int(originalbally * (1. / n1)):
        if bally - fallspeed > originalbally + int(originalbally * (1. / n1)):
            bally -= fallspeed
        else:
            bally = originalbally + int(originalbally * (1. / n1))
#       print(f'win_op {ball} move {ballx} {bally}')
        p.write(f'win_op {ball} move {ballx} {bally}\n')
        p.flush()
        time.sleep(.020)

p.write(f'win_op {ball} close\n')
p.close()

# that's all folks.
