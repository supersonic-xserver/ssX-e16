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
#include "E.h"
#include "borders.h"
#include "dialog.h"
#include "emodule.h"
#include "ewins.h"
#include "groups.h"
#include "list.h"
#include "settings.h"
#include "snaps.h"

#define DEBUG_GROUPS 0
#if DEBUG_GROUPS
#define Dprintf(fmt, ...) Eprintf("%s: " fmt, __func__, __VA_ARGS__)
#else
#define Dprintf(fmt...)
#endif

#define USE_GROUP_SHOWHIDE 1	/* Enable group borders */

#define GROUP_SELECT_ALL             0
#define GROUP_SELECT_EWIN_ONLY       1
#define GROUP_SELECT_ALL_EXCEPT_EWIN 2

#define SET_OFF    0
#define SET_ON     1
#define SET_TOGGLE 2

typedef struct _groupconfig {
   char                iconify;
   char                kill;
   char                move;
   char                raise;
   char                set_border;
   char                shade;
   char                stick;
} GroupConfig;

struct _group {
   dlist_t             list;
   int                 index;
   EWin              **members;
   int                 num_members;
   GroupConfig         cfg;
   char                keep;	/* Don't destroy when empty */
   char                used;	/* Don't discard when saving */
};

static              LIST_HEAD(group_list);

static struct {
   GroupConfig         dflt;
   char                swapmove;
} Conf_groups;

static struct {
   Group              *current;
} Mode_groups;

static void         _GroupsSave(void);

int
GroupsGetSwapmove(void)
{
   return Conf_groups.swapmove;
}

static Group       *
_GroupCreate(int gid, int cfg_update)
{
   Group              *g;

   g = ECALLOC(Group, 1);
   if (!g)
      return NULL;

   LIST_APPEND(Group, &group_list, g);

   if (gid == -1)
     {
	/* Create new group id */
	/* ... using us time. Should really be checked for uniqueness. */
	g->index = (int)GetTimeUs();
     }
   else
     {
	/* Use given group id */
	g->index = gid;
     }
   g->cfg.iconify = Conf_groups.dflt.iconify;
   g->cfg.kill = Conf_groups.dflt.kill;
   g->cfg.move = Conf_groups.dflt.move;
   g->cfg.raise = Conf_groups.dflt.raise;
   g->cfg.set_border = Conf_groups.dflt.set_border;
   g->cfg.stick = Conf_groups.dflt.stick;
   g->cfg.shade = Conf_groups.dflt.shade;

   Dprintf("%s: grp=%p gid=%d\n", __func__, g, g->index);

   if (cfg_update)
      _GroupsSave();

   return g;
}

static void
_GroupDestroy(Group * g, int cfg_update)
{
   if (!g)
      return;

   Dprintf("%s: grp=%p gid=%d\n", __func__, g, g->index);

   LIST_REMOVE(Group, &group_list, g);

   if (g == Mode_groups.current)
      Mode_groups.current = NULL;

   Efree(g->members);
   Efree(g);

   if (cfg_update)
      _GroupsSave();
}

static int
_GroupMatchId(const void *data, const void *match)
{
   return ((const Group *)data)->index != PTR2INT(match);
}

static Group       *
_GroupFind(int gid)
{
   return LIST_FIND(Group, &group_list, _GroupMatchId, INT2PTR(gid));
}

EWin               *const *
GroupGetMembers(const Group * g, int *num)
{
   *num = g->num_members;
   return g->members;
}

int
GroupGetIndex(const Group * g)
{
   return g->index;
}

void
GroupSetUsed(int gid)
{
   Group              *g;

   g = _GroupFind(gid);
   if (!g)
      return;

   g->used = 1;
}

int
GroupMatchAction(const Group * g, int action)
{
   int                 match;

   switch (action)
     {
     default:
	match = 0;
	break;
     case GROUP_ACTION_SET_WINDOW_BORDER:
	match = g->cfg.set_border;
	break;
     case GROUP_ACTION_ICONIFY:
	match = g->cfg.iconify;
	break;
     case GROUP_ACTION_MOVE:
	match = g->cfg.move;
	break;
     case GROUP_ACTION_STACKING:
	match = g->cfg.raise;
	break;
     case GROUP_ACTION_STICK:
	match = g->cfg.stick;
	break;
     case GROUP_ACTION_SHADE:
	match = g->cfg.shade;
	break;
     case GROUP_ACTION_KILL:
	match = g->cfg.kill;
	break;
     }

   return match;
}

static Group       *
_GroupFind2(const char *groupid)
{
   int                 gid;

   if (groupid[0] == '*' || groupid[0] == '\0')
      return Mode_groups.current;

   gid = 0;
   sscanf(groupid, "%d", &gid);
   if (gid == 0)
      return NULL;

   return _GroupFind(gid);
}

static int
_EwinGroupIndex(const EWin * ewin, const Group * g)
{
   int                 i;

   for (i = 0; i < ewin->num_groups; i++)
      if (ewin->groups[i] == g)
	 return i;

   return -1;
}

static int
_GroupEwinIndex(const Group * g, const EWin * ewin)
{
   int                 i;

   for (i = 0; i < g->num_members; i++)
      if (g->members[i] == ewin)
	 return i;

   return -1;
}

static void
_GroupEwinAdd(Group * g, EWin * ewin, int snap_update)
{
   if (!ewin || !g)
      return;

   if (_EwinGroupIndex(ewin, g) >= 0)
      return;			/* Already there */

   Dprintf("%s: gid=%8d: %s\n", __func__, g->index, EoGetName(ewin));

   ewin->num_groups++;
   ewin->groups = EREALLOC(Group *, ewin->groups, ewin->num_groups);
   ewin->groups[ewin->num_groups - 1] = g;
   g->num_members++;
   g->members = EREALLOC(EWin *, g->members, g->num_members);
   g->members[g->num_members - 1] = ewin;

   if (snap_update)
      SnapshotEwinUpdate(ewin, SNAP_USE_GROUPS);
}

static void
_GroupEwinRemove(Group * g, EWin * ewin, int snap_update)
{
   int                 i, ie, ig;

   if (!ewin || !g)
      return;

   Dprintf("%s: gid=%8d: %s\n", __func__, g->index, EoGetName(ewin));

   ie = _EwinGroupIndex(ewin, g);
   if (ie < 0)
     {
	/* Should not happen */
	Dprintf("%s: g=%p gid=%8d: %s: Group not found?!?\n", __func__,
		g, g->index, EoGetName(ewin));
	return;
     }

   ig = _GroupEwinIndex(g, ewin);
   if (ig < 0)
     {
	/* Should not happen */
	Dprintf("%s: g=%p gid=%8d: %s: Ewin not found?!?\n", __func__,
		g, g->index, EoGetName(ewin));
	return;
     }

   Dprintf("%s: gid=%8d index=%d/%d: %s\n", __func__,
	   g->index, ie, ig, EoGetName(ewin));

   /* remove it from the group */
   g->num_members--;
   for (i = ig; i < g->num_members; i++)
      g->members[i] = g->members[i + 1];

   if (g->num_members > 0)
      g->members = EREALLOC(EWin *, g->members, g->num_members);
   else if (g->keep)
      EFREE_NULL(g->members);
   else
      _GroupDestroy(g, snap_update);

   /* and remove the group from the groups that the window is in */
   ewin->num_groups--;
   for (i = ie; i < ewin->num_groups; i++)
      ewin->groups[i] = ewin->groups[i + 1];

   if (ewin->num_groups > 0)
      ewin->groups = EREALLOC(Group *, ewin->groups, ewin->num_groups);
   else
      EFREE_NULL(ewin->groups);

   if (snap_update)
      SnapshotEwinUpdate(ewin, SNAP_USE_GROUPS);
}

static void
_GroupDelete(Group * g)
{
   if (!g)
      return;

   Dprintf("%s: gid=%d\n", __func__, g->index);

   g->keep = 1;
   while (g->num_members > 0)
      _GroupEwinRemove(g, g->members[0], 1);
   _GroupDestroy(g, 1);
}

Group             **
GroupsGetList(int *pnum)
{
   return LIST_GET_ITEMS(Group, &group_list, pnum);
}

#if ENABLE_DIALOGS
static Group      **
_EwinListGroups(const EWin * ewin, char group_select, int *num)
{
   Group             **groups;
   Group             **groups2;
   int                 i, j, n, killed;

   groups = NULL;
   *num = 0;

   switch (group_select)
     {
     case GROUP_SELECT_EWIN_ONLY:
	*num = n = ewin->num_groups;
	if (n <= 0)
	   break;
	groups = EMALLOC(Group *, n);
	if (!groups)
	   break;
	memcpy(groups, ewin->groups, n * sizeof(Group *));
	break;
     case GROUP_SELECT_ALL_EXCEPT_EWIN:
	groups2 = GroupsGetList(num);
	if (!groups2)
	   break;
	n = *num;
	for (i = killed = 0; i < n; i++)
	  {
	     for (j = 0; j < ewin->num_groups; j++)
	       {
		  if (ewin->groups[j] == groups2[i])
		    {
		       groups2[i] = NULL;
		       killed++;
		    }
	       }
	  }
	if (n - killed > 0)
	  {
	     groups = EMALLOC(Group *, n - killed);
	     if (groups)
	       {
		  for (i = j = 0; i < n; i++)
		     if (groups2[i])
			groups[j++] = groups2[i];
		  *num = n - killed;
	       }
	  }
	Efree(groups2);
	break;
     case GROUP_SELECT_ALL:
     default:
	groups = GroupsGetList(num);
	break;
     }

   return groups;
}
#endif /* ENABLE_DIALOGS */

/* Update groups on snapped window appearance */
void
GroupsEwinAdd(EWin * ewin, const int *pgid, int ngid)
{
   Group              *g;
   int                 i, gid;

   for (i = 0; i < ngid; i++)
     {
	gid = pgid[i];
	g = _GroupFind(gid);
	Dprintf("ewin=%p gid=%d grp=%p\n", ewin, gid, g);
	if (!g)
	  {
	     /* This should not happen, but may if group/snap configs are corrupted */
	     g = _GroupCreate(gid, 1);
	  }
	_GroupEwinAdd(g, ewin, 0);
     }
}

/* Update groups on snapped window disappearance */
void
GroupsEwinRemove(EWin * ewin)
{
   while (ewin->num_groups > 0)
      _GroupEwinRemove(ewin->groups[0], ewin, 0);
}

static int
_EwinInGroup(const EWin * ewin, const Group * g)
{
   int                 i;

   if (ewin && g)
     {
	for (i = 0; i < g->num_members; i++)
	  {
	     if (g->members[i] == ewin)
		return 1;
	  }
     }
   return 0;
}

Group              *
EwinsInGroup(const EWin * ewin1, const EWin * ewin2)
{
   int                 i;

   if (ewin1 && ewin2)
     {
	for (i = 0; i < ewin1->num_groups; i++)
	  {
	     if (_EwinInGroup(ewin2, ewin1->groups[i]))
		return ewin1->groups[i];
	  }
     }
   return NULL;
}

#if ENABLE_DIALOGS
static char       **
_GrouplistMemberNames(Group ** groups, int num)
{
   int                 i, j, len;
   char              **group_member_strings;
   const char         *name;

   group_member_strings = ECALLOC(char *, num);

   if (!group_member_strings)
      return NULL;

   for (i = 0; i < num; i++)
     {
	group_member_strings[i] = EMALLOC(char, 1024);

	if (!group_member_strings[i])
	   break;

	len = 0;
	for (j = 0; j < groups[i]->num_members; j++)
	  {
	     name = EwinGetTitle(groups[i]->members[j]);
	     if (!name)		/* Should never happen */
		continue;
	     len += Esnprintf(group_member_strings[i] + len, 1024 - len,
			      "%s\n", name);
	     if (len >= 1024)
		break;
	  }
	if (len == 0)
	   snprintf(group_member_strings[i], 1024, "(empty)");
     }

   return group_member_strings;
}
#endif /* ENABLE_DIALOGS */

#if USE_GROUP_SHOWHIDE
static void
_EwinGroupsShowHide(EWin * ewin, int group_index, char onoff)
{
   EWin              **gwins;
   int                 i, num;
   const Border       *b = NULL;

   if (!ewin || group_index >= ewin->num_groups)
      return;

   if (group_index < 0)
     {
	gwins = ListWinGroupMembersForEwin(ewin, GROUP_ACTION_ANY, 0, &num);
     }
   else
     {
	gwins = ewin->groups[group_index]->members;
	num = ewin->groups[group_index]->num_members;
     }

   if (onoff == SET_TOGGLE)
      onoff = (ewin->border == ewin->normal_border) ? SET_ON : SET_OFF;

   for (i = 0; i < num; i++)
     {
	if (onoff == SET_ON)
	   b = BorderFind(gwins[i]->border->group_border_name);
	else
	   b = gwins[i]->normal_border;

	EwinBorderChange(gwins[i], b, 0);
     }
   if (group_index < 0)
      Efree(gwins);
}
#else

#define _EwinGroupsShowHide(ewin, group_index, onoff)

#endif /* USE_GROUP_SHOWHIDE */

static void
_GroupsSave(void)
{
   Group              *g;
   FILE               *f;
   char                s[1024];

   Dprintf("%s\n", __func__);

   Esnprintf(s, sizeof(s), "%s.groups", EGetSavePrefix());
   f = fopen(s, "w");
   if (!f)
      return;

   LIST_FOR_EACH(Group, &group_list, g)
   {
      fprintf(f, "NEW: %i\n", g->index);
      fprintf(f, "ICONIFY: %i\n", g->cfg.iconify);
      fprintf(f, "KILL: %i\n", g->cfg.kill);
      fprintf(f, "MOVE: %i\n", g->cfg.move);
      fprintf(f, "RAISE: %i\n", g->cfg.raise);
      fprintf(f, "SET_BORDER: %i\n", g->cfg.set_border);
      fprintf(f, "STICK: %i\n", g->cfg.stick);
      fprintf(f, "SHADE: %i\n", g->cfg.shade);
   }

   fclose(f);
}

void
GroupsPrune(void)
{
   Group              *g, *tmp;
   int                 pruned;

   pruned = 0;

   LIST_FOR_EACH_SAFE(Group, &group_list, g, tmp)
   {
      if (g->used)
	 continue;
      if (g->members)
	 continue;
      _GroupDestroy(g, 0);
      pruned += 1;
   }

   Dprintf("%s: Pruned=%d\n", __func__, pruned);

   if (pruned)
      _GroupsSave();
}

static int
_GroupsLoad(FILE * fs)
{
   char                s[1024];
   Group              *g = NULL;

   while (fgets(s, sizeof(s), fs))
     {
	char                ss[128];
	int                 ii;

	if (strlen(s) > 0)
	   s[strlen(s) - 1] = 0;
	ii = 0;
	sscanf(s, "%100s %d", ss, &ii);

	if (!strcmp(ss, "NEW:"))
	  {
	     g = _GroupCreate(ii, 0);
	     continue;
	  }
	if (!g)
	   continue;

	if (!strcmp(ss, "ICONIFY:"))
	  {
	     g->cfg.iconify = ii;
	  }
	else if (!strcmp(ss, "KILL:"))
	  {
	     g->cfg.kill = ii;
	  }
	else if (!strcmp(ss, "MOVE:"))
	  {
	     g->cfg.move = ii;
	  }
	else if (!strcmp(ss, "RAISE:"))
	  {
	     g->cfg.raise = ii;
	  }
	else if (!strcmp(ss, "SET_BORDER:"))
	  {
	     g->cfg.set_border = ii;
	  }
	else if (!strcmp(ss, "STICK:"))
	  {
	     g->cfg.stick = ii;
	  }
	else if (!strcmp(ss, "SHADE:"))
	  {
	     g->cfg.shade = ii;
	  }
     }

   return 0;
}

void
GroupsLoad(void)
{
   char                s[4096];

   Dprintf("%s\n", __func__);

   Esnprintf(s, sizeof(s), "%s.groups", EGetSavePrefix());

   ConfigFileLoad(s, NULL, _GroupsLoad, 0);
}

#if ENABLE_DIALOGS

#define GROUP_OP_ADD	1
#define GROUP_OP_DEL	2
#define GROUP_OP_BREAK	3

typedef struct {
   EWin               *ewin;
   int                 action;
   const char         *message;
   Group             **groups;
   int                 group_num;
   int                 cur_grp;	/* Current  group */
   int                 prv_grp;	/* Previous group */
} GroupSelDlgData;

static void
_DlgApplyGroupChoose(Dialog * d, int val __UNUSED__, void *data __UNUSED__)
{
   GroupSelDlgData    *dd = DLG_DATA_GET(d, GroupSelDlgData);

   if (!dd->groups)
      return;

   switch (dd->action)
     {
     case GROUP_OP_ADD:
	_GroupEwinAdd(dd->groups[dd->cur_grp], dd->ewin, 1);
	break;
     case GROUP_OP_DEL:
	_GroupEwinRemove(dd->groups[dd->cur_grp], dd->ewin, 1);
	break;
     case GROUP_OP_BREAK:
	_GroupDelete(dd->groups[dd->cur_grp]);
	break;
     default:
	break;
     }
}

static void
_DlgExitGroupChoose(Dialog * d)
{
   GroupSelDlgData    *dd = DLG_DATA_GET(d, GroupSelDlgData);

   if (!dd->groups)
      return;
   _EwinGroupsShowHide(dd->ewin, dd->cur_grp, SET_OFF);
   Efree(dd->groups);
}

static void
_DlgSelectCbGroupChoose(Dialog * d, int val, void *data __UNUSED__)
{
   GroupSelDlgData    *dd = DLG_DATA_GET(d, GroupSelDlgData);

   /* val is equal to dd->cur_grp */
   _EwinGroupsShowHide(dd->ewin, dd->prv_grp, SET_OFF);
   _EwinGroupsShowHide(dd->ewin, val, SET_ON);
   dd->prv_grp = val;
}

static void
_DlgFillGroupChoose(Dialog * d, DItem * table, void *data)
{
   GroupSelDlgData    *dd = DLG_DATA_GET(d, GroupSelDlgData);
   DItem              *di, *radio;
   int                 i, num_groups;
   char              **group_member_strings;

   *dd = *(GroupSelDlgData *) data;

   DialogItemTableSetOptions(table, 2, 0, 0, 0);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 2);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetText(di, dd->message);

   num_groups = dd->group_num;
   group_member_strings = _GrouplistMemberNames(dd->groups, num_groups);
   if (!group_member_strings)
      return;			/* Silence clang - It should not be possible to go here */

   radio = NULL;		/* Avoid warning */
   for (i = 0; i < num_groups; i++)
     {
	di = DialogAddItem(table, DITEM_RADIOBUTTON);
	if (i == 0)
	   radio = di;
	DialogItemSetColSpan(di, 2);
	DialogItemSetCallback(di, _DlgSelectCbGroupChoose, i, NULL);
	DialogItemSetText(di, group_member_strings[i]);
	DialogItemRadioButtonSetFirst(di, radio);
	DialogItemRadioButtonGroupSetVal(di, i);
     }
   DialogItemRadioButtonGroupSetValPtr(radio, &dd->cur_grp);

   StrlistFree(group_member_strings, num_groups);
}

static const DialogDef DlgGroupChoose = {
   "GROUP_SELECTION",
   NULL, N_("Window Group Selection"),
   sizeof(GroupSelDlgData),
   SOUND_SETTINGS_GROUP,
   "pix/group.png",
   N_("Enlightenment Window Group\n" "Selection Dialog"),
   _DlgFillGroupChoose,
   DLG_OC, _DlgApplyGroupChoose, _DlgExitGroupChoose
};

static void
_EwinGroupChooseDialog(EWin * ewin, int action)
{
   int                 group_sel;
   GroupSelDlgData     gsdd, *dd = &gsdd;

   if (!ewin)
      return;
   dd->ewin = ewin;

   dd->action = action;
   dd->cur_grp = dd->prv_grp = 0;

   switch (action)
     {
     default:
	return;
     case GROUP_OP_ADD:
	dd->message = _("Pick the group the window will belong to:");
	group_sel = GROUP_SELECT_ALL_EXCEPT_EWIN;
	break;
     case GROUP_OP_DEL:
	dd->message = _("Select the group to remove the window from:");
	group_sel = GROUP_SELECT_EWIN_ONLY;
	break;
     case GROUP_OP_BREAK:
	dd->message = _("Select the group to break:");
	group_sel = GROUP_SELECT_EWIN_ONLY;
	break;
     }

   dd->groups = _EwinListGroups(dd->ewin, group_sel, &dd->group_num);

   if (!dd->groups)
     {
	if (action == GROUP_OP_BREAK || action == GROUP_OP_DEL)
	  {
	     DialogOK(_("Window Group Error"),
		      _
		      ("This window currently does not belong to any groups.\n"
		       "You can only destroy groups or remove windows from groups\n"
		       "through a window that actually belongs to at least one group."));
	     return;
	  }

	if (group_sel == GROUP_SELECT_ALL_EXCEPT_EWIN)
	  {
	     DialogOK(_("Window Group Error"),
		      _("Currently, no groups exist or this window\n"
			"already belongs to all existing groups.\n"
			"You have to start other groups first."));
	     return;
	  }

	DialogOK(_("Window Group Error"),
		 _
		 ("Currently, no groups exist. You have to start a group first."));
	return;
     }

   _EwinGroupsShowHide(dd->ewin, 0, SET_ON);

   DialogShowSimple(&DlgGroupChoose, dd);
}

typedef struct {
   EWin               *ewin;
   GroupConfig         cfg;	/* Dialog data for current group */
   GroupConfig        *cfgs;	/* Work copy of ewin group cfgs */
   int                 ngrp;
   int                 cur_grp;	/* Current  group */
   int                 prv_grp;	/* Previous group */
} EwinGroupDlgData;

static void
_DlgApplyGroups(Dialog * d, int val __UNUSED__, void *data __UNUSED__)
{
   EwinGroupDlgData   *dd = DLG_DATA_GET(d, EwinGroupDlgData);
   EWin               *ewin;
   int                 i;

   /* Check ewin */
   ewin = EwinFindByPtr(dd->ewin);
   if (ewin && ewin->num_groups != dd->ngrp)
      ewin = NULL;

   if (ewin)
     {
	dd->cfgs[dd->cur_grp] = dd->cfg;
	for (i = 0; i < ewin->num_groups; i++)
	   ewin->groups[i]->cfg = dd->cfgs[i];
     }

   _GroupsSave();
}

static void
_DlgExitGroups(Dialog * d)
{
   EwinGroupDlgData   *dd = DLG_DATA_GET(d, EwinGroupDlgData);
   EWin               *ewin;

   ewin = EwinFindByPtr(dd->ewin);
   _EwinGroupsShowHide(ewin, dd->cur_grp, SET_OFF);

   Efree(dd->cfgs);
}

static void
_DlgSelectCbGroups(Dialog * d, int val, void *data __UNUSED__)
{
   EwinGroupDlgData   *dd = DLG_DATA_GET(d, EwinGroupDlgData);

   /* val is equal to dd->cur_grp */
   dd->cfgs[dd->prv_grp] = dd->cfg;
   dd->cfg = dd->cfgs[val];
   DialogRedraw(d);
   _EwinGroupsShowHide(dd->ewin, dd->prv_grp, SET_OFF);
   _EwinGroupsShowHide(dd->ewin, val, SET_ON);
   dd->prv_grp = val;
}

static void
_DlgFillGroups(Dialog * d, DItem * table, void *data)
{
   EwinGroupDlgData   *dd = DLG_DATA_GET(d, EwinGroupDlgData);
   EWin               *ewin = (EWin *) data;
   DItem              *radio, *di;
   int                 i;
   char              **group_member_strings;

   dd->ewin = ewin;
   dd->cfgs = EMALLOC(GroupConfig, ewin->num_groups);
   dd->ngrp = ewin->num_groups;
   dd->cur_grp = dd->prv_grp = 0;
   for (i = 0; i < ewin->num_groups; i++)
      dd->cfgs[i] = ewin->groups[i]->cfg;
   dd->cfg = dd->cfgs[0];

   _EwinGroupsShowHide(ewin, 0, SET_ON);

   DialogItemTableSetOptions(table, 2, 0, 0, 0);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 2);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetText(di, _("Pick the group to configure:"));

   group_member_strings = _GrouplistMemberNames(ewin->groups, ewin->num_groups);
   if (!group_member_strings)
      return;			/* Silence clang - It should not be possible to go here */

   radio = NULL;		/* Avoid warning */
   for (i = 0; i < ewin->num_groups; i++)
     {
	di = DialogAddItem(table, DITEM_RADIOBUTTON);
	if (i == 0)
	   radio = di;
	DialogItemSetColSpan(di, 2);
	DialogItemSetCallback(di, _DlgSelectCbGroups, i, d);
	DialogItemSetText(di, group_member_strings[i]);
	DialogItemRadioButtonSetFirst(di, radio);
	DialogItemRadioButtonGroupSetVal(di, i);
     }
   DialogItemRadioButtonGroupSetValPtr(radio, &dd->cur_grp);

   StrlistFree(group_member_strings, ewin->num_groups);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 2);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 2);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetText(di, _("The following actions are\n"
			   "applied to all group members:"));

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Changing Border Style"));
   DialogItemCheckButtonSetPtr(di, &(dd->cfg.set_border));

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Iconifying"));
   DialogItemCheckButtonSetPtr(di, &(dd->cfg.iconify));

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Killing"));
   DialogItemCheckButtonSetPtr(di, &(dd->cfg.kill));

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Moving"));
   DialogItemCheckButtonSetPtr(di, &(dd->cfg.move));

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Raising/Lowering"));
   DialogItemCheckButtonSetPtr(di, &(dd->cfg.raise));

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Sticking"));
   DialogItemCheckButtonSetPtr(di, &(dd->cfg.stick));

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Shading"));
   DialogItemCheckButtonSetPtr(di, &(dd->cfg.shade));
}

static const DialogDef DlgGroups = {
   "CONFIGURE_GROUP",
   NULL, N_("Window Group Settings"),
   sizeof(EwinGroupDlgData),
   SOUND_SETTINGS_GROUP,
   "pix/group.png",
   N_("Enlightenment Window Group\n" "Settings Dialog"),
   _DlgFillGroups,
   DLG_OAC, _DlgApplyGroups, _DlgExitGroups
};

static void
_EwinGroupsConfig(EWin * ewin)
{
   if (!ewin)
      return;

   if (ewin->num_groups == 0)
     {
	DialogOK(_("Window Group Error"),
		 _("This window currently does not belong to any groups."));
	return;
     }

   DialogShowSimple(&DlgGroups, ewin);
}

typedef struct {
   GroupConfig         group_cfg;
   char                group_swap;
} GroupCfgDlgData;

static void
_DlgApplyGroupDefaults(Dialog * d, int val __UNUSED__, void *data __UNUSED__)
{
   GroupCfgDlgData    *dd = DLG_DATA_GET(d, GroupCfgDlgData);

   Conf_groups.dflt = dd->group_cfg;
   Conf_groups.swapmove = dd->group_swap;

   autosave();
}

static void
_DlgFillGroupDefaults(Dialog * d, DItem * table, void *data __UNUSED__)
{
   GroupCfgDlgData    *dd = DLG_DATA_GET(d, GroupCfgDlgData);
   DItem              *di;

   dd->group_cfg = Conf_groups.dflt;
   dd->group_swap = Conf_groups.swapmove;

   DialogItemTableSetOptions(table, 2, 0, 0, 0);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 2);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetText(di, _("Per-group settings:"));

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 2);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Changing Border Style"));
   DialogItemCheckButtonSetPtr(di, &dd->group_cfg.set_border);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Iconifying"));
   DialogItemCheckButtonSetPtr(di, &dd->group_cfg.iconify);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Killing"));
   DialogItemCheckButtonSetPtr(di, &dd->group_cfg.kill);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Moving"));
   DialogItemCheckButtonSetPtr(di, &dd->group_cfg.move);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Raising/Lowering"));
   DialogItemCheckButtonSetPtr(di, &dd->group_cfg.raise);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Sticking"));
   DialogItemCheckButtonSetPtr(di, &dd->group_cfg.stick);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Shading"));
   DialogItemCheckButtonSetPtr(di, &dd->group_cfg.shade);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 2);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 2);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetText(di, _("Global settings:"));

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Swap Window Locations"));
   DialogItemCheckButtonSetPtr(di, &dd->group_swap);
}

const DialogDef     DlgGroupDefaults = {
   "CONFIGURE_DEFAULT_GROUP_CONTROL",
   N_("Groups"), N_("Default Group Control Settings"),
   sizeof(GroupCfgDlgData),
   SOUND_SETTINGS_GROUP,
   "pix/group.png",
   N_("Enlightenment Default\n" "Group Control Settings Dialog"),
   _DlgFillGroupDefaults,
   DLG_OAC, _DlgApplyGroupDefaults, NULL
};

static void
_GroupsConfigure(const char *params)
{
   char                s[128];
   const char         *p;
   int                 l;
   EWin               *ewin;

   p = params;
   l = 0;
   s[0] = '\0';
   sscanf(p, "%100s %n", s, &l);
   p += l;

   ewin = GetContextEwin();
   if (*p)
     {
	ewin = EwinFindByExpr(p);
	if (!ewin)
	  {
	     IpcPrintf("Error: no such window: %s\n", p);
	     return;
	  }
     }

   if (!strcmp(s, "group"))
     {
	_EwinGroupsConfig(ewin);
     }
   else if (!strcmp(s, "add"))
     {
	_EwinGroupChooseDialog(ewin, GROUP_OP_ADD);
     }
   else if (!strcmp(s, "del"))
     {
	_EwinGroupChooseDialog(ewin, GROUP_OP_DEL);
     }
   else if (!strcmp(s, "break"))
     {
	_EwinGroupChooseDialog(ewin, GROUP_OP_BREAK);
     }
}
#endif /* ENABLE_DIALOGS */

/*
 * Groups module
 */

static void
_GroupShow(Group * g)
{
   int                 j;

   for (j = 0; j < g->num_members; j++)
      IpcPrintf("%d: %s\n", g->index, EwinGetIcccmName(g->members[j]));

   IpcPrintf("        index: %d\n" "  num_members: %d\n"
	     "      iconify: %d\n" "         kill: %d\n"
	     "         move: %d\n" "        raise: %d\n"
	     "   set_border: %d\n" "        stick: %d\n"
	     "        shade: %d\n",
	     g->index, g->num_members,
	     g->cfg.iconify, g->cfg.kill,
	     g->cfg.move, g->cfg.raise,
	     g->cfg.set_border, g->cfg.stick, g->cfg.shade);
}

static void
IPC_GroupOps(const char *params)
{
   Group              *group;
   char                windowid[128];
   char                operation[128];
   char                groupid[128];
   unsigned int        win;
   EWin               *ewin;

   if (!params)
     {
	IpcPrintf("Error: no window specified\n");
	return;
     }

   windowid[0] = operation[0] = groupid[0] = '\0';
   sscanf(params, "%100s %100s %100s", windowid, operation, groupid);
   win = 0;
   sscanf(windowid, "%x", &win);

   if (!operation[0])
     {
	IpcPrintf("Error: no operation specified\n");
	return;
     }

   ewin = EwinFindByExpr(windowid);
   if (!ewin)
     {
	IpcPrintf("Error: no such window: %s\n", windowid);
	return;
     }

   if (!strcmp(operation, "start"))
     {
	group = _GroupCreate(-1, 1);
	Mode_groups.current = group;
	_GroupEwinAdd(group, ewin, 1);
	IpcPrintf("start %8x\n", win);
     }
   else if (!strcmp(operation, "add"))
     {
	group = _GroupFind2(groupid);
	_GroupEwinAdd(group, ewin, 1);
	IpcPrintf("add %8x\n", win);
     }
   else if (!strcmp(operation, "del"))
     {
	group = _GroupFind2(groupid);
	_GroupEwinRemove(group, ewin, 1);
	IpcPrintf("del %8x\n", win);
     }
   else if (!strcmp(operation, "break"))
     {
	group = _GroupFind2(groupid);
	_GroupDelete(group);
	IpcPrintf("break %8x\n", win);
     }
   else if (!strcmp(operation, "showhide"))
     {
	_EwinGroupsShowHide(ewin, -1, SET_TOGGLE);
	IpcPrintf("showhide %8x\n", win);
     }
   else
     {
	IpcPrintf("Error: no such operation: %s\n", operation);
	return;
     }
}

static void
IPC_Group(const char *params)
{
   char                groupid[128];
   char                operation[128];
   char                param1[128];
   Group              *group;
   int                 onoff;

   if (!params)
     {
	IpcPrintf("Error: no operation specified\n");
	return;
     }

   groupid[0] = operation[0] = param1[0] = '\0';
   sscanf(params, "%100s %100s %100s", groupid, operation, param1);

   if (!strcmp(groupid, "info"))
     {
	IpcPrintf("Number of groups: %d\n", LIST_GET_COUNT(&group_list));
	LIST_FOR_EACH(Group, &group_list, group) _GroupShow(group);
	return;
     }

   if (!operation[0])
     {
	IpcPrintf("Error: no operation specified\n");
	return;
     }

   group = _GroupFind2(groupid);
   if (!group)
     {
	IpcPrintf("Error: no such group: %s\n", groupid);
	return;
     }

   if (!strcmp(operation, "info"))
     {
	_GroupShow(group);
	return;
     }
   else if (!strcmp(operation, "del"))
     {
	_GroupDelete(group);
	return;
     }

   if (!param1[0])
     {
	IpcPrintf("Error: no mode specified\n");
	return;
     }

   onoff = -1;
   if (!strcmp(param1, "on"))
      onoff = 1;
   else if (!strcmp(param1, "off"))
      onoff = 0;

   if (onoff == -1 && strcmp(param1, "?"))
     {
	IpcPrintf("Error: unknown mode specified\n");
     }
   else if (!strcmp(operation, "iconify"))
     {
	if (onoff >= 0)
	   group->cfg.iconify = onoff;
	else
	   onoff = group->cfg.iconify;
     }
   else if (!strcmp(operation, "kill"))
     {
	if (onoff >= 0)
	   group->cfg.kill = onoff;
	else
	   onoff = group->cfg.kill;
     }
   else if (!strcmp(operation, "move"))
     {
	if (onoff >= 0)
	   group->cfg.move = onoff;
	else
	   onoff = group->cfg.move;
     }
   else if (!strcmp(operation, "raise"))
     {
	if (onoff >= 0)
	   group->cfg.raise = onoff;
	else
	   onoff = group->cfg.raise;
     }
   else if (!strcmp(operation, "set_border"))
     {
	if (onoff >= 0)
	   group->cfg.set_border = onoff;
	else
	   onoff = group->cfg.set_border;
     }
   else if (!strcmp(operation, "stick"))
     {
	if (onoff >= 0)
	   group->cfg.stick = onoff;
	else
	   onoff = group->cfg.stick;
     }
   else if (!strcmp(operation, "shade"))
     {
	if (onoff >= 0)
	   group->cfg.shade = onoff;
	else
	   onoff = group->cfg.shade;
     }
   else
     {
	IpcPrintf("Error: no such operation: %s\n", operation);
	onoff = -1;
     }

   if (onoff == 1)
      IpcPrintf("%s: on\n", operation);
   else if (onoff == 0)
      IpcPrintf("%s: off\n", operation);

   if (onoff >= 0)
      _GroupsSave();
}

static void
IPC_GroupsConfig(const char *params)
{
   const char         *p;
   char                cmd[128];
   int                 len;

   cmd[0] = '\0';
   p = params;
   if (p)
     {
	len = 0;
	sscanf(p, "%100s %n", cmd, &len);
	p += len;
     }

   if (!p || cmd[0] == '?')
     {
	/* Show groups */
     }
#if ENABLE_DIALOGS
   else if (!strcmp(cmd, "cfg"))
     {
	_GroupsConfigure(p);
     }
#endif
}

static const IpcItem GroupsIpcArray[] = {
   {
    IPC_GroupsConfig,
    "groups", "grp",
    "Configure window groups",
    "  groups cfg group <windowid>  Configure windows groups\n"
    "  groups cfg add   <windowid>  Add window to group\n"
    "  groups cfg del   <windowid>  Remove window from group\n"
    "  groups cfg break <windowid>  Destroy one of the windows groups\n"}
   ,
   {
    IPC_GroupOps,
    "group_op", "gop",
    "Group operations",
    "use \"group_op <windowid> <property> [<value>]\" to perform "
    "group operations on a window.\n" "Available group_op commands are:\n"
    "  group_op <windowid> start\n"
    "  group_op <windowid> add [<group_index>]\n"
    "  group_op <windowid> del [<group_index>]\n"
    "  group_op <windowid> break [<group_index>]\n"
    "  group_op <windowid> showhide\n"}
   ,
   {
    IPC_Group,
    "group", "gc",
    "Group commands",
    "use \"group <groupid> <property> <value>\" to set group properties.\n"
    "Available group commands are:\n"
    "  group info\n"
    "  group <groupid> info\n"
    "  group <groupid> del\n"
    "  group <groupid> iconify <on/off/?>\n"
    "  group <groupid> kill <on/off/?>\n"
    "  group <groupid> move <on/off/?>\n"
    "  group <groupid> raise <on/off/?>\n"
    "  group <groupid> set_border <on/off/?>\n"
    "  group <groupid> stick <on/off/?>\n"
    "  group <groupid> shade <on/off/?>\n"}
   ,
};
#define N_IPC_FUNCS (sizeof(GroupsIpcArray)/sizeof(IpcItem))

/*
 * Configuration items
 */
static const CfgItem GroupsCfgItems[] = {
   CFG_ITEM_BOOL(Conf_groups, dflt.iconify, 1),
   CFG_ITEM_BOOL(Conf_groups, dflt.kill, 0),
   CFG_ITEM_BOOL(Conf_groups, dflt.move, 1),
   CFG_ITEM_BOOL(Conf_groups, dflt.raise, 0),
   CFG_ITEM_BOOL(Conf_groups, dflt.set_border, 1),
   CFG_ITEM_BOOL(Conf_groups, dflt.stick, 1),
   CFG_ITEM_BOOL(Conf_groups, dflt.shade, 1),
   CFG_ITEM_BOOL(Conf_groups, swapmove, 1),
};
#define N_CFG_ITEMS (sizeof(GroupsCfgItems)/sizeof(CfgItem))

extern const EModule ModGroups;

const EModule       ModGroups = {
   "groups", "grp",
   NULL,
   {N_IPC_FUNCS, GroupsIpcArray},
   {N_CFG_ITEMS, GroupsCfgItems}
};
