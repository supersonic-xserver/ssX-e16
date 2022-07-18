/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2021 Kim Woelders
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

#include "E.h"
#include "desktops.h"
#include "eobj.h"
#include "ewins.h"
#include "menus.h"		/* FIXME - Should not be here */
#include "timers.h"
#include "xwin.h"

#define EW_L	0		/* Left   */
#define EW_R	1		/* Right  */
#define EW_T	2		/* Top    */
#define EW_B	3		/* Bottom */

static EObj        *w1 = NULL, *w2 = NULL, *w3 = NULL, *w4 = NULL;
static Timer       *edge_timer = NULL;

static int
EdgeTimeout(void *data)
{
   int                 val;
   int                 ax, ay, aw, ah, dx, dy, dax, day;
   EWin               *ewin;

   if (MenusActive())
      goto done;
   if (Conf.desks.edge_flip_mode == EDGE_FLIP_OFF)
      goto done;

   /* Quit if pointer has left screen */
   if (!EQueryPointer(NULL, NULL, NULL, NULL, NULL))
      goto done;

   /* Quit if in fullscreen window */
   ewin = GetEwinPointerInClient();
   if (ewin && ewin->state.fullscreen)
      goto done;

   DeskCurrentGetArea(&ax, &ay);
   DesksGetAreaSize(&aw, &ah);
   val = PTR2INT(data);
   dx = 0;
   dy = 0;
   dax = 0;
   day = 0;
   switch (val)
     {
     case EW_L:
	if (ax == 0 && !Conf.desks.areas_wraparound)
	   goto done;
	dx = WinGetW(VROOT) - 2;
	dax = -1;
	break;
     case EW_R:
	if (ax == (aw - 1) && !Conf.desks.areas_wraparound)
	   goto done;
	dx = -(WinGetW(VROOT) - 2);
	dax = 1;
	break;
     case EW_T:
	if (ay == 0 && !Conf.desks.areas_wraparound)
	   goto done;
	dy = WinGetH(VROOT) - 2;
	day = -1;
	break;
     case EW_B:
	if (ay == (ah - 1) && !Conf.desks.areas_wraparound)
	   goto done;
	dy = -(WinGetH(VROOT) - 2);
	day = 1;
	break;
     default:
	break;
     }
   if (aw == 1)
      dx = 0;
   if (ah == 1)
      dy = 0;
   Mode.events.px = Mode.events.mx;
   Mode.events.py = Mode.events.my;
   Mode.events.mx = Mode.events.cx += dx;
   Mode.events.my = Mode.events.cy += dy;
   EWarpPointer(VROOT, Mode.events.mx, Mode.events.my);
   DeskCurrentMoveAreaBy(dax, day);
   Mode.events.px = Mode.events.mx;
   Mode.events.py = Mode.events.my;

 done:
   edge_timer = NULL;
   return 0;
}

static void
EdgeEvent(int dir)
{
   static int          lastdir = -1;

#if 0
   Eprintf("%s: %d -> %d\n", __func__, lastdir, dir);
#endif
   if (lastdir == dir || Conf.desks.edge_flip_mode == EDGE_FLIP_OFF)
      return;

   if (Conf.desks.edge_flip_mode == EDGE_FLIP_MOVE && Mode.mode != MODE_MOVE)
      return;

   TIMER_DEL(edge_timer);
   if (dir >= 0)
     {
	if (Conf.desks.edge_flip_resistance <= 0)
	   Conf.desks.edge_flip_resistance = 1;
	TIMER_ADD(edge_timer, 10 * Conf.desks.edge_flip_resistance,
		  EdgeTimeout, INT2PTR(dir));
     }
   lastdir = dir;
}

static void
EdgeHandleEvents(Win win __UNUSED__, XEvent * ev, void *prm)
{
   int                 dir;

   dir = PTR2INT(prm);

   switch (ev->type)
     {
     default:
	break;

     case EnterNotify:
	EdgeEvent(dir);
	break;

     case LeaveNotify:
	EdgeEvent(-1);
	break;
     }
}

void
EdgeCheckMotion(int x, int y)
{
   int                 dir;

   if (x == 0)
      dir = 0;
   else if (x == WinGetW(VROOT) - 1)
      dir = 1;
   else if (y == 0)
      dir = 2;
   else if (y == WinGetH(VROOT) - 1)
      dir = 3;
   else
      dir = -1;

   EdgeEvent(dir);
}

static EObj        *
EdgeWindowCreate(int which, int x, int y, int w, int h)
{
   static const char  *const names[] =
      { "Edge-L", "Edge-R", "Edge-T", "Edge-B" };
   EObj               *eo;

   eo = EobjWindowCreate(EOBJ_TYPE_EVENT, x, y, w, h, 0, names[which & 3]);
   ESelectInput(EobjGetWin(eo), EnterWindowMask | LeaveWindowMask);
   EventCallbackRegister(EobjGetWin(eo), EdgeHandleEvents, INT2PTR(which));

   return eo;
}

static void
EdgeWindowShow(int which, int on)
{
   EObj               *eo, **peo;
   int                 x, y, w, h;

   x = y = 0;
   w = h = 1;

   switch (which)
     {
     default:
     case EW_L:		/* Left */
	peo = &w1;
	h = WinGetH(VROOT);
	break;
     case EW_R:		/* Right */
	peo = &w2;
	x = WinGetW(VROOT) - 1;
	h = WinGetH(VROOT);
	break;
     case EW_T:		/* Top */
	peo = &w3;
	w = WinGetW(VROOT);
	break;
     case EW_B:		/* Bottom */
	peo = &w4;
	y = WinGetH(VROOT) - 1;
	w = WinGetW(VROOT);
	break;
     }

   eo = *peo;
   if (!eo)
     {
	eo = *peo = EdgeWindowCreate(which, x, y, w, h);
	if (!eo)
	   return;
     }

   if (on)
     {
	EobjMoveResize(eo, x, y, w, h);
	EobjMap(eo, 0);
     }
   else
     {
	EobjUnmap(eo);
     }
}

void
EdgeWindowsShow(void)
{
   int                 ax, ay, cx, cy;

   if (Conf.desks.edge_flip_mode == EDGE_FLIP_OFF)
     {
	EdgeWindowsHide();
	return;
     }

   DeskCurrentGetArea(&cx, &cy);
   DesksGetAreaSize(&ax, &ay);

   EdgeWindowShow(EW_L, cx != 0 || Conf.desks.areas_wraparound);
   EdgeWindowShow(EW_R, cx != (ax - 1) || Conf.desks.areas_wraparound);
   EdgeWindowShow(EW_T, cy != 0 || Conf.desks.areas_wraparound);
   EdgeWindowShow(EW_B, cy != (ay - 1) || Conf.desks.areas_wraparound);
}

void
EdgeWindowsHide(void)
{
   if (!w1)
      return;

   EobjUnmap(w1);
   EobjUnmap(w2);
   EobjUnmap(w3);
   EobjUnmap(w4);
}
