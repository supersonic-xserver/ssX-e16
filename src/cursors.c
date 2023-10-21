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

#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#include "E.h"
#include "conf.h"
#include "cursors.h"
#include "emodule.h"
#include "list.h"
#include "xwin.h"
#if USE_XRENDER
#include "eimage.h"
#endif

struct _ecursor {
    dlist_t         list;
    char           *name;
    EX_Cursor       cursor;
    unsigned int    ref_count;
    char           *file;
    unsigned int    bg;
    unsigned int    fg;
    int             native_id;
};

static          LIST_HEAD(cursor_list);

#if !USE_XRENDER

static          EX_Cursor
_ECursorCreateFromBitmapData(int w, int h, unsigned char *cdata,
                             unsigned char *cmask, int xh, int yh,
                             unsigned int fg, unsigned int bg)
{
    EX_Cursor       curs;
    Pixmap          pmap, mask;
    XColor          fgxc, bgxc;

    curs = NoXID;
    mask = NoXID;

    pmap = XCreateBitmapFromData(disp, WinGetXwin(VROOT), (char *)cdata, w, h);
    if (!pmap)
        goto done;

    if (cmask)
        mask =
            XCreateBitmapFromData(disp, WinGetXwin(VROOT), (char *)cmask, w, h);

    /* Looks like the pmap (not mask) bits in all theme cursors are inverted.
     * Fix by swapping fg and bg colors. */
    COLOR32_TO_RGB16(bg, fgxc.red, fgxc.green, fgxc.blue);
    COLOR32_TO_RGB16(fg, bgxc.red, bgxc.green, bgxc.blue);
    XAllocColor(disp, WinGetCmap(VROOT), &fgxc);
    XAllocColor(disp, WinGetCmap(VROOT), &bgxc);

    curs = XCreatePixmapCursor(disp, pmap, mask, &fgxc, &bgxc, xh, yh);

    EFreePixmap(pmap);
    if (mask)
        EFreePixmap(mask);

  done:
    return curs;
}

#endif                          /* !USE_XRENDER */

static          EX_Cursor
_ECursorCreateFromBitmaps(const char *img, unsigned int fg, unsigned int bg)
{
    EX_Cursor       curs;
    unsigned char  *cdata, *cmask;
    unsigned int    w, h, wm, hm;
    int             xh, yh;
    char            msk[FILEPATH_LEN_MAX];

    curs = NoXID;
    w = h = 0;
    xh = yh = 0;
    cdata = cmask = NULL;

    XReadBitmapFileData(img, &w, &h, &cdata, &xh, &yh);
    if (!cdata)
        goto done;
    XQueryBestCursor(disp, WinGetXwin(VROOT), w, h, &wm, &hm);
    if (w > wm || h > hm)
        goto done;

    Esnprintf(msk, sizeof(msk), "%s.mask", img);
    XReadBitmapFileData(msk, &wm, &hm, &cmask, NULL, NULL);
    if (cmask && (w != wm || h != hm))
    {
        /* Dimension mismatch - drop mask */
        XFree(cmask);
        cmask = NULL;
    }

    if (xh < 0 || xh >= (int)w)
        xh = w / 2;
    if (yh < 0 || yh >= (int)h)
        yh = h / 2;

#if USE_XRENDER
    curs = EImageCursorCreateFromBitmapData(w, h, cdata, cmask, xh, yh, fg, bg);
#else
    curs = _ECursorCreateFromBitmapData(w, h, cdata, cmask, xh, yh, fg, bg);
#endif

    if (cmask)
        XFree(cmask);
  done:
    if (cdata)
        XFree(cdata);

    return curs;
}

static void
_ECursorCreate(const char *name, const char *image, int native_id,
               unsigned int fg, unsigned int bg)
{
    ECursor        *ec;

    if ((!name) || (!image && native_id == -1))
        return;

    ec = ECALLOC(ECursor, 1);
    if (!ec)
        return;

    ec->name = Estrdup(name);

    ec->file = Estrdup(image);
    ec->fg = 0xff000000 | fg;
    ec->bg = 0xff000000 | bg;
    ec->native_id = native_id;

    LIST_PREPEND(ECursor, &cursor_list, ec);
}

static void
_ECursorDestroy(ECursor *ec)
{
    if (!ec)
        return;

    if (ec->ref_count > 0)
    {
        DialogOK("ECursor Error!", _("%u references remain"), ec->ref_count);
        return;
    }

    LIST_REMOVE(ECursor, &cursor_list, ec);

    Efree(ec->name);
    Efree(ec->file);

    Efree(ec);
}

static ECursor *
_ECursorRealize(ECursor *ec)
{
    char           *img;

    if (ec->file)
    {
        img = ThemeFileFind(ec->file, FILE_TYPE_CURSOR);
        EFREE_NULL(ec->file);   /* Ok or not - we never need file again */
        if (!img)
            goto done;

        ec->cursor = _ECursorCreateFromBitmaps(img, ec->fg, ec->bg);
        if (ec->cursor == NoXID)
        {
            Eprintf("*** Failed to create cursor \"%s\" from %s,%s.mask\n",
                    ec->name, img, img);
        }

        Efree(img);
    }
    else
    {
        ec->cursor = (ec->native_id == 999) ?
            None : XCreateFontCursor(disp, ec->native_id);
    }

  done:
    if (ec->cursor == NoXID)
    {
        _ECursorDestroy(ec);
        ec = NULL;
    }

    return ec;
}

static int
_ECursorMatchName(const void *data, const void *match)
{
    return strcmp(((const ECursor *)data)->name, (const char *)match);
}

static ECursor *
_ECursorFind(const char *name)
{
    if (!name || !name[0])
        return NULL;
    return LIST_FIND(ECursor, &cursor_list, _ECursorMatchName, name);
}

ECursor        *
ECursorAlloc(const char *name)
{
    ECursor        *ec;

    if (!name)
        return NULL;

    ec = _ECursorFind(name);
    if (!ec)
        return NULL;

    if (ec->cursor == NoXID)
        ec = _ECursorRealize(ec);
    if (!ec)
        return NULL;

    ec->ref_count++;

    return ec;
}

void
ECursorFree(ECursor *ec)
{
    if (ec)
        ec->ref_count--;
}

int
ECursorConfigLoad(FILE *fs)
{
    int             err = 0;
    unsigned int    cbg, cfg;
    char            s[FILEPATH_LEN_MAX];
    char            s2[FILEPATH_LEN_MAX];
    char           *p2;
    int             i1, i2, r, g, b;
    char            name[FILEPATH_LEN_MAX], *pname;
    char            file[FILEPATH_LEN_MAX], *pfile;
    int             native_id = -1;

    COLOR32_FROM_RGB(cbg, 0, 0, 0);
    COLOR32_FROM_RGB(cfg, 255, 255, 255);

    pname = pfile = NULL;

    while (GetLine(s, sizeof(s), fs))
    {
        i1 = ConfigParseline1(s, s2, &p2, NULL);
        switch (i1)
        {
        case CONFIG_CURSOR:
            err = -1;
            i2 = atoi(s2);
            if (i2 != CONFIG_OPEN)
                goto done;
            COLOR32_FROM_RGB(cbg, 0, 0, 0);
            COLOR32_FROM_RGB(cfg, 255, 255, 255);
            pname = pfile = NULL;
            native_id = -1;
            break;
        case CONFIG_CLOSE:
            _ECursorCreate(pname, pfile, native_id, cfg, cbg);
            err = 0;
            break;

        case CONFIG_CLASSNAME:
            if (_ECursorFind(s2))
            {
                SkipTillEnd(fs);
                goto done;
            }
            strcpy(name, s2);
            pname = name;
            break;
        case CURS_BG_RGB:
            r = g = b = 0;
            sscanf(p2, "%d %d %d", &r, &g, &b);
            COLOR32_FROM_RGB(cbg, r, g, b);
            break;
        case CURS_FG_RGB:
            r = g = b = 255;
            sscanf(p2, "%d %d %d", &r, &g, &b);
            COLOR32_FROM_RGB(cfg, r, g, b);
            break;
        case XBM_FILE:
            strcpy(file, s2);
            pfile = file;
            break;
        case NATIVE_ID:
            native_id = atoi(s2);
            break;
        default:
            break;
        }
    }

  done:
    if (err)
        ConfigAlertLoad("Cursor");

    return err;
}

void
ECursorApply(ECursor *ec, Win win)
{
    if (!ec)
        return;
    XDefineCursor(disp, WinGetXwin(win), ec->cursor);
}

static          EX_Cursor
_ECursorGetByName(const char *name, const char *name2, unsigned int fallback)
{
    ECursor        *ec;

    ec = ECursorAlloc(name);
    if (!ec && name2)
        ec = ECursorAlloc(name2);
    if (ec)
        return ec->cursor;

    return XCreateFontCursor(disp, fallback);
}

typedef struct {
    const char     *pri;
    const char     *sec;
    unsigned int    fallback;
} ECDataRec;

static const ECDataRec ECData[ECSR_COUNT] = {
    { "DEFAULT", NULL, XC_left_ptr },
    { "GRAB", NULL, XC_crosshair },
    { "PGRAB", NULL, XC_X_cursor },
    { "GRAB_MOVE", NULL, XC_fleur },
    { "GRAB_RESIZE", NULL, XC_sizing },
    { "RESIZE_H", NULL, XC_sb_h_double_arrow },
    { "RESIZE_V", NULL, XC_sb_v_double_arrow },
    { "RESIZE_TL", "RESIZE_BR", XC_top_left_corner },
    { "RESIZE_TR", "RESIZE_BL", XC_top_right_corner },
    { "RESIZE_BL", "RESIZE_TR", XC_bottom_left_corner },
    { "RESIZE_BR", "RESIZE_TL", XC_bottom_right_corner },
};

static EX_Cursor ECsrs[ECSR_COUNT] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

EX_Cursor
ECsrGet(int which)
{
    if (which < 0 || which >= ECSR_COUNT)
        return NoXID;
    if (ECsrs[which] == 1)
        ECsrs[which] = _ECursorGetByName(ECData[which].pri, ECData[which].sec,
                                         ECData[which].fallback);

    return ECsrs[which];
}

void
ECsrApply(int which, EX_Window win)
{
    XDefineCursor(disp, win, ECsrGet(which));
}

static void
_CursorsIpc(const char *params)
{
    const char     *p;
    char            cmd[128], prm[4096];
    int             len;
    ECursor        *ec;

    cmd[0] = prm[0] = '\0';
    p = params;
    if (p)
    {
        len = 0;
        sscanf(p, "%100s %4000s %n", cmd, prm, &len);
    }

    if (!strncmp(cmd, "list", 2))
    {
        LIST_FOR_EACH(ECursor, &cursor_list, ec) IpcPrintf("%s\n", ec->name);
    }
}

static const IpcItem CursorIpcArray[] = {
    {
     _CursorsIpc,
     "cursor", "csr",
     "Cursor functions",
     "  cursor list                       Show all cursors\n" }
};

/*
 * Module descriptor
 */
extern const EModule ModCursors;

const EModule   ModCursors = {
    "cursor", "csr",
    NULL,
    MOD_ITEMS(CursorIpcArray),
    { 0, NULL }
};
