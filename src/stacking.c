/*
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
#include "E.h"
#include "desktops.h"
#include "eobj.h"
#include "ewins.h"

#define ENABLE_DEBUG_STACKING 1

#define EobjGetCwin(p) \
    ((p->type == EOBJ_TYPE_EWIN) ? EwinGetClientXwin(((EWin*)(p))) : NoXID)

typedef struct {
   const char         *name;
   struct {
      int                 nalloc;
      int                 nobjs;
      EObj              **list;
   } o;
   struct {
      int                 nalloc;
      int                 nwins;
      EWin              **list;
   } e;
   char                layered;
   char                changed;	/* Used by _EwinListGet() */
} EobjList;

static int          EobjListRaise(EobjList * eol, EObj * eo, int test);
static int          EobjListLower(EobjList * eol, EObj * eo, int test);

#if ENABLE_DEBUG_STACKING
static void
EobjListShow(const char *txt, EobjList * eol)
{
   int                 i;
   EObj               *eo;

   if (EDebug(EDBUG_TYPE_STACKING) < 2)
      return;

   Eprintf("%s-%s:\n", eol->name, txt);
   for (i = 0; i < eol->o.nobjs; i++)
     {
	eo = eol->o.list[i];
	Eprintf(" %2d: %#10x %#10x %d %d %s\n", i, EobjGetXwin(eo),
		EobjGetCwin(eo), eo->desk->num, eo->ilayer, EobjGetName(eo));
     }
}
#else
#define EobjListShow(txt, eol)
#endif

static int
EobjListGetIndex(EobjList * eol, EObj * eo)
{
   int                 i;

   for (i = 0; i < eol->o.nobjs; i++)
      if (eol->o.list[i] == eo)
	 return i;

   return -1;
}

static void
EobjListAdd(EobjList * eol, EObj * eo, int ontop)
{
   int                 i;

   /* Quit if already in list */
   i = EobjListGetIndex(eol, eo);
   if (i >= 0)
      return;

   if (eol->o.nobjs >= eol->o.nalloc)
     {
	eol->o.nalloc += 16;
	eol->o.list = EREALLOC(EObj *, eol->o.list, eol->o.nalloc);
     }

   if (eol->layered)
     {
	/* The simple way for now (add, raise/lower) */
	if (ontop)
	  {
	     eol->o.list[eol->o.nobjs] = eo;
	     eol->o.nobjs++;
	     EobjListRaise(eol, eo, 0);
	  }
	else
	  {
	     memmove(eol->o.list + 1, eol->o.list,
		     eol->o.nobjs * sizeof(EObj *));
	     eol->o.list[0] = eo;
	     eol->o.nobjs++;
	     EobjListLower(eol, eo, 0);
	  }
	if (eo->stacked == 0)
	   DeskSetDirtyStack(eo->desk, eo);
     }
   else
     {
	if (ontop)
	  {
	     memmove(eol->o.list + 1, eol->o.list,
		     eol->o.nobjs * sizeof(EObj *));
	     eol->o.list[0] = eo;
	  }
	else
	  {
	     eol->o.list[eol->o.nobjs] = eo;
	  }
	eol->o.nobjs++;
     }

   eol->changed = 1;

   EobjListShow("EobjListAdd", eol);
}

static void
EobjListDel(EobjList * eol, EObj * eo)
{
   int                 i, n;

   /* Quit if not in list */
   i = EobjListGetIndex(eol, eo);
   if (i < 0)
      return;

   eol->o.nobjs--;
   n = eol->o.nobjs - i;
   if (n > 0)
     {
	memmove(eol->o.list + i, eol->o.list + i + 1, n * sizeof(EObj *));
     }
   else if (eol->o.nobjs <= 0)
     {
	/* Enables autocleanup at shutdown, if ever implemented */
	EFREE_NULL(eol->o.list);
	eol->o.nalloc = 0;
     }

   eol->changed = 1;

   EobjListShow("EobjListDel", eol);
}

static int
EobjListLower(EobjList * eol, EObj * eo, int test)
{
   int                 i, j, n;

   /* Quit if not in list */
   i = EobjListGetIndex(eol, eo);
   if (i < 0)
      return 0;

   j = eol->o.nobjs - 1;
   if (eol->layered)
     {
	/* Take the layer into account */
	for (; j >= 0; j--)
	   if (i != j && eo->ilayer <= eol->o.list[j]->ilayer)
	      break;
	if (j < i)
	   j++;
     }

   n = j - i;
   if (test)
      return n;

   if (n > 0)
     {
	memmove(eol->o.list + i, eol->o.list + i + 1, n * sizeof(EObj *));
	eol->o.list[j] = eo;
	if (eol->layered && eo->stacked > 0)
	   DeskSetDirtyStack(eo->desk, eo);
	eol->changed = 1;
     }
   else if (n < 0)
     {
	memmove(eol->o.list + j + 1, eol->o.list + j, -n * sizeof(EObj *));
	eol->o.list[j] = eo;
	if (eol->layered && eo->stacked > 0)
	   DeskSetDirtyStack(eo->desk, eo);
	eol->changed = 1;
     }

   EobjListShow("EobjListLower", eol);
   return n;
}

static int
EobjListRaise(EobjList * eol, EObj * eo, int test)
{
   int                 i, j, n;

   /* Quit if not in list */
   i = EobjListGetIndex(eol, eo);
   if (i < 0)
      return 0;

   j = 0;
   if (eol->layered)
     {
	/* Take the layer into account */
	for (; j < eol->o.nobjs; j++)
	   if (j != i && eo->ilayer >= eol->o.list[j]->ilayer)
	      break;
	if (j > i)
	   j--;
     }

   n = j - i;
   if (test)
      return n;

   if (n > 0)
     {
	memmove(eol->o.list + i, eol->o.list + i + 1, n * sizeof(EObj *));
	eol->o.list[j] = eo;
	if (eol->layered && eo->stacked > 0)
	   DeskSetDirtyStack(eo->desk, eo);
	eol->changed = 1;
     }
   else if (n < 0)
     {
	memmove(eol->o.list + j + 1, eol->o.list + j, -n * sizeof(EObj *));
	eol->o.list[j] = eo;
	if (eol->layered && eo->stacked > 0)
	   DeskSetDirtyStack(eo->desk, eo);
	eol->changed = 1;
     }

   EobjListShow("EobjListRaise", eol);
   return n;
}

static EObj        *
EobjListFind(const EobjList * eol, EX_Window win)
{
   int                 i;

   for (i = 0; i < eol->o.nobjs; i++)
      if (EobjGetXwin(eol->o.list[i]) == win)
	 return eol->o.list[i];

   return NULL;
}

#if 0
static int
EobjListTypeCount(const EobjList * eol, int type)
{
   int                 i, n;

   for (i = n = 0; i < eol->o.nobjs; i++)
      if (eol->o.list[i]->type == type)
	 n++;

   return n;
}
#endif

/*
 * The global object/client lists
 */
#define EOBJ_LIST(_name, _layered) \
    { .name = _name, .layered = _layered }

static EobjList     EobjListStack = EOBJ_LIST("Stack", 1);
static EobjList     EwinListFocus = EOBJ_LIST("Focus", 0);
static EobjList     EwinListOrder = EOBJ_LIST("Order", 0);

static EObj        *const *
EobjListGet(EobjList * eol, int *num)
{
   *num = eol->o.nobjs;
   return eol->o.list;
}

int
EobjListStackCheck(EObj * eo)
{
   return EobjListGetIndex(&EobjListStack, eo);
}

EObj               *
EobjListStackFind(EX_Window win)
{
   return EobjListFind(&EobjListStack, win);
}

EObj               *const *
EobjListStackGet(int *num)
{
   return EobjListGet(&EobjListStack, num);
}

void
EobjListStackAdd(EObj * eo, int ontop)
{
   EobjListAdd(&EobjListStack, eo, ontop);
}

void
EobjListStackDel(EObj * eo)
{
   EobjListDel(&EobjListStack, eo);
}

int
EobjListStackRaise(EObj * eo, int test)
{
   return EobjListRaise(&EobjListStack, eo, test);
}

int
EobjListStackLower(EObj * eo, int test)
{
   return EobjListLower(&EobjListStack, eo, test);
}

void
EobjListFocusAdd(EObj * eo, int ontop)
{
   EobjListAdd(&EwinListFocus, eo, ontop);
}

void
EobjListFocusDel(EObj * eo)
{
   EobjListDel(&EwinListFocus, eo);
}

int
EobjListFocusRaise(EObj * eo)
{
   return EobjListRaise(&EwinListFocus, eo, 0);
}

static EWin        *const *
_EwinListGet(EobjList * eol, int *pnum)
{
   int                 i, j;
   EObj               *eo;

   if (EDebug(EDBUG_TYPE_STACKING))
      Eprintf("%s: %s changed=%d\n", __func__, eol->name, eol->changed);

   if (!eol->changed)
      goto done;

   for (i = j = 0; i < eol->o.nobjs; i++)
     {
	eo = eol->o.list[i];
	if (eo->type != EOBJ_TYPE_EWIN)
	   continue;

	if (eol->e.nalloc <= j)
	  {
	     eol->e.nalloc += 16;	/* 16 at the time */
	     eol->e.list = EREALLOC(EWin *, eol->e.list, eol->e.nalloc);
	  }

	eol->e.list[j++] = (EWin *) eo;
     }

   eol->e.nwins = j;
   eol->changed = 0;

 done:
   *pnum = eol->e.nwins;
   return eol->e.list;
}

EWin               *const *
EwinListStackGet(int *num)
{
   return _EwinListGet(&EobjListStack, num);
}

EWin               *const *
EwinListFocusGet(int *num)
{
   return (EWin * const *)EobjListGet(&EwinListFocus, num);
}

EWin               *const *
EwinListGetForDesk(int *num, Desk * dsk)
{
   static EWin       **lst = NULL;
   static int          nalloc = 0;
   const EobjList     *eol;
   int                 i, j;
   EObj               *eo;

   eol = &EobjListStack;

   for (i = j = 0; i < eol->o.nobjs; i++)
     {
	eo = eol->o.list[i];
	if (eo->type != EOBJ_TYPE_EWIN || eo->desk != dsk)
	   continue;

	if (nalloc <= j)
	  {
	     nalloc += 16;	/* 16 at the time */
	     lst = EREALLOC(EWin *, lst, nalloc);
	  }

	lst[j++] = (EWin *) eo;
     }

   *num = j;
   return lst;
}

EObj               *const *
EobjListStackGetForDesk(int *num, Desk * dsk)
{
   static EObj       **lst = NULL;
   static int          nalloc = 0;
   const EobjList     *eol;
   int                 i, j;
   EObj               *eo;

   eol = &EobjListStack;

   /* Too many - who cares. */
   if (nalloc < eol->o.nobjs)
     {
	nalloc = (eol->o.nobjs + 16) & ~0xf;	/* 16 at the time */
	lst = EREALLOC(EObj *, lst, nalloc);
     }

   for (i = j = 0; i < eol->o.nobjs; i++)
     {
	eo = eol->o.list[i];
	if (eo->desk != dsk)
	   continue;

	lst[j++] = eo;
     }

   *num = j;
   return lst;
}

int
EwinListStackIsRaised(const EWin * ewin)
{
   const EobjList     *eol;
   int                 i;
   const EObj         *eo, *eox;

   eol = &EobjListStack;
   eox = EoObj(ewin);

   for (i = 0; i < eol->o.nobjs; i++)
     {
	eo = eol->o.list[i];
	if (eo->type != EOBJ_TYPE_EWIN)
	   continue;
	if (eo->desk != eox->desk)
	   continue;
	if (eo->ilayer > eox->ilayer)
	   continue;
	if (EwinGetTransientFor((EWin *) eo) == EwinGetClientXwin(ewin))
	   continue;
	return eo == eox;
     }

   return 1;			/* It should be impossible to get here */
}

void
EobjListOrderAdd(EObj * eo)
{
   EobjListAdd(&EwinListOrder, eo, 0);
}

void
EobjListOrderDel(EObj * eo)
{
   EobjListDel(&EwinListOrder, eo);
}

EWin               *const *
EwinListOrderGet(int *num)
{
   return (EWin * const *)EobjListGet(&EwinListOrder, num);
}
