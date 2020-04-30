/*
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
#if defined(ENABLE_SOUND) && defined(USE_SOUND_ALSA)
#include "sound.h"
#include <alsa/asoundlib.h>

#define DO_FORK 1

#define PCM_DEVICE "default"

struct _sample {
   SoundSampleData     ssd;
};

static Sample      *
_sound_alsa_Load(const char *file)
{
   Sample             *s;
   int                 err;

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
_sound_alsa_Destroy(Sample * s)
{
   Efree(s->ssd.data);
   Efree(s);
}

static void
_sound_alsa_Play(Sample * s)
{
   int                 err;
   snd_pcm_t          *hdl;
   snd_pcm_hw_params_t *hwp;

#if DO_FORK
   if (fork())
      return;
#endif

   err = snd_pcm_open(&hdl, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
   if (err < 0)
     {
	Eprintf("%s: Open '%s': %s\n", __func__, PCM_DEVICE, snd_strerror(err));
#if DO_FORK
	exit(0);
#else
	return;
#endif
     }

   snd_pcm_hw_params_alloca(&hwp);
   snd_pcm_hw_params_any(hdl, hwp);

   err = snd_pcm_hw_params_set_access(hdl, hwp, SND_PCM_ACCESS_RW_INTERLEAVED);
   if (err < 0)
      Eprintf("%s: Set interleaved: %s\n", __func__, snd_strerror(err));

   err = snd_pcm_hw_params_set_format(hdl, hwp, SND_PCM_FORMAT_S16_LE);
   if (err < 0)
      Eprintf("%s: Set format: %s\n", __func__, snd_strerror(err));

   err = snd_pcm_hw_params_set_channels(hdl, hwp, s->ssd.channels);
   if (err < 0)
      Eprintf("%s: Set channels: %s\n", __func__, snd_strerror(err));

   err = snd_pcm_hw_params_set_rate(hdl, hwp, s->ssd.rate, 0);
   if (err < 0)
      Eprintf("%s: Set rate: %s\n", __func__, snd_strerror(err));

   err = snd_pcm_hw_params(hdl, hwp);
   if (err < 0)
      Eprintf("%s: Set HW prm: %s\n", __func__, snd_strerror(err));

   err = snd_pcm_writei(hdl, s->ssd.data, s->ssd.size / (2 * s->ssd.channels));
   if (err < 0)
      Eprintf("%s: Write: %s\n", __func__, snd_strerror(err));

   snd_pcm_drain(hdl);
   snd_pcm_close(hdl);

#if DO_FORK
   exit(0);
#endif
}

static int
_sound_alsa_Init(void)
{
   return 0;
}

static void
_sound_alsa_Exit(void)
{
}

__EXPORT__ extern const SoundOps SoundOps_alsa;

const SoundOps      SoundOps_alsa = {
   _sound_alsa_Init, _sound_alsa_Exit, _sound_alsa_Load,
   _sound_alsa_Destroy, _sound_alsa_Play,
};

#endif /* HAVE_SOUND && USE_SOUND_ALSA */
