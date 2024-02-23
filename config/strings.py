#!/usr/bin/python3
#
# Script to extract localizable strings from config files
# $ (cd config && ./strings.py *.cfg *.menu >strings.c)

import sys

ttt = ''
tat = ''
tdt = ''


def quote1(line):
    return line.split('"')[1]

def do_line_cfg(line):
    global ttt, tat
    global tdt

    # actionclasses.cfg
    TTT = 0
    TAT = 0

    if line.find('__TOOLTIP_TEXT') >= 0:
        txt = quote1(line)
        if ttt:
            ttt += '\\n' + txt
        else:
            ttt = txt
        TTT = 1
    elif line.find('__TOOLTIP_ACTION_TEXT') > 0:
        txt = quote1(line)
        tat = txt
        TAT = 1

    if TTT == 0 and ttt:
        print(f'    _("{ttt}"),')
        ttt = ''

    if TAT == 0 and tat:
        print(f'    _("{tat}"),')
        tat = ''

    # bindings.cfg
    TDT = 0

    if line.startswith('Tooltip'):
        line = line.strip()
        tok = line.split(' ', 1)
        if tdt:
            tdt += '\\n' + tok[1]
        else:
            tdt = tok[1]
        TDT = 1

    if TDT == 0 and tdt:
        print(f'    _("{tdt}"),')
        tdt = ''

    # menus.cfg (obsolete)
    if line.find('ADD_MENU_TITLE') >= 0:
        txt = quote1(line)
        print(f'    _("{txt}"),')
    elif line.find('ADD_MENU_TEXT_ITEM') >= 0 or \
            line.find('ADD_MENU_SUBMENU_TEXT_ITEM') >= 0:
        txt = quote1(line)
        print(f'    _("{txt}"),')


def do_line_menu(line):
    # *.menu
    if line.startswith('"'):
        txt = quote1(line)
        print(f'    _("{txt}"),')


# From e_gen_menu + misc
sl = [
    'User menus',
    'User application list',
    'Applications',
    'Epplets',
    'Restart',
    'Log out',

    'Help',
]

#
# Start
#
print('#define _(x) x\n')
print('const char     *txt[] = {')


for arg in sys.argv[1:]:
    print(f'/* {arg} */')
    f = open(arg, 'r')
    if arg.endswith('cfg'):
        for line in f:
            do_line_cfg(line)
    elif arg.endswith('menu'):
        for line in f:
            do_line_menu(line)
    f.close()

# Other strings.
print('')
for str in sl:
    print(f'    _("{str}"),')

print('};')
