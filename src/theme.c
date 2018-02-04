/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2018 Kim Woelders
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
#include "emodule.h"
#include "file.h"
#include "user.h"
#include "util.h"
#include "session.h"

/* Update Mode.theme.paths (theme path list) */
static void
_ThemePathsUpdate(void)
{
   char                paths[FILEPATH_LEN_MAX];

   Esnprintf(paths, sizeof(paths), "%s/themes:%s/.themes:%s/themes:%s",
	     EDirUser(), userhome(), EDirRoot(),
	     (Conf.theme.extra_path) ? Conf.theme.extra_path : "");
   _EFDUP(Mode.theme.paths, paths);
}

/* Check if this is a theme dir */
static int
_ThemeCheckDir(const char *path, const char *px)
{
   static const char  *const theme_files[] = {
      "init.cfg",
#if 0
      "epplets/epplets.cfg",
#endif
      NULL
   };
   const char         *tf;
   int                 i;
   char                s[FILEPATH_LEN_MAX];

   if (EDebug(EDBUG_TYPE_CONFIG))
      Eprintf("%s: %s%s\n", __func__, path, px);

   for (i = 0; (tf = theme_files[i]); i++)
     {
	Esnprintf(s, sizeof(s), "%s%s/%s", path, px, tf);
	if (!isfile(s))
	   return 0;
     }

   return 1;
}

static int
_ThemeCheckPath1(const char *path)
{
   if (_ThemeCheckDir(path, ""))
      return 1;

   if (_ThemeCheckDir(path, "/e16"))
      return 1;

   return 0;
}

static char        *
_ThemeCheckPath(const char *path)
{
   char                ss[FILEPATH_LEN_MAX];

   if (EDebug(EDBUG_TYPE_CONFIG))
      Eprintf("%s: %s\n", __func__, path);

   if (_ThemeCheckDir(path, ""))
      return Estrdup(path);

   if (_ThemeCheckDir(path, "/e16"))
     {
	Esnprintf(ss, sizeof(ss), "%s/e16", path);
	return Estrdup(ss);
     }

   return NULL;
}

char               *
ThemePathName(const char *path)
{
   const char         *p;
   char               *s;

   if (!path)
      return NULL;
   p = strrchr(path, '/');
   if (!p)
      return Estrdup(path);	/* Name only */
   if (strcmp(p + 1, "e16"))
      return Estrdup(p + 1);	/* Regular path */

   /* <path>/<themename>/e16 */
   s = Estrdup(path);
   s[p - path] = '\0';
   p = strrchr(s, '/');
   if (!p)
      return s;			/* Should not happen */
   p++;
   memmove(s, p, strlen(p) + 1);
   return s;
}

static void
_append_merge_dir(char *dir, char ***list, int *count)
{
   char                ss[FILEPATH_LEN_MAX], s1[FILEPATH_LEN_MAX];
   char              **str, *s;
   int                 i, num;

   str = E_ls(dir, &num);
   if (!str)
      return;

   for (i = 0; i < num; i++)
     {
	if (!strcmp(str[i], "DEFAULT"))
	   continue;

	Esnprintf(ss, sizeof(ss), "%s/%s", dir, str[i]);

	if (isdir(ss))
	  {
	     if (_ThemeCheckPath1(ss))
		goto got_one;
	     continue;
	  }

	if (!isfile(ss))
	   continue;

	s = strrchr(ss, '.');
	if (!s)
	   continue;

	if (strcmp(s + 1, "etheme") == 0)
	  {
	     Esnprintf(s1, sizeof(s1), "%s/themes/%s", EDirUser(), str[i]);
	     s = strrchr(s1, '.');
	     if (!s)
		continue;
	     *s = '\0';
	     if (!isdir(s1))
		goto got_one;
	  }

	if (strcmp(s + 1, "theme") == 0)
	  {
	     Esnprintf(s1, sizeof(s1), "%s/.themes/%s", userhome(), str[i]);
	     s = strrchr(s1, '.');
	     if (!s)
		continue;
	     *s = '\0';
	     if (!isdir(s1))
		goto got_one;
	  }

	continue;

      got_one:
	(*count)++;
	*list = EREALLOC(char *, *list, *count);

	(*list)[(*count) - 1] = Estrdup(ss);
     }
   StrlistFree(str, num);
}

char              **
ThemesList(int *number)
{
   char              **lst, **list;
   int                 i, num, count;

   _ThemePathsUpdate();
   lst = StrlistFromString(Mode.theme.paths, ':', &num);

   count = 0;
   list = NULL;
   for (i = 0; i < num; i++)
      _append_merge_dir(lst[i], &list, &count);

   StrlistFree(lst, num);

   *number = count;
   return list;
}

static char        *
_ThemeExtract(const char *path)
{
   char                th[FILEPATH_LEN_MAX];
   FILE               *f;
   unsigned char       buf[262];
   size_t              ret;
   const char         *p;
   char                name[128], *type;

   if (EDebug(EDBUG_TYPE_CONFIG))
      Eprintf("%s: %s\n", __func__, path);

   /* its a file - check its type */
   f = fopen(path, "r");
   if (!f)
      return NULL;
   ret = fread(buf, 1, sizeof(buf), f);
   memset(buf + ret, 0, sizeof(buf) - ret);
   fclose(f);

   p = strrchr(path, '/');
   p = (p) ? p + 1 : path;
   Esnprintf(name, sizeof(name), "%s", p);
   type = strchr(name, '.');
   if (type)
      *type++ = '\0';

   if (type && strcmp(type, "theme") == 0)
     {
	Esnprintf(th, sizeof(th), "%s/.themes", userhome());
	if (!isdir(th))
	   E_md(th);
	Esnprintf(th, sizeof(th), "%s/.themes/%s", userhome(), name);
     }
   else
      Esnprintf(th, sizeof(th), "%s/themes/%s", EDirUser(), name);

   /* check magic numbers */
   if ((buf[0] == 31) && (buf[1] == 139))
     {
	/* gzipped tarball */
	E_md(th);
	Esystem("gzip -d -c < %s | (cd %s ; tar -xf -)", path, th);
     }
   else if ((buf[257] == 'u') && (buf[258] == 's') &&
	    (buf[259] == 't') && (buf[260] == 'a') && (buf[261] == 'r'))
     {
	/* vanilla tarball */
	E_md(th);
	Esystem("cat %s | (cd %s ; tar -xf -)", path, th);
     }
   else
      return NULL;

   return _ThemeCheckPath(th);
}

char               *
ThemeFind(const char *theme)
{
   static const char  *const default_themes[] = {
      "DEFAULT", "winter", "BrushedMetal-Tigert", "ShinyMetal", NULL
   };
   char                tpbuf[FILEPATH_LEN_MAX], *path;
   char              **lst;
   int                 i, j, num;

   if (EDebug(EDBUG_TYPE_CONFIG))
      Eprintf("%s: %s\n", __func__, theme);

   _ThemePathsUpdate();

   if (!theme || !theme[0])
      theme = NULL;		/* Lookup default */
   else if (!strcmp(theme, "-"))	/* Use fallbacks */
      return NULL;
   else if (exists(theme))
      goto check;

   lst = StrlistFromString(Mode.theme.paths, ':', &num);

   i = 0;
   do
     {
	if (!theme)
	   goto next;
	for (j = 0; j < num; j++)
	  {
	     Esnprintf(tpbuf, sizeof(tpbuf), "%s/%s", lst[j], theme);
	     if (exists(tpbuf))
	       {
		  theme = tpbuf;
		  goto done;
	       }
	  }
      next:
	theme = default_themes[i++];
     }
   while (theme);

 done:
   StrlistFree(lst, num);

 check:
   if (theme)
     {
	path = NULL;
	if (isdir(theme))
	   path = _ThemeCheckPath(theme);
	else if (isfile(theme))
	   path = _ThemeExtract(theme);
	if (path)
	   return path;
     }

   /* No theme found yet, just find any theme */
   lst = ThemesList(&num);
   if (!lst)
      return NULL;
   path = Estrdup(lst[0]);
   StrlistFree(lst, num);

   return path;
}

void
ThemePathFind(void)
{
   char               *name, *path, *s;

   /*
    * Conf.theme.name is read from the configuration.
    * Mode.theme.path may be assigned on the command line.
    */
   name = (Mode.theme.path) ? Mode.theme.path : Conf.theme.name;
   s = (name) ? strchr(name, '=') : NULL;
   if (s)
     {
	*s++ = '\0';
	Efree(Mode.theme.variant);
	Mode.theme.variant = Estrdup(s);
     }

   path = ThemeFind(name);

   if (!path && (!name || strcmp(name, "-")))
     {
	Alert(_("No themes were found in the default directories:\n"
		" %s\n"
		"Proceeding from here is mostly pointless.\n"),
	      Mode.theme.paths);
     }

   Efree(Conf.theme.name);
   Conf.theme.name = ThemePathName(path);

   Efree(Mode.theme.path);
   Mode.theme.path = (path) ? path : Estrdup("-");
}

#if ENABLE_DIALOGS
#include "dialog.h"
#include "settings.h"
/*
 * Configuration dialog
 */
static char         tmp_use_theme_font;
static char         tmp_use_alt_font;

static void
_DlgThemeApply(Dialog * d __UNUSED__, int val __UNUSED__, void *data __UNUSED__)
{
   if (Conf.theme.use_theme_font_cfg == tmp_use_theme_font &&
       Conf.theme.use_alt_font_cfg == tmp_use_alt_font)
      return;

   DialogOK(_("Message"), _("Changes will take effect after restart"));

   Conf.theme.use_theme_font_cfg = tmp_use_theme_font;
   Conf.theme.use_alt_font_cfg = tmp_use_alt_font;
   autosave();
}

static void
_DlgThemeFill(Dialog * d __UNUSED__, DItem * table, void *data __UNUSED__)
{
   DItem              *di;
   char                buf[1024];

   tmp_use_theme_font = Conf.theme.use_theme_font_cfg;
   tmp_use_alt_font = Conf.theme.use_alt_font_cfg;

   DialogItemTableSetOptions(table, 2, 0, 0, 0);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Use theme font configuration"));
   DialogItemCheckButtonSetPtr(di, &tmp_use_theme_font);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   Esnprintf(buf, sizeof(buf), _("Use alternate font configuration (%s)"),
	     Conf.theme.font_cfg ? Conf.theme.font_cfg : _("Not set"));
   DialogItemSetText(di, buf);
   DialogItemCheckButtonSetPtr(di, &tmp_use_alt_font);
}

const DialogDef     DlgTheme = {
   "CONFIGURE_AUDIO",
   N_("Theme"), N_("Theme Settings"),
   0,
   SOUND_SETTINGS_MISCELLANEOUS,
   "pix/miscellaneous.png",
   N_("Enlightenment Theme\n" "Settings Dialog"),
   _DlgThemeFill,
   DLG_OAC, _DlgThemeApply, NULL
};
#endif /* ENABLE_DIALOGS */

/*
 * Theme module
 */

static void
ThemesIpc(const char *params)
{
   const char         *p;
   char                cmd[128], prm[128];
   int                 len;

   cmd[0] = prm[0] = '\0';
   p = params;
   if (p)
     {
	len = 0;
	sscanf(p, "%100s %100s %n", cmd, prm, &len);
	p += len;
     }

   if (!p || cmd[0] == '?')
     {
	char               *path;

	IpcPrintf("Name: %s\n", (Conf.theme.name) ? Conf.theme.name : "-");
	IpcPrintf("Full: %s\n", Mode.theme.path);
	path = ThemeFind(NULL);
	IpcPrintf("Default: %s\n", path);
	Efree(path);
	IpcPrintf("Path: %s\n", Mode.theme.paths);
     }
   else if (!strncmp(cmd, "list", 2))
     {
	char              **lst;
	int                 i, num;

	lst = ThemesList(&num);
	if (!lst)
	   return;
	for (i = 0; i < num; i++)
	   IpcPrintf("%s\n", lst[i]);
	StrlistFree(lst, num);
     }
   else if (!strcmp(cmd, "use"))
     {
	/* FIXME - ThemeCheckIfValid(s) */
	SessionExit(EEXIT_THEME, prm);
     }
}

static const IpcItem ThemeIpcArray[] = {
   {
    ThemesIpc,
    "theme", "th",
    "Theme commands",
    "  theme             Show current theme\n"
    "  theme list        Show all themes\n"
    "  theme use <name>  Switch to theme <name>\n"}
   ,
};
#define N_IPC_FUNCS (sizeof(ThemeIpcArray)/sizeof(IpcItem))

static const CfgItem ThemeCfgItems[] = {
   CFG_ITEM_STR(Conf.theme, name),
   CFG_ITEM_STR(Conf.theme, extra_path),
   CFG_ITEM_BOOL(Conf.theme, use_theme_font_cfg, 0),
   CFG_ITEM_BOOL(Conf.theme, use_alt_font_cfg, 0),
   CFG_ITEM_STR(Conf.theme, font_cfg),
};
#define N_CFG_ITEMS (sizeof(ThemeCfgItems)/sizeof(CfgItem))

/*
 * Module descriptor
 */
extern const EModule ModTheme;

const EModule       ModTheme = {
   "theme", "th",
   NULL,
   {N_IPC_FUNCS, ThemeIpcArray},
   {N_CFG_ITEMS, ThemeCfgItems}
};
