/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2022 Kim Woelders
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
#include "config.h"
#include "tclass.h"

#if FONT_TYPE_XFS
#include <X11/Xlib.h>

#include "E.h"

static          GC
_get_gc(Win win)
{
    static GC       gc = NULL;
    static Visual  *last_vis = NULL;
    Visual         *vis;

    vis = WinGetVisual(win);
    if (vis != last_vis)
    {
        if (gc)
            EXFreeGC(gc);
        gc = NULL;
        last_vis = vis;
    }

    if (!gc)
        gc = EXCreateGC(WinGetXwin(win), 0, NULL);

    return gc;
}

/*
 * XFontSet - XCreateFontSet
 */
__EXPORT__ extern const FontOps FontOps_xfs;

typedef struct {
    XFontSet        font;
    int             ascent;
    Win             win;
    EX_Drawable     draw;
    GC              gc;
} FontCtxXfs;

static int
_xfs_Load(TextState *ts, const char *name)
{
    XFontSet        font;
    FontCtxXfs     *fdc;
    int             i, missing_cnt, font_cnt;
    char          **missing_list, *def_str, **fnlr;
    XFontStruct   **fs;

    font = XCreateFontSet(disp, name, &missing_list, &missing_cnt, &def_str);
    if (missing_cnt)
        XFreeStringList(missing_list);
    if (!font)
        return -1;

    if (EDebug(EDBUG_TYPE_FONTS) >= 2)
    {
        Eprintf("- XBaseFontNameListOfFontSet %s\n",
                XBaseFontNameListOfFontSet(font));
        font_cnt = XFontsOfFontSet(font, &fs, &fnlr);
        for (i = 0; i < font_cnt; i++)
            Eprintf("- XFontsOfFontSet %d: %s\n", i, fnlr[i]);
    }

    fdc = EMALLOC(FontCtxXfs, 1);
    if (!fdc)
        return -1;
    fdc->font = font;
    ts->fdc = fdc;
    fdc->ascent = 0;
    font_cnt = XFontsOfFontSet(font, &fs, &fnlr);
    for (i = 0; i < font_cnt; i++)
        fdc->ascent = MAX(fs[i]->ascent, fdc->ascent);
    ts->type = FONT_TYPE_XFS;
    ts->ops = &FontOps_xfs;
    return 0;
}

static void
_xfs_Unload(TextState *ts)
{
    FontCtxXfs     *fdc = (FontCtxXfs *) ts->fdc;

    XFreeFontSet(disp, fdc->font);
}

static void
_xfs_TextSize(TextState *ts, const char *text, int len,
              int *width, int *height, int *ascent)
{
    FontCtxXfs     *fdc = (FontCtxXfs *) ts->fdc;
    XRectangle      ret2;

    if (len == 0)
        len = strlen(text);
    XmbTextExtents(fdc->font, text, len, NULL, &ret2);
    *height = ret2.height;
    *width = ret2.width;
    *ascent = fdc->ascent;
}

static void
_xfs_TextDraw(TextState *ts, int x, int y, const char *text, int len)
{
    FontCtxXfs     *fdc = (FontCtxXfs *) ts->fdc;

    XmbDrawString(disp, fdc->draw, fdc->font, fdc->gc, x, y, text, len);
}

static int
_xfs_FdcInit(TextState *ts, Win win, EX_Drawable draw)
{
    FontCtxXfs     *fdc = (FontCtxXfs *) ts->fdc;

    fdc->win = win;
    fdc->draw = draw;
    fdc->gc = _get_gc(win);
    return 0;
}

static void
_xfs_FdcSetDrawable(TextState *ts, unsigned long draw)
{
    FontCtxXfs     *fdc = (FontCtxXfs *) ts->fdc;

    fdc->draw = draw;
}

static void
_xfs_FdcSetColor(TextState *ts, unsigned int color)
{
    FontCtxXfs     *fdc = (FontCtxXfs *) ts->fdc;
    unsigned int    pixel;

    pixel = EAllocColor(WinGetCmap(fdc->win), color);
    XSetForeground(disp, fdc->gc, pixel);
}

const FontOps   FontOps_xfs = {
    _xfs_Load, _xfs_Unload, _xfs_TextSize, TextstateTextFit, _xfs_TextDraw,
    _xfs_FdcInit, NULL, _xfs_FdcSetDrawable, _xfs_FdcSetColor
};

#endif                          /* FONT_TYPE_XFS */
