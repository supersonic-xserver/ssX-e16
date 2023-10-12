/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2023 Kim Woelders
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
#include "file.h"

static void
_ExecSetStartupId(void)
{
   char                buf[128];
   Desk               *dsk;
   int                 ax, ay;

   dsk = DesksGetCurrent();
   DeskGetArea(dsk, &ax, &ay);

   Esnprintf(buf, sizeof(buf), "e16/%d:%d:%d,%d", Mode.apps.startup_id,
	     dsk->num, ax, ay);
   Esetenv("DESKTOP_STARTUP_ID", buf);
}

static void
_ExecSetupEnv(int flags)
{
   int                 fd;

   setsid();

   /* Close all file descriptors except the std 3 */
   for (fd = 3; fd < 1024; fd++)
      close(fd);

   /* Set up env stuff */
   if (flags & EXEC_SET_LANG)
      LangExport();
   if (flags & EXEC_SET_STARTUP_ID)
      _ExecSetStartupId();

#if USE_LIBHACK
   if (Mode.wm.window && !(flags & EXEC_NO_LIBHACK))
     {
	char                buf[1024];

	Esnprintf(buf, sizeof(buf), "%s/libhack.so", EDirLib());
	Esetenv("LD_PRELOAD", buf);
     }
#endif
}

void
Eexec(const char *cmd)
{
   char              **lst;
   int                 num;

   _ExecSetupEnv(EXEC_NO_LIBHACK);

   lst = StrlistFromString(cmd, ' ', &num);

   if (EDebug(EDBUG_TYPE_EXEC))
      Eprintf("%s: '%s'\n", __func__, cmd);

   execvp(lst[0], lst);

   StrlistFree(lst, num);
}

int
EspawnApplication(const char *params, int flags)
{
   int                 argc;
   char              **argv;

   if (!params)
      return -1;

   if (EDebug(EDBUG_TYPE_EXEC))
      Eprintf("%s: '%s'\n", __func__, params);

   Mode.apps.startup_id++;
   if (fork())
      return 0;

   _ExecSetupEnv(flags);

   argv = StrlistDecodeEscaped(params, &argc);
   if (argc <= 0)
      return -1;

   execvp(argv[0], argv);

   if (!Mode.wm.startup)
      AlertOK(_("There was a problem running the command\n '%s'\nError: %m"),
	      params);

   StrlistFree(argv, argc);

   exit(100);
}

static void
_Espawn(int argc __UNUSED__, char **argv)
{
   if (!argv || !argv[0])
      return;

   if (fork())
      return;

   _ExecSetupEnv(EXEC_SET_LANG | EXEC_SET_STARTUP_ID);

   execvp(argv[0], argv);

   AlertOK(_("There was a problem running the command\n '%s'\nError: %m"),
	   argv[0]);

   exit(100);
}

__EXPORT__ void
Espawn(const char *fmt, ...)
{
   va_list             args;
   char                buf[FILEPATH_LEN_MAX];
   int                 argc;
   char              **argv;

   va_start(args, fmt);
   vsnprintf(buf, sizeof(buf), fmt, args);
   va_end(args);

   argv = StrlistDecodeEscaped(buf, &argc);
   _Espawn(argc, argv);
   StrlistFree(argv, argc);
}

int
Esystem(const char *fmt, ...)
{
   va_list             args;
   char                buf[FILEPATH_LEN_MAX];

   va_start(args, fmt);
   vsnprintf(buf, sizeof(buf), fmt, args);
   va_end(args);

   return system(buf);
}
