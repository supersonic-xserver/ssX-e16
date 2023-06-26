/*
 * Copyright (C) 2008-2023 Kim Woelders
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

/*
 * Trim leading and trailing whitespace
 */
char               *
Estrtrim(char *s)
{
   int                 l;

   while (isspace(*s))
      s++;
   if (!*s)
      return s;

   l = strlen(s);
   while (isspace(s[l - 1]))
      l--;
   s[l] = '\0';

   return s;
}

/*
 * Trim comments and trailing whitespace
 */
char               *
Estrtrim2(char *s)
{
   int                 len, len2, ch, quote;

   while (isspace(*s) == ' ')
      s++;

   quote = '\0';
   for (len = len2 = 0;; len++)
     {
	ch = s[len];
	switch (ch)
	  {
	  default:
	     break;
	  case '\0':
	  case '\n':
	  case '\r':
	     goto got_len;
	  case '\'':
	  case '"':
	     quote = (ch == quote) ? '\0' : ch;
	     break;
	  case '#':
	     if (quote)
		break;
	     goto got_len;
	  case ' ':
	  case '\t':
	     if (quote)
		break;
	     continue;
	  }
	len2 = len + 1;
     }
 got_len:
   s[len2] = '\0';

   return s;
}

#if 0				/* Unused */
char              **
StrlistDup(char **lst, int num)
{
   char              **ss;
   int                 i;

   if (!lst || num <= 0)
      return NULL;

   ss = EMALLOC(char *, num + 1);
   for (i = 0; i < num; i++)
      ss[i] = Estrdup(lst[i]);
   ss[i] = NULL;

   return ss;
}
#endif

void
StrlistFree(char **lst, int num)
{
   if (!lst)
      return;
   while (num--)
      Efree(lst[num]);
   Efree(lst);
}

#if 0				/* FIXME - Remove? */
char               *
StrlistJoin(char **lst, int num)
{
   int                 i, size;
   char               *s;

   if (!lst || num <= 0)
      return NULL;

   s = NULL;

   size = strlen(lst[0]) + 1;
   s = EMALLOC(char, size);
   strcpy(s, lst[0]);
   for (i = 1; i < num; i++)
     {
	size += strlen(lst[i]) + 1;
	s = EREALLOC(char, s, size);

	strcat(s, " ");
	strcat(s, lst[i]);
     }

   return s;
}
#endif

char               *
StrlistEncodeEscaped(char *buf, int len, char **lst, int num)
{
   int                 i, j, ch;
   char               *s, *p;

   if (!lst || num <= 0)
      return NULL;

   j = 0;
   s = buf;
   p = lst[0];
   for (i = 0; i < len - 2; i++)
     {
	if (!p)			/* A string list should not contain NULL items */
	   break;

	ch = *p++;
	switch (ch)
	  {
	  default:
	     *s++ = ch;
	     break;
	  case '\0':
	     if (++j >= num)
		goto done;
	     p = lst[j];
	     if (!p || !p[0])
		goto done;
	     *s++ = ' ';
	     break;
	  case ' ':
	     *s++ = '\\';
	     *s++ = ' ';
	     i++;
	     break;
	  }
     }

 done:
   *s = '\0';
   return buf;
}

static int
_StrlistDecodeArgLen(const char *str, const char **pnext)
{
   int                 len, ch, delim;
   const char         *s;

   len = 0;
   delim = '\0';
   for (s = str;; s++)
     {
	ch = *s;
	switch (ch)
	  {
	  default:
	     break;

	  case '\0':
	     len = s - str;
	     goto done;

	  case ' ':
	     if (delim)
		break;
	     if (s > str && s[-1] == '\\')
		break;
	     len = s - str;
	     s += 1;
	     goto done;

	  case '\'':
	  case '"':
	     if (ch == delim)
		delim = '\0';
	     else
		delim = ch;
	     break;
	  }
     }

 done:
   *pnext = s;
   return len;
}

static char        *
_StrlistDecodeArgParse(const char *str, int len)
{
   char               *buf, *p;
   int                 i, ch, ch_last, delim;

   buf = EMALLOC(char, len + 1);

   p = buf;
   ch_last = '\0';
   delim = '\0';
   for (i = 0; i < len; i++)
     {
	ch = str[i];
	switch (ch)
	  {
	  default:
	     if (ch_last == '\\' && ch != ' ')
		*p++ = '\\';	/* TBD!!! */
	     *p++ = ch;
	     break;

	  case '\\':
	     break;

	  case '\'':
	  case '"':
	     if (!delim)
		delim = ch;	/* Quote start */
	     else if (delim == ch)
		delim = '\0';	/* Quote end */
	     else
		*p++ = ch;
	     break;
	  }
	ch_last = ch;
     }

   *p++ = '\0';
   return buf;
}

char              **
StrlistDecodeEscaped(const char *str, int *pnum)
{
   int                 num, len;
   const char         *s, *p;
   char              **lst;

   if (!str)
      return NULL;

   lst = NULL;
   num = 0;
   s = str;
   for (;; s = p)
     {
	/* Find next token */
	while (*s == ' ')
	   s++;
	if (*s == '\0')
	   break;

	/* Find token extents (including quoting etc.) */
	len = _StrlistDecodeArgLen(s, &p);

	/* Add token */
	lst = EREALLOC(char *, lst, num + 1);
	lst[num++] = _StrlistDecodeArgParse(s, len);
     }

   /* Append NULL item */
   lst = EREALLOC(char *, lst, num + 1);

   lst[num] = NULL;

   *pnum = num;
   return lst;
}

char              **
StrlistFromString(const char *str, int delim, int *num)
{
   const char         *s, *p;
   char              **lst;
   int                 n, len;

   lst = NULL;
   n = 0;
   for (s = str; s; s = p)
     {
	p = strchr(s, delim);
	if (p)
	  {
	     len = p - s;
	     p++;
	  }
	else
	  {
	     len = strlen(s);
	  }
	if (len <= 0)
	   continue;

	lst = EREALLOC(char *, lst, n + 2);

	lst[n++] = Estrndup(s, len);
     }

   if (lst)
      lst[n] = NULL;
   *num = n;
   return lst;
}

static int
_qsort_strcmp(const void *s1, const void *s2)
{
   return strcmp(*(const char **)s1, *(const char **)s2);
}

void
StrlistSort(char **lst, int len)
{
   qsort(lst, (unsigned int)len, sizeof(char *), _qsort_strcmp);
}
