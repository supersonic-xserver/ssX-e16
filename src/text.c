/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2025 Kim Woelders
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

#include "E.h"
#include "eimage.h"
#include "tclass.h"
#include "xwin.h"

static void
TextDrawRotTo(Win win, EX_Drawable src, EX_Drawable dst, int x, int y,
              int w, int h, TextState *ts)
{
    EImage         *im;
    int             win_w;

    switch (ts->style.orientation)
    {
    case FONT_TO_UP:
        im = EImageGrabDrawable(src, 0, y, x, h, w, 0);
        EImageOrientate(im, 1);
        EImageRenderOnDrawable(im, win, dst, 0, 0, 0, w, h);
        EImageFree(im);
        break;
    case FONT_TO_DOWN:
        EXGetSize(src, &win_w, NULL);
        im = EImageGrabDrawable(src, NoXID, win_w - y - h, x, h, w, 0);
        EImageOrientate(im, 3);
        EImageRenderOnDrawable(im, win, dst, 0, 0, 0, w, h);
        EImageFree(im);
        break;
    case FONT_TO_LEFT:         /* Holy carumba! That's for yoga addicts, maybe .... */
        im = EImageGrabDrawable(src, NoXID, x, y, w, h, 0);
        EImageOrientate(im, 2);
        EImageRenderOnDrawable(im, win, dst, 0, 0, 0, w, h);
        EImageFree(im);
        break;
    default:
        break;
    }
}

static void
TextDrawRotBack(Win win, EX_Drawable dst, EX_Drawable src, int x, int y,
                int w, int h, TextState *ts)
{
    EImage         *im;
    int             win_w;

    switch (ts->style.orientation)
    {
    case FONT_TO_UP:
        im = EImageGrabDrawable(src, NoXID, 0, 0, w, h, 0);
        EImageOrientate(im, 3);
        EImageRenderOnDrawable(im, win, dst, 0, y, x, h, w);
        EImageFree(im);
        break;
    case FONT_TO_DOWN:
        EXGetSize(dst, &win_w, NULL);
        im = EImageGrabDrawable(src, NoXID, 0, 0, w, h, 0);
        EImageOrientate(im, 1);
        EImageRenderOnDrawable(im, win, dst, 0, win_w - y - h, x, h, w);
        EImageFree(im);
        break;
    case FONT_TO_LEFT:         /* Holy carumba! That's for yoga addicts, maybe .... */
        im = EImageGrabDrawable(src, NoXID, 0, 0, w, h, 0);
        EImageOrientate(im, 2);
        EImageRenderOnDrawable(im, win, dst, 0, x, y, w, h);
        EImageFree(im);
        break;
    default:
        break;
    }
}

#if FONT_TYPE_IFT
static EImage  *
TextImageGet(Win win __UNUSED__, EX_Drawable src, int x, int y, int w, int h,
             TextState *ts)
{
    EImage         *im;
    int             win_w;

    switch (ts->style.orientation)
    {
    default:
    case FONT_TO_RIGHT:
        im = EImageGrabDrawable(src, NoXID, x, y, w, h, 0);
        break;
    case FONT_TO_LEFT:
        im = EImageGrabDrawable(src, NoXID, x, y, w, h, 0);
        EImageOrientate(im, 2);
        break;
    case FONT_TO_UP:
        im = EImageGrabDrawable(src, 0, y, x, h, w, 0);
        EImageOrientate(im, 1);
        break;
    case FONT_TO_DOWN:
        EXGetSize(src, &win_w, NULL);
        im = EImageGrabDrawable(src, NoXID, win_w - y - h, x, h, w, 0);
        EImageOrientate(im, 3);
        break;
    }

    return im;
}

static void
TextImagePut(EImage *im, Win win, EX_Drawable dst, int x, int y,
             int w, int h, TextState *ts)
{
    int             win_w;

    switch (ts->style.orientation)
    {
    default:
    case FONT_TO_RIGHT:
        EImageRenderOnDrawable(im, win, dst, 0, x, y, w, h);
        break;
    case FONT_TO_LEFT:
        EImageOrientate(im, 2);
        EImageRenderOnDrawable(im, win, dst, 0, x, y, w, h);
        break;
    case FONT_TO_UP:
        EImageOrientate(im, 3);
        EImageRenderOnDrawable(im, win, dst, 0, y, x, h, w);
        break;
    case FONT_TO_DOWN:
        EXGetSize(dst, &win_w, NULL);
        EImageOrientate(im, 1);
        EImageRenderOnDrawable(im, win, dst, 0, win_w - y - h, x, h, w);
        break;
    }
    EImageFree(im);
}
#endif                          /* FONT_TYPE_IFT */

TextState      *
TextclassGetTextState(TextClass *tclass, int state, int active, int sticky)
{
    if (!tclass)
        return NULL;

    if (active)
    {
        if (!sticky)
        {
            switch (state)
            {
            case STATE_NORMAL:
                return tclass->active.normal;
            case STATE_HILITED:
                return tclass->active.hilited;
            case STATE_CLICKED:
                return tclass->active.clicked;
            case STATE_DISABLED:
                return tclass->active.disabled;
            default:
                break;
            }
        }
        else
        {
            switch (state)
            {
            case STATE_NORMAL:
                return tclass->sticky_active.normal;
            case STATE_HILITED:
                return tclass->sticky_active.hilited;
            case STATE_CLICKED:
                return tclass->sticky_active.clicked;
            case STATE_DISABLED:
                return tclass->sticky_active.disabled;
            default:
                break;
            }

        }
    }
    else if (sticky)
    {
        switch (state)
        {
        case STATE_NORMAL:
            return tclass->sticky.normal;
        case STATE_HILITED:
            return tclass->sticky.hilited;
        case STATE_CLICKED:
            return tclass->sticky.clicked;
        case STATE_DISABLED:
            return tclass->sticky.disabled;
        default:
            break;
        }
    }
    else
    {
        switch (state)
        {
        case STATE_NORMAL:
            return tclass->norm.normal;
        case STATE_HILITED:
            return tclass->norm.hilited;
        case STATE_CLICKED:
            return tclass->norm.clicked;
        case STATE_DISABLED:
            return tclass->norm.disabled;
        default:
            break;
        }
    }
    return NULL;
}

static void
TextstateTextFit1(TextState *ts, char **ptext, int *pw, int textwidth_limit)
{
    char           *text = *ptext;
    int             width, hh, ascent, dw;
    char           *new_line;
    int             i, nuke_count, nc2;
    int             len;

    len = strlen(text);
    if (len <= 1)
        return;

    new_line = EMALLOC(char, len + 10);
    if (!new_line)
        return;

    width = *pw;
    nuke_count = ((width - textwidth_limit) * len) / width + 3;

    for (i = 0; i < 10; i++, nuke_count++)
    {
        if (nuke_count >= len - 1)
        {
            new_line[0] = text[0];
            memcpy(new_line + 1, "...", 4);
            break;
        }

        nc2 = (len - nuke_count) / 2;

        memcpy(new_line, text, nc2);
        memcpy(new_line + nc2, "...", 3);
        strcpy(new_line + nc2 + 3, text + nc2 + nuke_count);

        ts->ops->TextSize(ts, new_line, 0, pw, &hh, &ascent);

        width = *pw;
        dw = textwidth_limit - width;
        if (dw >= 0)
            break;
    }

    Efree(text);
    *ptext = new_line;
}

static void
TextstateTextFitMB(TextState *ts, char **ptext, int *pw, int textwidth_limit)
{
    char           *text = *ptext;
    int             width, hh, ascent, dw;
    char           *new_line;
    int             i, nuke_count, nc2;
    int             len, len_mb;
    wchar_t        *wc_line = NULL;
    int             wc_len;

    if (EwcOpen(ts->need_utf8 || Mode.locale.utf8_int))
        return;

    len = strlen(text);
    wc_len = EwcStrToWcs(text, len, NULL, 0);
    if (wc_len <= 1)
        goto done;

    wc_line = EMALLOC(wchar_t, wc_len + 1);
    if (!wc_line)
        goto done;

    if (EwcStrToWcs(text, len, wc_line, wc_len) <= 0)
        goto done;

    new_line = EMALLOC(char, len + 10);
    if (!new_line)
        goto done;

    width = *pw;
    nuke_count = ((width - textwidth_limit) * wc_len) / width + 3;

    for (i = 0; i < 10; i++, nuke_count++)
    {
        if (nuke_count >= wc_len - 1)
        {
            len_mb = EwcWcsToStr(wc_line, 1, new_line, MB_CUR_MAX);
            if (len_mb < 0)
                len_mb = 1;

            strcpy(new_line + len_mb, "...");
            break;
        }

        nc2 = (wc_len - nuke_count) / 2;

        len_mb = EwcWcsToStr(wc_line, nc2, new_line, len + 10);
        memcpy(new_line + len_mb, "...", 3);
        len_mb += 3;
        len_mb += EwcWcsToStr(wc_line + nc2 + nuke_count,
                              wc_len - nc2 - nuke_count,
                              new_line + len_mb, len + 10 - len_mb);
        new_line[len_mb] = '\0';

        ts->ops->TextSize(ts, new_line, 0, pw, &hh, &ascent);

        width = *pw;
        dw = textwidth_limit - width;
        if (dw >= 0)
            break;
    }

    Efree(text);
    *ptext = new_line;
  done:
    Efree(wc_line);
    EwcClose();
}

static void
TsTextDraw(TextState *ts, int x, int y, const char *text, int len)
{
    if (ts->style.effect == 1)
    {
        ts->ops->FdcSetColor(ts, ts->bg_col);
        ts->ops->TextDraw(ts, x + 1, y + 1, text, len);
    }
    else if (ts->style.effect == 2)
    {
        ts->ops->FdcSetColor(ts, ts->bg_col);
        ts->ops->TextDraw(ts, x - 1, y, text, len);
        ts->ops->TextDraw(ts, x + 1, y, text, len);
        ts->ops->TextDraw(ts, x, y - 1, text, len);
        ts->ops->TextDraw(ts, x, y + 1, text, len);
    }
    ts->ops->FdcSetColor(ts, ts->fg_col);
    ts->ops->TextDraw(ts, x, y, text, len);
}

typedef struct {
    const char     *type;
    const FontOps  *ops;
#if USE_MODULES
    char            checked;
#endif
} FontHandler;

#if FONT_TYPE_XFS
extern const FontOps FontOps_xfs;
#endif

#if USE_MODULES

#define FONT(type, ops, opsm) { type, opsm, 0 }

#else

#define FONT(type, ops, opsm) { type, ops }

#if FONT_TYPE_IFT
extern const FontOps FontOps_ift;
#endif
#if FONT_TYPE_XFT
extern const FontOps FontOps_xft;
#endif
#if FONT_TYPE_PANGO_XFT
extern const FontOps FontOps_pango;
#endif

#endif                          /* USE_MODULES */

static FontHandler fhs[] = {
#if FONT_TYPE_XFS
    FONT("xfs", &FontOps_xfs, &FontOps_xfs),    /* XFontSet - XCreateFontSet */
#endif
#if FONT_TYPE_IFT
    FONT("ift", &FontOps_ift, NULL),    /* Imlib2/FreeType */
#endif
#if FONT_TYPE_XFT
    FONT("xft", &FontOps_xft, NULL),    /* Xft */
#endif
#if FONT_TYPE_PANGO_XFT
    FONT("pango", &FontOps_pango, NULL),        /* Pango-Xft */
#endif
    FONT(NULL, NULL, NULL)
};

static void
TextStateLoadFont(TextState *ts)
{
    const char     *s, *type, *name;
    char            buf[1024];
    FontHandler    *fhp = fhs;

    if (!ts->fontname)
        return;

    /* Quit if already done */
    if (ts->type)
        return;

    ts->need_utf8 = Mode.locale.utf8_int;

    type = NULL;
    name = ts->fontname;
#if FONT_TYPE_IFT
    if (strchr(ts->fontname, '/'))
    {
        type = "ift";
        goto check;
    }
#endif
#if FONT_TYPE_XFS
    if (ts->fontname[0] == '-')
    {
        type = "xfs";
        goto check;
    }
#endif
    s = strchr(ts->fontname, ':');
    if (!s || s == ts->fontname)
        goto fallback;
    if (s - ts->fontname > 16)
        goto fallback;
    memcpy(buf, ts->fontname, s - ts->fontname);
    buf[s - ts->fontname] = '\0';
    type = buf;
    name = s + 1;

  check:
    for (fhp = fhs; fhp->type; fhp++)
    {
        if (strcmp(fhp->type, type))
            continue;
#if USE_MODULES
        if (!fhp->ops)
        {
            if (fhp->checked)
                goto fallback;
            fhp->checked = 1;
            fhp->ops = ModLoadSym("font", "FontOps", type);
            if (!fhp->ops)
                goto fallback;
        }
#endif
        if (fhp->ops->Load(ts, name) == 0)
            goto done;
    }
  fallback:
#if FONT_TYPE_XFS
    if (!FontOps_xfs.Load(ts, "fixed")) /* XFontSet - XCreateFontSet */
        goto done;
#endif

  done:
    if (!ts->ops)
        Eprintf("*** Unable to load font \"%s\"\n", ts->fontname);
    else if (EDebug(EDBUG_TYPE_FONTS))
        Eprintf("%s: %s: type=%d\n", __func__, ts->fontname, ts->type);
    return;
}

void
TextSize(TextClass *tclass, int active, int sticky, int state,
         const char *text, int *width, int *height)
{
    const char     *str;
    char          **lines;
    int             i, num_lines, ww, hh, asc;
    TextState      *ts;

    *width = 0;
    *height = 0;

    if (!text)
        return;

    ts = TextclassGetTextState(tclass, state, active, sticky);
    if (!ts)
        return;

    TextStateLoadFont(ts);
    if (!ts->ops)
        return;

    /* Do encoding conversion, if necessary */
    str = EstrInt2Enc(text, ts->need_utf8);
    lines = StrlistFromString(str, '\n', &num_lines);
    EstrInt2EncFree(str, ts->need_utf8);
    if (!lines)
        return;

    for (i = 0; i < num_lines; i++)
    {
        ts->ops->TextSize(ts, lines[i], strlen(lines[i]), &ww, &hh, &asc);
        *height += hh;
        if (ww > *width)
            *width = ww;
    }

    StrlistFree(lines, num_lines);
}

static void
TextstateTextFit(TextState *ts, char **ptext, int *pw, int textwidth_limit)
{
    if (ts->need_utf8 || MB_CUR_MAX > 1)
        TextstateTextFitMB(ts, ptext, pw, textwidth_limit);
    else
        TextstateTextFit1(ts, ptext, pw, textwidth_limit);
}

void
TextstateTextDraw(TextState *ts, Win win, EX_Drawable draw,
                  const char *text, int x, int y, int w, int h,
                  const EImageBorder *pad, int justh, int justv)
{
    const char     *str;
    char          **lines;
    int             i, num_lines;
    int             textwidth_limit, textheight_limit, offset_x, offset_y;
    int             xx, yy, ww, hh, ascent;
    EX_Pixmap       drawable;

    if (w <= 0 || h <= 0)
        return;

    TextStateLoadFont(ts);
    if (!ts->ops)
        return;

    /* Do encoding conversion, if necessary */
    str = EstrInt2Enc(text, ts->need_utf8);
    lines = StrlistFromString(str, '\n', &num_lines);
    EstrInt2EncFree(str, ts->need_utf8);
    if (!lines)
        return;

    if (draw == NoXID)
        draw = WinGetXwin(win);

    if (ts->style.orientation == FONT_TO_RIGHT ||
        ts->style.orientation == FONT_TO_LEFT)
    {
        if (pad)
        {
            x += pad->left;
            w -= pad->left + pad->right;
            y += pad->top;
            h -= pad->top + pad->bottom;
        }
        textwidth_limit = w;
        textheight_limit = h;
    }
    else
    {
        if (pad)
        {
            x += pad->left;
            h -= pad->left + pad->right;
            y += pad->top;
            w -= pad->top + pad->bottom;
        }
        textwidth_limit = h;
        textheight_limit = w;
    }

#if 0
    Eprintf("%s: %d,%d %dx%d(%dx%d): %s\n", __func__, x, y, w, h,
            textwidth_limit, textheight_limit, text);
#endif

    yy = y;

    if (ts->ops->FdcInit(ts, win, draw))
        return;

#if FONT_TYPE_IFT
    if (ts->type == FONT_TYPE_IFT)
    {
        for (i = 0; i < num_lines; i++)
        {
            EImage         *im;

            ts->ops->TextSize(ts, lines[i], 0, &ww, &hh, &ascent);
            if (ww > textwidth_limit)
                TextstateTextFit(ts, &lines[i], &ww, textwidth_limit);

            if (justv && num_lines == 1 && textheight_limit > 0)
                yy += (textheight_limit - hh) / 2;
            if (i == 0)
                yy += ascent;
            xx = x + (((textwidth_limit - ww) * justh) >> 10);

            im = TextImageGet(win, draw, xx - 1, yy - 1 - ascent,
                              ww + 2, hh + 2, ts);
            if (!im)
                break;

            offset_x = 1;
            offset_y = ascent + 1;

            ts->ops->FdcSetDrawable(ts, (unsigned long)im);

            TsTextDraw(ts, offset_x, offset_y, lines[i], strlen(lines[i]));

            TextImagePut(im, win, draw, xx - 1, yy - 1 - ascent,
                         ww + 2, hh + 2, ts);

            yy += hh;
        }
    }
    else
#endif                          /* FONT_TYPE_IFT */
    {
        for (i = 0; i < num_lines; i++)
        {
            ts->ops->TextSize(ts, lines[i], 0, &ww, &hh, &ascent);
            if (ww > textwidth_limit)
                TextstateTextFit(ts, &lines[i], &ww, textwidth_limit);

            if (justv && num_lines == 1 && textheight_limit > 0)
                yy += (textheight_limit - hh) / 2;
            if (i == 0)
                yy += ascent;
            xx = x + (((textwidth_limit - ww) * justh) >> 10);

            if (ts->style.orientation != FONT_TO_RIGHT)
                drawable = ECreatePixmap(win, ww + 2, hh + 2, 0);
            else
                drawable = draw;
            TextDrawRotTo(win, draw, drawable, xx - 1, yy - 1 - ascent,
                          ww + 2, hh + 2, ts);

            if (ts->style.orientation == FONT_TO_RIGHT)
            {
                offset_x = xx;
                offset_y = yy;
            }
            else
            {
                offset_x = 1;
                offset_y = ascent + 1;
            }

            if (drawable != draw)
                ts->ops->FdcSetDrawable(ts, drawable);

            TsTextDraw(ts, offset_x, offset_y, lines[i], strlen(lines[i]));

            TextDrawRotBack(win, draw, drawable, xx - 1, yy - 1 - ascent,
                            ww + 2, hh + 2, ts);
            if (drawable != draw)
                EFreePixmap(drawable);

            yy += hh;
        }
    }

    if (ts->ops->FdcFini)
        ts->ops->FdcFini(ts);

    StrlistFree(lines, num_lines);
}

void
TextDraw(TextClass *tclass, Win win, EX_Drawable draw, int active,
         int sticky, int state, const char *text, int x, int y, int w, int h,
         int justh)
{
    TextState      *ts;

    if (!tclass || !text)
        return;

    ts = TextclassGetTextState(tclass, state, active, sticky);
    if (!ts)
        return;

    TextstateTextDraw(ts, win, draw, text, x, y, w, h, NULL, justh, 0);
}
