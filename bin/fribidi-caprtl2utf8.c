#include <stdio.h>
#include <string.h>
#include <fribidi.h>

static char buf[BUFSIZ];
static FriBidiChar ubuf[BUFSIZ];

#if defined(BUILD_MONOLITHIC)
#define main   fribidi_caprtl2utf8_main
#endif

int
main (
  void
)
{
  FriBidiCharSet caprtl = fribidi_parse_charset ("CapRTL");
  FriBidiCharSet utf8 = fribidi_parse_charset ("UTF-8");

  while (fgets (buf, sizeof buf, stdin))
    {
      char eol[5];

      FriBidiStrIndex len = strlen (buf);
      while (len && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
	len--;
      strncpy (buf, buf + len, sizeof eol);
      buf[len] = 0;

      len = fribidi_charset_to_unicode (caprtl, buf, len, ubuf);
      fprintf (stderr, "%d\n", len);
      len = fribidi_unicode_to_charset (utf8, ubuf, len, buf);

      buf[len] = 0;
      printf ("%s%s", buf, eol);
    }

  return 0;
}
