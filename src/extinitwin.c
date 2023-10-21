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
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#define USE_EIWC_WINDOW 1
#if USE_XRENDER
#define USE_EIWC_RENDER 1
#include <X11/extensions/Xrender.h>
#endif

#include "E.h"
#include "eimage.h"
#include "events.h"
#include "xprop.h"
#include "xwin.h"

typedef struct {
    EX_Cursor       curs;
    XSetWindowAttributes attr;
    EX_Window       cwin;
} EiwData;

typedef void    (EiwLoopFunc) (EX_Window win, EImage * im, EiwData * d);

#if USE_EIWC_RENDER
static void     _eiw_render_loop(EX_Window win, EImage * im, EiwData * d);

static EiwLoopFunc *
_eiw_render_init(EX_Window win __UNUSED__, EiwData *d)
{
    Visual         *vis;

    /* Quit if no ARGB visual.
     * If we have, assume(?) a colored XRenderCreateCursor is available. */
    vis = EVisualFindARGB();
    if (!vis)
        return NULL;

    d->curs = NoXID;

    return _eiw_render_loop;
}

static void
_eiw_render_loop(EX_Window win, EImage *im, EiwData *d)
{
    int             w, h;

    EImageGetSize(im, &w, &h);

    if (d->curs != NoXID)
        XFreeCursor(disp, d->curs);
    d->curs = EImageDefineCursor(im, w / 2, h / 2);

    XDefineCursor(disp, win, d->curs);
}

#endif                          /* USE_EIWC_RENDER */

#if USE_EIWC_WINDOW

static void     _eiw_window_loop(EX_Window win, EImage * im, EiwData * d);

static          EX_Cursor
_DefineNullCursor(EX_Window win)
{
    EX_Cursor       curs;
    EX_Pixmap       pmap, mask;
    XColor          cl = { };

    pmap = XCreatePixmap(disp, win, 16, 16, 1);
    EXFillAreaSolid(pmap, 0, 0, 16, 16, 0);
    mask = pmap;

    curs = XCreatePixmapCursor(disp, pmap, mask, &cl, &cl, 0, 0);

    XFreePixmap(disp, pmap);

    return curs;
}

static EiwLoopFunc *
_eiw_window_init(EX_Window win, EiwData *d)
{
    d->cwin = XCreateWindow(disp, WinGetXwin(RROOT), 0, 0, 32, 32, 0,
                            CopyFromParent, InputOutput, CopyFromParent,
                            CWOverrideRedirect | CWBackingStore | CWBackPixel,
                            &d->attr);

    d->curs = _DefineNullCursor(d->cwin);

    XDefineCursor(disp, win, d->curs);
    XDefineCursor(disp, d->cwin, d->curs);

    return _eiw_window_loop;
}

static void
_eiw_window_loop(EX_Window win, EImage *im, EiwData *d)
{
    EX_Pixmap       pmap, mask;
    Window          ww;
    int             dd, x, y, w, h;
    unsigned int    mm;

    EImageRenderPixmaps(im, RROOT, 0, &pmap, &mask, 0, 0);
    EImageGetSize(im, &w, &h);
    XShapeCombineMask(disp, d->cwin, ShapeBounding, 0, 0, mask, ShapeSet);
    XSetWindowBackgroundPixmap(disp, d->cwin, pmap);
    EImagePixmapsFree(pmap, mask);
    XClearWindow(disp, d->cwin);
    XQueryPointer(disp, win, &ww, &ww, &dd, &dd, &x, &y, &mm);
    XMoveResizeWindow(disp, d->cwin, x - w / 2, y - h / 2, w, h);
    XMapWindow(disp, d->cwin);
}

#endif                          /* USE_EIWC_WINDOW */

static          EX_Window
ExtInitWinMain(void)
{
    int             i, loop, err;
    EX_Window       win;
    EX_Pixmap       pmap;
    EX_Atom         a;
    EiwData         eiwd;
    EiwLoopFunc    *eiwc_loop_func;
    unsigned int    wclass, vmask;

    if (EDebug(EDBUG_TYPE_SESSION))
        Eprintf("%s: enter\n", __func__);

    err = EDisplayOpen(NULL, -1);
    if (err)
        return NoXID;

    EDisplaySetErrorHandlers(EventShowError, NULL);

    EGrabServer();

    EImageInit();

    eiwd.attr.override_redirect = True;
    eiwd.attr.background_pixmap = NoXID;
    eiwd.attr.backing_store = NotUseful;
    eiwd.attr.save_under = True;
    eiwd.attr.background_pixel = 0;     /* Not used */
    wclass = InputOnly;
    vmask = CWOverrideRedirect;
    if (!Mode.wm.window)
    {
        pmap = ECreatePixmap(RROOT, WinGetW(RROOT), WinGetH(RROOT), 0);
        EXCopyArea(WinGetXwin(RROOT), pmap, 0, 0,
                   WinGetW(RROOT), WinGetH(RROOT), 0, 0);
        eiwd.attr.background_pixmap = pmap;
        wclass = InputOutput;
        vmask |= CWBackPixmap | CWBackingStore | CWSaveUnder;
    }
    win = XCreateWindow(disp, WinGetXwin(RROOT),
                        0, 0, WinGetW(RROOT), WinGetH(RROOT),
                        0, CopyFromParent, wclass, CopyFromParent,
                        vmask, &eiwd.attr);
    if (!Mode.wm.window)
    {
        EFreePixmap(eiwd.attr.background_pixmap);
    }

    XMapRaised(disp, win);

    a = ex_atom_get("ENLIGHTENMENT_RESTART_SCREEN");
    ex_window_prop_window_set(WinGetXwin(RROOT), a, &win, 1);

    XSelectInput(disp, win, StructureNotifyMask);

    EUngrabServer();
    ESync(0);

#if USE_EIWC_WINDOW && USE_EIWC_RENDER
    eiwc_loop_func = _eiw_render_init(win, &eiwd);
    if (!eiwc_loop_func)
        eiwc_loop_func = _eiw_window_init(win, &eiwd);
#elif USE_EIWC_RENDER
    eiwc_loop_func = _eiw_render_init(win, &eiwd);
#elif USE_EIWC_WINDOW
    eiwc_loop_func = _eiw_window_init(win, &eiwd);
#endif
    if (!eiwc_loop_func)
        return NoXID;

    {
        XWindowAttributes xwa;
        char            s[1024];
        EImage         *im;

        for (i = loop = 1;; i++, loop++)
        {
            if (i > 12)
                i = 1;

            /* If we get unmapped we are done */
            if (!XGetWindowAttributes(disp, win, &xwa) ||
                (xwa.map_state == IsUnmapped))
            {
                if (EDebug(EDBUG_TYPE_SESSION))
                    Eprintf("%s: child done\n", __func__);
                break;
            }

            Esnprintf(s, sizeof(s), "pix/wait%i.png", i);
            if (EDebug(EDBUG_TYPE_SESSION) > 1)
                Eprintf("%s: child %s\n", __func__, s);

            im = ThemeImageLoad(s);
            if (im)
            {
                eiwc_loop_func(win, im, &eiwd);
                EImageFree(im);
            }
            ESync(0);
            SleepUs(50 * 1000);

            /* If we still are here after 60 sec something is wrong. */
            if (loop > 60000 / 50)
            {
                if (EDebug(EDBUG_TYPE_SESSION))
                    Eprintf("%s: child timeout\n", __func__);
                break;
            }
        }
    }

    EDisplayClose();

    exit(0);
}

EX_Window
ExtInitWinCreate(void)
{
    EX_Window       win_ex;     /* Hmmm.. */
    EX_Window       win;
    EX_Atom         a;

    if (EDebug(EDBUG_TYPE_SESSION))
        Eprintf("%s\n", __func__);

    a = ex_atom_get("ENLIGHTENMENT_RESTART_SCREEN");
    ex_window_prop_window_set(WinGetXwin(RROOT), a, NULL, 0);
    ESync(0);

    if (fork())
    {
        /* Parent */
        EUngrabServer();

        for (;;)
        {
            if (EDebug(EDBUG_TYPE_SESSION))
                Eprintf("%s: parent\n", __func__);

            /* Hack to give the child some space. Not foolproof. */
            SleepUs(500000);

            if (ex_window_prop_window_get(WinGetXwin(RROOT), a, &win_ex, 1) > 0)
                break;
        }

        win = win_ex;
        if (EDebug(EDBUG_TYPE_SESSION))
            Eprintf("%s: parent - %#x\n", __func__, win);

        return win;
    }

    /* Child - Create the init window */

    if (EDebug(EDBUG_TYPE_SESSION))
        Eprintf("%s: child\n", __func__);

    /* Clean up inherited stuff */

    SignalsRestore();

    EImageExit(0);
    EDisplayDisconnect();

    ExtInitWinMain();

    /* We will never get here */
    return NoXID;
}

static EX_Window init_win_ext = NoXID;

void
ExtInitWinSet(EX_Window win)
{
    init_win_ext = win;
}

EX_Window
ExtInitWinGet(void)
{
    return init_win_ext;
}

void
ExtInitWinKill(void)
{
    if (!disp || init_win_ext == NoXID)
        return;

    if (EDebug(EDBUG_TYPE_SESSION))
        Eprintf("%s: %#x\n", __func__, init_win_ext);
    XUnmapWindow(disp, init_win_ext);
    init_win_ext = NoXID;
}
