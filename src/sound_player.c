/*
 * Copyright (C) 2021 Kim Woelders
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
#if defined(ENABLE_SOUND) && defined(USE_SOUND_PLAYER)
#include "file.h"
#include "sound.h"

#ifdef USE_MODULES
#define Estrdup strdup
#endif

static Sample      *
_sound_player_Load(const char *file)
{
   return (Sample *) Estrdup(file);
}

static void
_sound_player_Destroy(Sample * s)
{
   Efree(s);
}

static void
_sound_player_Play(Sample * s)
{
   Espawn(SOUND_PLAYER_FMT, (char *)s);
}

static int
_sound_player_Init(void)
{
   if (!path_canexec0(SOUND_PLAYER_FMT))
      return -1;
   return 0;
}

static void
_sound_player_Exit(void)
{
}

__EXPORT__ extern const SoundOps SoundOps_player;

const SoundOps      SoundOps_player = {
   _sound_player_Init, _sound_player_Exit,
   _sound_player_Load, _sound_player_Destroy, _sound_player_Play,
};

#endif /* ENABLE_SOUND && USE_SOUND_PLAYER */
