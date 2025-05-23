/* FriBidi
 * gen-brackets-type-tab.c - generate brackets.tab.i
 *
 * Author:
 *   Behdad Esfahbod, 2001, 2002, 2004
 *   Dov Grobgeld 2017
 *
 * Copyright (C) 2004 Sharif FarsiWeb, Inc
 * Copyright (C) 2001,2002,2004 Behdad Esfahbod
 * Copyright (C) 2017 Dov Grobgeld
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

#define appname "gen-brackets-type-tab"
#define outputname "brackets-type.tab.i"

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
die4 (
  const char *fmt,
  unsigned long l,
  unsigned long p,
  unsigned long q
)
{
  fprintf (stderr, appname ": ");
  fprintf (stderr, fmt, l, p, q);
  fprintf (stderr, "\n");
  exit (1);
}

#define table_name "Brt"
#define macro_name "FRIBIDI_GET_BRACKET_TYPE"

static signed int table[FRIBIDI_UNICODE_CHARS];
static char buf[4000];
static signed long max_dist;

static void
init (
  void
)
{
  max_dist = 0;
}

static void
clear_tab (
  void
)
{
  register FriBidiChar c;

  for (c = 0; c < FRIBIDI_UNICODE_CHARS; c++)
    table[c] = 0;
}

static void
init_tab_brackets_type_txt (
  void
)
{
  clear_tab ();
}

static void
read_bidi_brackets_type_txt (
  FILE *f
)
{
  unsigned long l;

  init_tab_brackets_type_txt ();

  l = 0;
  while (fgets (buf, sizeof buf, f))
    {
      unsigned long i, j;
      int k;
      const char *s = buf;
      char open_close;

      l++;

      while (*s == ' ')
	s++;

      if (s[0] == '#' || s[0] == '\0' || s[0] == '\n')
	continue;

      k = sscanf (s, "%lx; %lx; %c", &i, &j, &open_close);
      if (k != 3 || i >= FRIBIDI_UNICODE_CHARS || j >= FRIBIDI_UNICODE_CHARS)
	die4 ("invalid pair in input at line %ld: %04lX, %04lX", l, i, j);
      table[i] = 1 + (0x2 * (open_close=='o'));
    }
}

static void
read_data (
  const char *data_file_type,
  const char *data_file_name
)
{
  FILE *f;

  if (!(f = fopen (data_file_name, "rt")))
    die2 ("error: cannot open `%s' for reading", data_file_name);

  if (!strcmp (data_file_type, "BidiBrackets.txt"))
    read_bidi_brackets_type_txt (f);
  else
    die2 ("error: unknown data-file-type %s", data_file_type);

  fclose (f);
}

static void
gen_brackets_tab (
  int max_depth,
  const char *data_file_type
)
{
  int key_bytes;
  const char *key_type;

  printf ("/* " outputname "\n * generated by " appname " (" FRIBIDI_NAME " "
	  FRIBIDI_VERSION ")\n" " * from the file %s of Unicode version "
	  FRIBIDI_UNICODE_VERSION ". */\n\n", data_file_type);

  printf ("#define PACKTAB_UINT8 uint8_t\n"
	  "#define PACKTAB_UINT16 uint16_t\n"
	  "#define PACKTAB_UINT32 uint32_t\n\n");

  key_bytes = 1;
  key_type = key_bytes == 1 ? "int8_t" : key_bytes == 2 ?
    "int16_t" : "int32_t";

  if (!pack_table
      (table, FRIBIDI_UNICODE_CHARS, key_bytes, 0, max_depth, 1, NULL,
       key_type, table_name, macro_name, stdout))
    die ("error: insufficient memory, decrease max_depth");

  printf ("#undef PACKTAB_UINT8\n"
	  "#undef PACKTAB_UINT16\n" "#undef PACKTAB_UINT32\n\n");

  printf ("/* End of generated " outputname " */\n");
}

#if defined(BUILD_MONOLITHIC)
#define main   fribidi_gen_brackets_type_tab_main
#endif

int
main (
  int argc,
  const char **argv
)
{
  const char *data_file_type = "BidiBrackets.txt";

  if (argc < 3)
    die2 ("usage:\n  " appname " max-depth /path/to/%s [junk...]",
	  data_file_type);

  {
    int max_depth = atoi (argv[1]);
    const char *data_file_name = argv[2];

    if (max_depth < 2)
      die ("invalid depth");

    init ();
    read_data (data_file_type, data_file_name);
    gen_brackets_tab (max_depth, data_file_type);
  }

  return 0;
}
