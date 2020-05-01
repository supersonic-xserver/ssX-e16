/*
 * Copyright (c) 2012 Jonathan Armani <armani@openbsd.org>
 * Copyright (C) 2020 Kim Woelders
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
#if defined(ENABLE_SOUND) && defined(USE_SOUND_SNDIO)
#include "sound.h"
#include <sndio.h>

#ifdef USE_MODULES
#define Estrdup strdup
#endif

struct _sample {
   SoundSampleData     ssd;
};

static struct sio_hdl *hdl;

static Sample      *
_sound_sndio_Load(const char *file)
{
   Sample             *s;
   int                 err;

   if (hdl == NULL)
      return NULL;

   s = ECALLOC(Sample, 1);
   if (!s)
      return NULL;

   err = SoundSampleGetData(file, &s->ssd);
   if (err)
     {
	Efree(s);
	return NULL;
     }

   return s;
}

static void
_sound_sndio_Destroy(Sample * s)
{
   if (!s)
      return;

   EFREE_NULL(s->ssd.data);
   Efree(s);
}

static void
_sound_sndio_Play(Sample * s)
{
   struct sio_par      params;

   if (hdl == NULL || !s)
      return;

   sio_initpar(&params);
   params.bits = s->ssd.bit_per_sample;
   params.pchan = s->ssd.channels;
   params.rate = s->ssd.rate;

   if (!sio_setpar(hdl, &params))
      return;
   if (!sio_getpar(hdl, &params))
      return;
   if (params.bits != s->ssd.bit_per_sample ||
       params.pchan != s->ssd.channels || params.rate != s->ssd.rate)
      return;

   if (!sio_start(hdl))
      return;

   sio_write(hdl, s->ssd.data, s->ssd.size);
   sio_stop(hdl);
}

static int
_sound_sndio_Init(void)
{
   if (hdl != NULL)
      return 0;

   hdl = sio_open(SIO_DEVANY, SIO_PLAY, 0);

   return (hdl == NULL);
}

static void
_sound_sndio_Exit(void)
{
   if (hdl == NULL)
      return;

   sio_close(hdl);
   hdl = NULL;
}

__EXPORT__ extern const SoundOps SoundOps_sndio;

const SoundOps      SoundOps_sndio = {
   _sound_sndio_Init, _sound_sndio_Exit, _sound_sndio_Load,
   _sound_sndio_Destroy, _sound_sndio_Play,
};

#endif /* ENABLE_SOUND && USE_SOUND_SNDIO */
