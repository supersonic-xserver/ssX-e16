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
#include "E.h"
#if ENABLE_SOUND
#include "dialog.h"
#include "emodule.h"
#include "file.h"
#include "list.h"
#include "settings.h"
#include "sound.h"
#include "sounds.h"

#if USE_SOUND_ESD
#define SOUND_SERVER_NAME "esd"
#define SOUND_MODULE_NAME "esd"
#elif USE_SOUND_PULSE
#define SOUND_SERVER_NAME "pulseaudio"
#define SOUND_MODULE_NAME "pulse"
#elif USE_SOUND_SNDIO
#define SOUND_SERVER_NAME "sndio"
#define SOUND_MODULE_NAME "sndio"
#elif USE_SOUND_ALSA
#define SOUND_SERVER_NAME "ALSA"
#define SOUND_MODULE_NAME "alsa"
#elif USE_SOUND_PLAYER
#define SOUND_SERVER_NAME SOUND_PLAYER_FMT
#define SOUND_MODULE_NAME "player"
#else
#error Invalid sound configuration
#endif

#define N_SOUNDS (SOUND_NOT_USED - 1)

typedef struct {
   dlist_t             list;
   char               *name;
   char               *file;
   Sample             *sample;
} SoundClass;

#define SC_NAME(sc) ((sc) ? (sc)->name : "(none)")

static struct {
   char                enable;
   char               *theme;
   unsigned int        mask1, mask2;
} Conf_sound;

static struct {
   char                cfg_loaded;
   char               *theme_path;
} Mode_sound;

#define SOUND_THEME_PATH ((Mode_sound.theme_path) ? Mode_sound.theme_path : Mode.theme.path)

static              LIST_HEAD(sound_list);

#if USE_MODULES
static const SoundOps *ops = NULL;
#else
#if USE_SOUND_ESD
extern const SoundOps SoundOps_esd;
static const SoundOps *ops = &SoundOps_esd;
#elif USE_SOUND_PULSE
extern const SoundOps SoundOps_pulse;
static const SoundOps *ops = &SoundOps_pulse;
#elif USE_SOUND_SNDIO
extern const SoundOps SoundOps_sndio;
static const SoundOps *ops = &SoundOps_sndio;
#elif USE_SOUND_ALSA
extern const SoundOps SoundOps_alsa;
static const SoundOps *ops = &SoundOps_alsa;
#elif USE_SOUND_PLAYER
extern const SoundOps SoundOps_player;
static const SoundOps *ops = &SoundOps_player;
#endif
#endif

static void         _SoundConfigLoad(void);

static const char  *const sound_names[N_SOUNDS] = {
   "SOUND_ALERT",		/*  0 0x00000001 */
   "SOUND_BUTTON_CLICK",
   "SOUND_BUTTON_RAISE",
   "SOUND_DEICONIFY",
   "SOUND_DESKTOP_LOWER",	/*  4 0x00000010 */
   "SOUND_DESKTOP_RAISE",
   "SOUND_DESKTOP_SHUT",
   "SOUND_ERROR_IPC",
   "SOUND_EXIT",		/*  8 0x00000100 */
   "SOUND_FOCUS_SET",
   "SOUND_ICONIFY",
   "SOUND_INSERT_KEYS",
   "SOUND_LOGOUT",		/* 12 0x00001000 */
   "SOUND_LOWER",
   "SOUND_MENU_SHOW",
   "SOUND_MOVE_AREA_DOWN",
   "SOUND_MOVE_AREA_LEFT",	/* 16 0x00010000 */
   "SOUND_MOVE_AREA_RIGHT",
   "SOUND_MOVE_AREA_UP",
   "SOUND_MOVE_RESIST",
   "SOUND_MOVE_START",		/* 20 0x00100000 */
   "SOUND_MOVE_STOP",
   "SOUND_RAISE",
   "SOUND_RESIZE_START",
   "SOUND_RESIZE_STOP",		/* 24 0x01000000 */
   "SOUND_SCANNING",
   "SOUND_SETTINGS_ACTIVE",
   "SOUND_SETTINGS_ALL",
   "SOUND_SETTINGS_AREA",	/* 28 0x10000000 */
   "SOUND_SETTINGS_AUDIO",
   NULL,
   "SOUND_SETTINGS_BG",
   "SOUND_SETTINGS_COMPOSITE",	/*  0 0x00000001 */
   "SOUND_SETTINGS_DESKTOPS",
   "SOUND_SETTINGS_FOCUS",
   "SOUND_SETTINGS_FX",
   "SOUND_SETTINGS_GROUP",	/*  4 0x00000010 */
   "SOUND_SETTINGS_ICONBOX",
   "SOUND_SETTINGS_MENUS",
   "SOUND_SETTINGS_MISCELLANEOUS",
   "SOUND_SETTINGS_MOVERESIZE",	/*  8 0x00000100 */
   "SOUND_SETTINGS_PAGER",
   "SOUND_SETTINGS_PLACEMENT",
   "SOUND_SETTINGS_SESSION",
   "SOUND_SETTINGS_TOOLTIPS",	/* 12 0x00001000 */
   "SOUND_SETTINGS_TRANS",
   "SOUND_SHADE",
   "SOUND_SLIDEOUT_SHOW",
   "SOUND_STARTUP",		/* 16 0x00010000 */
   "SOUND_UNSHADE",
   "SOUND_WAIT",
   "SOUND_WINDOW_BORDER_CHANGE",
   "SOUND_WINDOW_CHANGE_LAYER_DOWN",	/* 20 0x00100000 */
   "SOUND_WINDOW_CHANGE_LAYER_UP",
   "SOUND_WINDOW_CLOSE",
   "SOUND_WINDOW_SLIDE",
   "SOUND_WINDOW_SLIDE_END",	/* 24 0x01000000 */
   "SOUND_WINDOW_STICK",
   "SOUND_WINDOW_UNSTICK",
};

static void
_SclassSampleDestroy(void *data, void *user_data __UNUSED__)
{
   SoundClass         *sclass = (SoundClass *) data;

   if (!sclass || !sclass->sample)
      return;

   if (ops)
      ops->SampleDestroy(sclass->sample);

   sclass->sample = NULL;
}

static SoundClass  *
_SclassCreate(const char *name, const char *file)
{
   SoundClass         *sclass;

   sclass = EMALLOC(SoundClass, 1);
   if (!sclass)
      return NULL;

   LIST_PREPEND(SoundClass, &sound_list, sclass);

   sclass->name = Estrdup(name);
   sclass->file = Estrdup(file);
   sclass->sample = NULL;

   return sclass;
}

static void
_SclassDestroy(SoundClass * sclass)
{
   if (!sclass)
      return;

   LIST_REMOVE(SoundClass, &sound_list, sclass);
   _SclassSampleDestroy(sclass, NULL);
   Efree(sclass->name);
   Efree(sclass->file);

   Efree(sclass);
}

static void
_SclassApply(SoundClass * sclass)
{
   if (!sclass || !Conf_sound.enable)
      return;

   if (!sclass->sample)
     {
	char               *file;

	file = FindFile(sclass->file, SOUND_THEME_PATH, FILE_TYPE_SOUND);
	if (file)
	  {
	     sclass->sample = ops->SampleLoad(file);
	     Efree(file);
	  }
	if (!sclass->sample)
	  {
	     DialogOK(_("Error finding sound file"),
		      _("Warning!  Enlightenment was unable to load the\n"
			"following sound file:\n%s\n"
			"Enlightenment will continue to operate, but you\n"
			"may wish to check your configuration settings.\n"),
		      sclass->file);
	     _SclassDestroy(sclass);
	     return;
	  }
     }

   ops->SamplePlay(sclass->sample);
}

static int
_SclassMatchName(const void *data, const void *match)
{
   return strcmp(((const SoundClass *)data)->name, (const char *)match);
}

static SoundClass  *
_SclassFind(const char *name)
{
   return LIST_FIND(SoundClass, &sound_list, _SclassMatchName, name);
}

static void
_SoundPlayByName(const char *name)
{
   SoundClass         *sclass;

   if (!Conf_sound.enable)
      return;

   if (!name || !*name)
      return;

   sclass = _SclassFind(name);

   if (EDebug(EDBUG_TYPE_SOUND))
      Eprintf("%s: %s file=%s\n", "SclassApply", name, SC_NAME(sclass));

   _SclassApply(sclass);
}

#define _SoundMasked(i) \
    (((i) <= 32) ? Conf_sound.mask1 & (1 << ((i) - 1)) : \
                   Conf_sound.mask2 & (1 << ((i) - 33)))
void
SoundPlay(int sound)
{
   if (!Conf_sound.enable)
      return;

   if (sound <= 0 || sound > N_SOUNDS)
      return;

   if (_SoundMasked(sound))
      return;

   _SoundPlayByName(sound_names[sound - 1]);
}

static int
_SoundFree(const char *name)
{
   SoundClass         *sclass;

   sclass = _SclassFind(name);
   _SclassDestroy(sclass);

   return !!sclass;
}

static void
_SoundInit(void)
{
   if (!Conf_sound.enable)
      return;

#if USE_MODULES
   if (!ops)
      ops = ModLoadSym("sound", "SoundOps", SOUND_MODULE_NAME);
#endif

   if (!ops || ops->Init())
     {
	Conf_sound.enable = 0;
	DialogOK(_("Error initialising sound"),
		 _
		 ("Audio was enabled for Enlightenment but there was an error\n"
		  "communicating with the audio server (%s).\n"
		  "Audio will now be disabled.\n"), SOUND_SERVER_NAME);
	return;
     }

   _SoundConfigLoad();
}

static void
_SoundExit(void)
{
   SoundClass         *sc;

   LIST_FOR_EACH(SoundClass, &sound_list, sc) _SclassSampleDestroy(sc, NULL);

   if (ops)
      ops->Exit();

   Conf_sound.enable = 0;
}

static void
_SoundConfigure(int enable)
{
   if (Conf_sound.enable == enable)
      return;
   Conf_sound.enable = enable;

   if (Conf_sound.enable)
      _SoundInit();
   else
      _SoundExit();
}

/*
 * Configuration load/save
 */

static int
_SoundConfigParse(FILE * fs)
{
   int                 err = 0;
   char                s[FILEPATH_LEN_MAX];
   char                s1[FILEPATH_LEN_MAX];
   char                s2[FILEPATH_LEN_MAX];
   int                 i1, fields;

   while (GetLine(s, sizeof(s), fs))
     {
	i1 = -1;
	fields = sscanf(s, "%d", &i1);
	if (fields == 1)	/* Just skip the numeric config stuff */
	   continue;

	s1[0] = s2[0] = '\0';
	fields = sscanf(s, "%4000s %4000s", s1, s2);
	if (fields != 2)
	  {
	     Eprintf("*** Ignoring line: %s\n", s);
	     continue;
	  }
	_SclassCreate(s1, s2);
     }
#if 0				/* Errors here are just ignored */
   if (err)
      ConfigAlertLoad("Sound");
#endif

   return err;
}

static void
_SoundConfigLoad(void)
{
   if (Mode_sound.cfg_loaded)
      return;
   Mode_sound.cfg_loaded = 1;

   Efree(Mode_sound.theme_path);
   if (Conf_sound.theme)
      Mode_sound.theme_path = ThemePathFind(Conf_sound.theme);
   else
      Mode_sound.theme_path = NULL;

   ConfigFileLoad("sound.cfg", SOUND_THEME_PATH, _SoundConfigParse, 1);
}

static void
_SoundConfigUnload(void)
{
   SoundClass         *sc, *tmp;

   LIST_FOR_EACH_SAFE(SoundClass, &sound_list, sc, tmp)
   {
      _SclassDestroy(sc);
   }

   Mode_sound.cfg_loaded = 0;
}

static void
_SoundEnableChange(void *item __UNUSED__, const char *sval)
{
   _SoundConfigure(!!atoi(sval));
}

static void
_SoundThemeChange(void *item __UNUSED__, const char *theme)
{
   if (*theme == '\0')
      theme = NULL;
   _SoundConfigUnload();
   EFREE_DUP(Conf_sound.theme, theme);
   _SoundConfigLoad();
}

/*
 * Sound module
 */

static void
_SoundSighan(int sig, void *prm __UNUSED__)
{
   switch (sig)
     {
     case ESIGNAL_INIT:
	memset(&Mode_sound, 0, sizeof(Mode_sound));
	break;
     case ESIGNAL_CONFIGURE:
	_SoundInit();
	break;
     case ESIGNAL_START:
	if (!Conf_sound.enable)
	   break;
	SoundPlay(SOUND_STARTUP);
	_SoundFree("SOUND_STARTUP");
	break;
     case ESIGNAL_EXIT:
/*      if (Mode.wm.master) */
	_SoundExit();
	break;
     }
}

#if ENABLE_DIALOGS
/*
 * Configuration dialog
 */
static char         tmp_audio;

static void
_Dlg_ApplySound(Dialog * d __UNUSED__, int val __UNUSED__,
		void *data __UNUSED__)
{
   _SoundConfigure(tmp_audio);
   autosave();
}

static void
_DlgFillSound(Dialog * d __UNUSED__, DItem * table, void *data __UNUSED__)
{
   DItem              *di;

   tmp_audio = Conf_sound.enable;

   DialogItemTableSetOptions(table, 2, 0, 0, 0);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Enable sounds"));
   DialogItemCheckButtonSetPtr(di, &tmp_audio);
}

const DialogDef     DlgSound = {
   "CONFIGURE_AUDIO",
   N_("Sound"), N_("Audio Settings"),
   0,
   SOUND_SETTINGS_AUDIO,
   "pix/sound.png",
   N_("Enlightenment Audio\n" "Settings Dialog"),
   _DlgFillSound,
   DLG_OAC, _Dlg_ApplySound, NULL
};
#endif /* ENABLE_DIALOGS */

/*
 * IPC functions
 */

static void
SoundIpc(const char *params)
{
   const char         *p;
   char                cmd[128], prm[4096];
   int                 len;
   SoundClass         *sc;

   cmd[0] = prm[0] = '\0';
   p = params;
   if (p)
     {
	len = 0;
	sscanf(p, "%100s %4000s %n", cmd, prm, &len);
	p += len;
     }

   if (!strncmp(cmd, "del", 3))
     {
	_SoundFree(prm);
     }
   else if (!strncmp(cmd, "list", 2))
     {
	LIST_FOR_EACH(SoundClass, &sound_list, sc) IpcPrintf("%s\n", sc->name);
     }
   else if (!strncmp(cmd, "new", 3))
     {
	_SclassCreate(prm, p);
     }
   else if (!strncmp(cmd, "off", 2))
     {
	_SoundConfigure(0);
	autosave();
     }
   else if (!strncmp(cmd, "on", 2))
     {
	_SoundConfigure(1);
	autosave();
     }
   else if (!strncmp(cmd, "play", 2))
     {
	_SoundPlayByName(prm);
     }
}

static const IpcItem SoundIpcArray[] = {
   {
    SoundIpc,
    "sound", "snd",
    "Sound functions",
    "  sound add <classname> <filename> Create soundclass\n"
    "  sound del <classname>            Delete soundclass\n"
    "  sound list                       Show all sounds\n"
    "  sound off                        Disable sounds\n"
    "  sound on                         Enable sounds\n"
    "  sound play <classname>           Play sounds\n"}
};
#define N_IPC_FUNCS (sizeof(SoundIpcArray)/sizeof(IpcItem))

static const CfgItem SoundCfgItems[] = {
   CFG_FUNC_BOOL(Conf_sound, enable, 0, _SoundEnableChange),
   CFG_FUNC_STR(Conf_sound, theme, _SoundThemeChange),
   CFG_ITEM_HEX(Conf_sound, mask1, 0),
   CFG_ITEM_HEX(Conf_sound, mask2, 0),
};
#define N_CFG_ITEMS (sizeof(SoundCfgItems)/sizeof(CfgItem))

/*
 * Module descriptor
 */
extern const EModule ModSound;

const EModule       ModSound = {
   "sound", "audio",
   _SoundSighan,
   {N_IPC_FUNCS, SoundIpcArray},
   {N_CFG_ITEMS, SoundCfgItems}
};

#endif /* ENABLE_SOUND */
