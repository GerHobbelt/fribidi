/* FriBidi
 * gen-joining-type-tab.c - generate joining-type.tab.i
 *
 * Author:
 *   Behdad Esfahbod, 2004
 *
 * Copyright (C) 2004 Sharif FarsiWeb, Inc
 * Copyright (C) 2004 Behdad Esfahbod
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library, in a file named COPYING; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA
 * 
 * For licensing issues, contact <fribidi.license@gmail.com>.
 */

#include <common.h>

#include <fribidi-unicode.h>

#include <stdio.h>
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#include "packtab.h"

#define appname "gen-joining-type-tab"
#define outputname "joining-type.tab.i"

static void
die (
  const char *msg
)
{
  fprintf (stderr, appname ": %s\n", msg);
  exit (1);
}

static void
die2 (
  const char *fmt,
  const char *p
)
{
  fprintf (stderr, appname ": ");
  fprintf (stderr, fmt, p);
  fprintf (stderr, "\n");
  exit (1);
}

static void
die3 (
  const char *fmt,
  const char *p,
  const char *q
)
{
  fprintf (stderr, appname ": ");
  fprintf (stderr, fmt, p, q);
  fprintf (stderr, "\n");
  exit (1);
}

static void
die3l (
  const char *fmt,
  unsigned long l,
  const char *p
)
{
  fprintf (stderr, appname ": ");
  fprintf (stderr, fmt, l, p);
  fprintf (stderr, "\n");
  exit (1);
}

enum FriBidiJoiningLinearEnumOffsetOne
{
# define _FRIBIDI_ADD_TYPE(TYPE,SYMBOL) TYPE,
# include <fribidi-joining-types-list.h>
# undef _FRIBIDI_ADD_TYPE
  NUM_TYPES
};

static struct
{
  const char *name;
  int key;
}
type_names[] =
{
# define _FRIBIDI_ADD_TYPE(TYPE,SYMBOL) {STRINGIZE(TYPE), TYPE},
# include <fribidi-joining-types-list.h>
# undef _FRIBIDI_ADD_TYPE
};

#define type_names_count (sizeof (type_names) / sizeof (type_names[0]))

static const char *names[type_names_count];

static char
get_type (
  const char *s
)
{
  unsigned int i;

  for (i = 0; i < type_names_count; i++)
    if (!strcmp (s, type_names[i].name))
      return type_names[i].key;
  die2 ("joining type name `%s' not found", s);
  return -1;
}

static const char *ignored_bidi_types[] = {
  "BN",
  "LRE",
  "RLE",
  "LRO",
  "RLO",
  "PDF",
  "LRI",
  "RLI",
  "FSI",
  "PDI",
  NULL
};

static const char *transparent_general_categories[] = {
  "Mn",
  "Me",
  "Cf",
  NULL
};

static const char *
type_is (
  const char *s,
  const char *type_list[]
)
{
  for (; type_list[0]; type_list++)
    if (!strcmp (s, type_list[0]))
      return type_list[0];
  return NULL;
}

#define table_name "Joi"
#define macro_name "FRIBIDI_GET_JOINING_TYPE"

static signed int table[FRIBIDI_UNICODE_CHARS];
static char buf[4000];
static char tp[sizeof (buf)], tp_gen[sizeof (buf)], tp_bidi[sizeof (buf)];

static void
clear_tab (
  void
)
{
  register FriBidiChar c;

  for (c = 0; c < FRIBIDI_UNICODE_CHARS; c++)
    table[c] = U;
}

static void
init (
  void
)
{
  register int i;

  for (i = 0; i < type_names_count; i++)
    names[i] = 0;
  for (i = type_names_count - 1; i >= 0; i--)
    names[type_names[i].key] = type_names[i].name;

  clear_tab ();
}

static void
read_unicode_data_txt (
  FILE *f
)
{
  unsigned long c, l;

  l = 0;
  while (fgets (buf, sizeof buf, f))
    {
      int i;
      const char *s = buf;

      l++;

      while (*s == ' ')
	s++;

      if (*s == '#' || *s == '\0' || *s == '\n')
	continue;

      i = sscanf (s, "%lx;%*[^;];%[^; ];%*[^;];%[^; ]", &c, tp_gen, tp_bidi);
      if (i != 3 || c >= FRIBIDI_UNICODE_CHARS)
	die3l ("UnicodeData.txt: invalid input at line %ld: %s", l, s);

      if (type_is (tp_bidi, ignored_bidi_types))
	table[c] = G;
      if (type_is (tp_gen, transparent_general_categories))
	table[c] = T;
    }
}

static void
read_arabic_shaping_txt (
  FILE *f
)
{
  unsigned long c, c2, l;

  l = 0;
  while (fgets (buf, sizeof buf, f))
    {
      int i;
      register char typ;
      const char *s = buf;

      l++;

      while (*s == ' ')
	s++;

      if (*s == '#' || *s == '\0' || *s == '\n')
	continue;

      i = sscanf (s, "%lx ; %*[^;]; %[^; ]", &c, tp);
      if (i == 2)
	c2 = c;
      else
	i = sscanf (s, "%lx..%lx ; %*[^;]; %[^; ]", &c, &c2, tp) - 1;

      if (i != 2 || c > c2 || c2 >= FRIBIDI_UNICODE_CHARS)
	die3l ("ArabicShaping.txt: invalid input at line %ld: %s", l, s);

      typ = get_type (tp);
      for (; c <= c2; c++)
	table[c] = typ;
    }
}

static void
read_data (
  const char *data_file_type[],
  const char *data_file_name[]
)
{
  FILE *f;

  for (; data_file_name[0] && data_file_type[0];
       data_file_name++, data_file_type++)
    {
      if (!(f = fopen (data_file_name[0], "rt")))
	die2 ("error: cannot open `%s' for reading", data_file_name[0]);

      if (!strcmp (data_file_type[0], "UnicodeData.txt"))
	read_unicode_data_txt (f);
      else if (!strcmp (data_file_type[0], "ArabicShaping.txt"))
	read_arabic_shaping_txt (f);
      else
	die2 ("error: unknown data-file type %s", data_file_type[0]);

      fclose (f);
    }

}

static void
gen_joining_type_tab (
  int max_depth,
  const char *data_file_type[]
)
{
  printf ("/* " outputname "\n * generated by " appname " (" FRIBIDI_NAME " "
	  FRIBIDI_VERSION ")\n" " * from the files %s, %s of Unicode version "
	  FRIBIDI_UNICODE_VERSION ". */\n\n", data_file_type[0],
	  data_file_type[1]);

  printf ("#define PACKTAB_UINT8 uint8_t\n"
	  "#define PACKTAB_UINT16 uint16_t\n"
	  "#define PACKTAB_UINT32 uint32_t\n\n");

  if (!pack_table
      (table, FRIBIDI_UNICODE_CHARS, 1, U, max_depth, 1, names,
       "unsigned char", table_name, macro_name, stdout))
    die ("error: insufficient memory, decrease max_depth");

  printf ("#undef PACKTAB_UINT8\n"
	  "#undef PACKTAB_UINT16\n" "#undef PACKTAB_UINT32\n\n");

  printf ("/* End of generated " outputname " */\n");
}

#if defined(BUILD_MONOLITHIC)
#define main   fribidi_gen_joining_type_tab_main
#endif

int
main (
  int argc,
  const char **argv
)
{
  const char *data_file_type[] =
    { "UnicodeData.txt", "ArabicShaping.txt", NULL };

  if (argc < 4)
    die3 ("usage:\n  " appname " max-depth /path/to/%s /path/to/%s [junk...]",
	  data_file_type[0], data_file_type[1]);

  {
    int max_depth = atoi (argv[1]);
    const char *data_file_name[] = { NULL, NULL, NULL };
    data_file_name[0] = argv[2];
    data_file_name[1] = argv[3];

    if (max_depth < 2)
      die ("invalid depth");

    init ();
    read_data (data_file_type, data_file_name);
    gen_joining_type_tab (max_depth, data_file_type);
  }

  return 0;
}
