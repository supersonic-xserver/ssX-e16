/*
 * Copyright (C) 2008-2022 Kim Woelders
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

#include <ctype.h>

#include "util.h"

#ifndef HAVE_STRCASECMP
int
Estrcasecmp(const char *s1, const char *s2)
{
   char                ch1, ch2;

   for (;;)
     {
	ch1 = toupper(*s1++);
	ch2 = toupper(*s2++);
	if (ch1 == '\0' || ch1 != ch2)
	   break;
     }

   return ch1 - ch2;
}
#endif

#ifndef HAVE_STRCASESTR
const char         *
Estrcasestr(const char *haystack, const char *needle)
{
   const char         *s1, *s2;
   char                ch1, ch2;

   for (;; haystack++)
     {
	s1 = haystack;
	s2 = needle;
	if (*s1 == '\0')
	   break;
	for (;;)
	  {
	     ch1 = toupper(*s1++);
	     ch2 = toupper(*s2++);
	     if (ch2 == '\0')
		return haystack;
	     if (ch1 == '\0' || ch1 != ch2)
		break;
	  }
     }
   return NULL;
}
#endif

/*
 * Substitute $ENV_VAR and ${ENV_VAR} with the environment variable
 * from str into bptr[blen].
 */
void
EnvSubst(const char *str, char *bptr, unsigned int blen)
{
   char                env[128];
   int                 nw, len;
   const char         *si, *p1, *p2;

   for (si = str, nw = 0;; si = p2)
     {
	p1 = strchr(si, '$');
	if (!p1)
	  {
	     snprintf(bptr + nw, blen - nw, "%s", si);
	     break;
	  }
	len = p1 - si;
	nw += snprintf(bptr + nw, blen - nw, "%.*s", len, si);
	p1 += 1;
	p2 = *p1 == '{' ? p1 + 1 : p1;
	/* $ENV_VAR - Name is validted */
	for (; *p2 != '\0'; p2++)
	  {
	     if (!(isalnum(*p2) || *p2 == '_'))
		break;
	  }
	len = p2 - p1;
	if (*p1 == '{')
	  {
	     if (*p2 != '}')
	       {
		  p2 = p1;
		  continue;
	       }
	     p1 += 1;
	     p2 += 1;
	     len -= 1;
	  }
	if (len <= 0)
	   continue;
	snprintf(env, sizeof(env), "%.*s", len, p1);
	p1 = getenv(env);
	if (p1)
	   nw += snprintf(bptr + nw, blen - nw, "%s", p1);
     }
}
