/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2020 Kim Woelders
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _BORDERS_H_
#define _BORDERS_H_

#include "eimage.h"
#include "etypes.h"
#include "xtypes.h"

Border         *BorderFind(const char *name);
const char     *BorderGetName(const Border * b);
int             BorderCanShade(const Border * b);
const EImageBorder *BorderGetSize(const Border * b);
int             BorderGetShadedir(const Border * b);
ActionClass    *BorderGetAclass(const Border * b);

int             BorderConfigLoad(FILE * fs);

void            EwinBorderSelect(EWin * ewin);
void            EwinBorderDetach(EWin * ewin);
void            EwinBorderSetTo(EWin * ewin, const Border * b);
void            EwinBorderDraw(EWin * ewin, int do_shape, int do_paint);
void            EwinBorderCalcSizes(EWin * ewin, int propagate);
void            EwinBorderMinShadeSize(const EWin * ewin, int *mw, int *mh);
void            EwinBorderUpdateInfo(EWin * ewin);
void            EwinBorderChange(EWin * ewin, const Border * b, int normal);
void            EwinBorderSetInitially(EWin * ewin, const char *name);
const Border   *EwinBorderGetGroupBorder(const EWin * ewin);
int             BorderWinpartIndex(EWin * ewin, Win win);
void            BorderCheckState(EWin * ewin, XEvent * ev);

Border         *BorderCreateFiller(int w, int h, int sw, int sh);
Border        **BordersGetList(int *pnum);

#endif                          /* _BORDERS_H_ */
