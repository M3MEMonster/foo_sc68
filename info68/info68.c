/*
 * @file    info68.c
 * @brief   program to retrieve information from sc68 files
 * @author  http://sourceforge.net/users/benjihan
 *
 * Copyright (c) 1998-2015 Benjamin Gerard
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* file68 includes */
#include <sc68/file68.h>
#include <sc68/file68_err.h>
#include <sc68/file68_str.h>
#include <sc68/file68_msg.h>
#include <sc68/file68_uri.h>
#include <sc68/file68_opt.h>

/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#define BUILD_DATE  __DATE__

static int error(const char *format, ...)
{
  va_list list;
  va_start(list, format);
  error68_va(format, list);
  va_end(list);
  return -1;
}

/* static void debug(const char *format, ...) */
/* { */
/* #ifdef DEBUG */
/*   va_list list; */
/*   va_start(list, format); */
/*   vfprintf(stderr, format, list); */
/*   va_end(list); */
/* #endif */
/* } */

static int file_error(const char *filename)
{
  error ("info68: bad or missing sc68 input file \"%s\"\n", filename);
  return 2;
}

static int tracklist_error(const char *command)
{
  error("info68: bad track-list \"%s\"\n", command);
  return 4;
}

static void print_option(void * data,
                         const char * option,
                         const char * envvar,
                         const char * values,
                         const char * desc)
{
  fprintf(data,"  %s", option);
  if (values && *values)
    fprintf(data,"=%s", values);
  fprintf(data,"\n"
          "    %-16s %c%s\n", envvar,
          (desc[0]>='a' && desc[0]<='z') ? desc[0]-'a'+'A' : desc[0],
          desc+1);
}

static int display_help(int more)
{
  puts
    ("Usage: info68 [OPTION] [--] <URI> track-list format ...\n"
     "\n"
     "Get and format information on sc68 files.\n");

  puts
    ("Options:\n"
     "\n"
     "  --                  Break option parsing\n"
     "  -h --help           Display this message and exit\n"
     "  -V --version        Display version and exit\n"
     "  -o --output=<URI>   Change output to file (- is stdout)\n"
     "  -A --all            Display all information and tags\n");

  if (more) {
    option68_help(stdout,print_option);
    puts("");
  }

  puts
    ("Track-list:\n"
     "\n"
     "  syntax:\n"
     "\n"
     "    -DIGIT[-DIGIT][,DIGIT[-DIGIT]...]\n"
     "\n"
     "    track-list executes the next format string for all listed tracks\n"
     "    one after another  in the given order.  First track is number 1.\n"
     "    0 is replaced by the last track number (number of track).\n"
     "\n"
     "  format:\n"
     "\n"
     "    format is a string with special commands sequence. Each of these\n"
     "    commands  start with  a percent '%' char  followed by  a command\n"
     "    identifer.\n");
  puts
    ("  disk-commands:\n"
     "\n"
     "    `%#'         number of tracks\n"
     "    `%?'         default track\n"
     "    `%N'         disk name\n"
     "    `%A'         default track author name\n"
     "    `%C'         default track composer name\n"
     "    `%P'         default track ripper name\n"
     "    `%V'         default track converter name\n"
     "    `%T'         disk time in sec\n"
     "    `%Y'         formated disk time. Format \"TT MM:SS\"\n"
     "    `%H'         all tracks ORed hardware flags (see %h)\n"
     "    `%F'         file format (sc68 or sndh)\n"
     "    `%~'         file hash code (unic-id)\n"
     "    `%U'         input URI\n");
  puts
    ("  track-commands:\n"
     "\n"
     "    `%&'         track number\n"
     "    `%n'         track name\n"
     "    `%a'         author name\n"
     "    `%c'         composer name\n"
     "    `%p'         ripper name\n"
     "    `%v'         converter name\n"
     "    `%r'         replay name\n"
     "    `%t'         time in sec\n"
     "    `%y'         formated time. Format \"TT MM:SS\"\n"
     "    `%f'         replay frequency (frame length)\n"
     "    `%m'         duration in frames\n"
     "    `%@'         load address (in hexdecimal)\n"
     "    `%h'         hardware flags [YSAT] uppercase means activated\n"
     "                 Y:YM-2149 S:STE A:Amiga T:Timers\n");

  puts
    ("  misc-commands:\n"
     "\n"
     "    `%%'         display %\n"
     "    `%0'         display null char\n"
     "    `%L'         display a newline character\n");

  puts
    ("  tag-commands:\n"
     "\n"
     "    `%{tag}'     display named-tag.\n"
     "                 Use uppercase first letter to address disk tag\n"
     "                 and lowercase to address current track tag\n");

  puts
    ("URI:\n"
     "\n"
     "  $ - or\n"
     "  $ stdin://           read file fron standard input.\n"
     "  $ path or\n"
     "  $ file://path or\n"
     "  $ local://path       local file.\n"
     "  $ null://            null file.\n"
     "  $ http://path or\n"
     "  $ ftp://path or\n"
     "  $ ...                load file via given protocol (see curl)\n"
     "  $ sc68://author/hw/title[/track:loop\n"
     "  Access sc68 music database. The music file is first searched in\n"
     "  local music path and if not found in remote music path.\n");

  puts
    ("Examples:\n"
     "\n"
     "  info68 'sc68://music/Jochen Hippel (Mad Max)/0/Wings Of Death/' '-1,5,4-6' '> %&/%# %N - %a - %n%L'\n"
     "\n"
     "  > 1/10 Wings Of Death - Jochen Hippel (Mad Max) - Level #1\n"
     "  > 5/10 Wings Of Death - Jochen Hippel (Mad Max) - Level #5\n"
     "  > 4/10 Wings Of Death - Jochen Hippel (Mad Max) - Level #4\n"
     "  > 5/10 Wings Of Death - Jochen Hippel (Mad Max) - Level #5\n"
     "  > 6/10 Wings Of Death - Jochen Hippel (Mad Max) - Level #6\n");

  puts
    ("Copyright (c) 1998-2015 Benjamin Gerard.\n"
     "\n"
     "Visit <" PACKAGE_URL ">\n"
     "Report bugs to <" PACKAGE_BUGREPORT ">");

  return 1;
}

/* Display version number. */
static int display_version(void)
{
  puts
    (PACKAGE_STRING "\n"
     "\n"
     "Copyright (c) 1998-2015 Benjamin Gerard.\n"
     "License GPLv3+ or later <http://gnu.org/licenses/gpl.html>\n"
     "This is free software: you are free to change and redistribute it.\n"
     "There is NO WARRANTY, to the extent permitted by law.\n"
     "\n"
     "Written by Benjamin Gerard"
      );
  return 0;
}

static const char * HWflags(const hwflags68_t f)
{
  static const char hex[] = "0123456789ABCDEF";
  static char flags[] = "YSA0";
  flags[0] = "yY"[!!(f & SC68_PSG)];
  flags[1] = "sS"[!!(f & (SC68_DMA|SC68_LMC))];
  flags[2] = "aA"[!!(f & SC68_AGA)];
  flags[3] = (f & SC68_XTD) ? hex[15 & (f>>SC68_MFP_BIT)] : '.';
  return flags;
}

static const char * Xflags(const hwflags68_t f)
{
  static char flags[32];
  static const struct {
    int letter, mask;
  } xf[] = {
    { 'y', SC68_PSG },
    { 'e', SC68_DMA },
    { 'a', SC68_MFP_TA },
    { 'b', SC68_MFP_TB },
    { 'c', SC68_MFP_TC },
    { 'd', SC68_MFP_TD },
    { 'p', SC68_AGA },
    { 'l', SC68_LMC },
    { 's', SC68_DSP },
    { 't', SC68_BLT },
    { 'h', SC68_HBL },
    { 'x', 0 },                        /* reserved for SFX */
    { 'g', 0 },                        /* reserved for digital only */
    { 'j', 0 },                        /* reserved for jingles */
    { 0,0 }
  };

  int i,j;
  flags[0] = '~';
  for (i=0, j=1; xf[i].letter; ++i)
    if (f & xf[i].mask)
      flags[j++] = xf[i].letter;
  flags[j] = 0;
  return flags;
}

static int fmt_0 = 0;                   /* prefix zero   */
static int fmt_l = 0;                   /* format length */
static char fmt_b[2048];                /* format temp buffer */
#define fmt_max (sizeof(fmt_b)-1)


static void PutC(vfs68_t *out, const int c)
{
  vfs68_putc(out, c);
}

static void PutS(vfs68_t *out, const char *s)
{
  if (s) {
    vfs68_puts(out, s);
    if (fmt_l) {
      int len = strlen(s);
      while (len++ < fmt_l)
        vfs68_putc(out, ' ');
    }
  }
}

static void PutI(vfs68_t *out, int v)
{
  snprintf(fmt_b, fmt_max, fmt_0 ? "%0*d" : "%-*d" , fmt_l, v);
  fmt_b[fmt_max] = 0;
  PutS(out, fmt_b);
}

static void PutX(vfs68_t *out, unsigned int v)
{
  snprintf(fmt_b, fmt_max, fmt_0 ? "%0*x" : "%-*x" , fmt_l, v);
  fmt_b[fmt_max] = 0;
  PutS(out, fmt_b);
}

static void PutX32(vfs68_t *out, unsigned int v)
{
  int sav_0 = fmt_0, sav_l = fmt_l;
  if (!fmt_0 && !fmt_l) {
    fmt_0 = 1;
    fmt_l = 8;
  }
  snprintf(fmt_b, fmt_max, fmt_0 ? "%0*x" : "%-*x" , fmt_l, v);
  fmt_b[fmt_max] = 0;
  PutS(out, fmt_b);
  fmt_0 = sav_0;
  fmt_l = sav_l;
}

static int ReadTrackNumber(char **ps, int max)
{
  int n;
  if (!isdigit((int)**ps)) {
    return -1;
  }
  n = strtol(*ps, ps, 10);
  if (n<0 || n>max) {
    return -1;
  } else if (n==0) {
    n = max;
  }
  return n - 1;
}

/* Process track-list
 * return  0  on end
 * return <0  on error
 * return  1  on continue
 */
static int ReadTrackList(char **trackList, int max, int *from, int *to)
{
  int fromTrack, toTrack;
  char *t = *trackList;

  if (t) {
    /* Skip comma ',' */
    while(*t == ',') {
      ++t;
    }
  }

  /* Done with this list. */
  if (!t || !*t) {
    *trackList = 0;
    return 0;
  }

  *trackList = t;
  fromTrack = ReadTrackNumber(trackList, max);
  if (fromTrack < 0) {
    return -1;
  }

  switch(**trackList) {
  case ',': case 0:
    toTrack = fromTrack;
    break;
  case '-':
    (*trackList)++;
    toTrack = ReadTrackNumber(trackList, max);
    if (toTrack < 0) {
      return -2;
    }
    break;
  default:
    return -1;
  }

  *from = fromTrack;
  *to   = toTrack;

  return 1;
}

static char * get_tag(const disk68_t * dsk, int trk, const char * key)
{
  return file68_tag(dsk, trk, key);
}

int main(int argc, char ** argv)
{
  int  i, j, opt_all = 0, code = 127;
  /*   hwflags68_t diskHW; */
  int has_time, has_loop;
  const disk68_t *d = 0;
  int curTrack, toTrack;
  char *trackList;
  vfs68_t * out = 0;
  const char * inname  = 0;
  const char * outname = "stdout:info68";

  argv[0] = "info68";
  argc = file68_init(argc, argv);
  if (argc < 0) {
    error("info68: file68 initialisation failed.\n");
    goto exit;
  }

  if (argc < 2) {
    error("info68: missing argument. Try --help.\n");
    code = 1;
    goto finish;
  }

  /* Scan options ...  */
  for (i=1; i<argc; ++i) {
    if (!strcmp(argv[i],"--")) {
      ++i;
      break;
    } else if (!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h")) {
      return display_help(1);
    } else if (!strcmp(argv[i],"--version") || !strcmp(argv[i],"-V")) {
      return display_version();
    } else if (!strcmp(argv[i],"--all") || !strcmp(argv[i],"-A")) {
      opt_all = 1;
    } else if ((!strcmp(argv[i], "-o") || !strcmp(argv[i],"--output"))) {
      if (++i >= argc) {
        error("info69: option `%s' missing argument.\n", argv[i-1]);
        code = 2;
        goto finish;
      }
      outname = argv[i];
    } else if (argv[i] == strstr(argv[i],"--output=")) {
      outname = argv[i]+9;
    } else {
      break;
    }
  }

  if (i >= argc) {
    error ("info68: missing input file.\n");
    code = 3;
    goto finish;
  }
  inname = argv[i];

  out = uri68_vfs(outname, 2, 0);
  if (vfs68_open(out)) {
    error ("info68: error opening output (%s).\n",
           out ? vfs68_filename(out) : outname);
    code = 4;
    goto finish;
  }

  /* Load input file */
  d = file68_load_uri(inname);
  if (!d) {
    code = 5;
    file_error(inname);
    goto finish;
  }

  /* determine if disk has time and loop for all its tracks */
  for (has_loop = has_time = j = 0; j<d->nb_mus; ++j) {
    has_time += d->mus[j].has.time;
    has_loop += d->mus[j].has.loop;
  }
  has_time = has_time == d->nb_mus;
  has_loop = has_loop == d->nb_mus;

  if (opt_all) {
    int i, j;
    const char * key, * val;

    PutS(out,"file: ");     PutS(out,inname);     PutC(out,'\n');
    PutS(out,"hash: ");     PutX32(out,d->hash);  PutC(out,'\n');
    PutS(out,"tracks: ");   PutI(out,d->nb_mus);  PutC(out,'\n');
    PutS(out,"default: ");  PutI(out,d->def_mus); PutC(out,'\n');
    if (has_time) {
      PutS(out,"time-ms: ");  PutI(out,d->time_ms); PutC(out,'\n');
    }
    PutS(out,"hardware: "); PutS(out,HWflags(d->hwflags)); PutC(out,'\n');
    for (j=0; !file68_tag_enum(d, 0, j, &key, &val); ++j) {
      PutS(out,key); PutS(out,": "); PutS(out,val); PutC(out,'\n');
    }

    for (i=1; i<=d->nb_mus; ++i) {
      const music68_t *m = d->mus+(i-1);
      PutS(out,"track: ");    PutI(out,i);           PutC(out,'\n');
      // PutS(out,"remap: ");    PutI(out,m->track);    PutC(out,'\n');
      if (m->has.time) {
        PutS(out,"time-ms: ");  PutI(out,m->first_ms); PutC(out,'\n');
        PutS(out,"time-fr: ");  PutI(out,m->first_fr); PutC(out,'\n');
      }
      if (m->has.loop) {
        PutS(out,"loops: ");    PutI(out,m->loops);    PutC(out,'\n');
        PutS(out,"loop-ms: ");  PutI(out,m->loops_ms); PutC(out,'\n');
        PutS(out,"loop-fr: ");  PutI(out,m->loops_fr); PutC(out,'\n');
      }

      if (m->replay) {
        PutS(out,"replay: ");
        PutS(out, m->replay);
        PutC(out,'\n');
      }

      PutS(out,"rate: "); PutI(out,m->frq); PutC(out,'\n');

      PutS(out,"address: ");
      if (m->has.pic)
        PutS(out,"PIC");
      else
        PutX(out,m->a0);
      PutC(out,'\n');

      PutS(out,"hardware: "); PutS(out,HWflags(m->hwflags)); PutC(out,'\n');
      if (m->hwflags & SC68_XTD) {
        PutS(out,"x-hardware: ");
        PutS(out,Xflags(m->hwflags)); PutC(out,'\n');
      }

      for (j=0; !file68_tag_enum(d, i, j, &key, &val); ++j) {
        PutS(out,key); PutS(out,": "); PutS(out,val); PutC(out,'\n');
      }
    }
    code = 0;
    goto finish;
  }

  /* Setup variable */
  trackList = 0;
  toTrack = curTrack = d->def_mus;

  /* Main loop */
  for (++i; i<argc; ++i) {
    if (!trackList && argv[i][0] == '-' && isdigit((int)argv[i][1])) {
      int res;
      trackList = argv[i]+1;
      res = ReadTrackList(&trackList, d->nb_mus, &curTrack , &toTrack);
      if (res < 0) {
        return tracklist_error(trackList);
      } else if (!res) {
        /* This can't be coz we check that almost one digit was there above */
        error("info68: %s(%d) : Internal bug error;"
              " program should not reach this point\n", __FILE__, __LINE__);
        code = 66;
        goto finish;
      }
      continue;
    }

    while (curTrack <= toTrack) {
      int esc = 0, c;
      char * s;
      const music68_t *m = d->mus + curTrack;

      /* Parse format string for this track */
      for (s=argv[i], esc=0; (c = *s, c); ++s) {
        if (!esc) {
          /* Not escaped */
          if(esc = (c=='%'), !esc) {
            PutC(out,c);
          } else {
            int d;
            fmt_l = 0;
            fmt_0 = 0;
            if (d = s[1], (d >= '0' && d <= '9')) {
              fmt_0 = ( d == '0' );
              do {
                ++s;
                fmt_l = fmt_l*10 + d - '0';
                d = s[1];
              } while (d >= '0' && d <= '9');
            }
          }
        } else {
          /* Escaped */
          esc = 0;
          switch (c) {

            /* SPECIAL commands */
          case 'L':
            PutC(out,'\n');
            break;
          case '%':
            PutC(out,'%');
            break;
          case '0':
            PutC(out,'\0');
            break;

            /* DISK commands */
          case 'U':
            PutS(out,inname);
            break;
          case '~':
            PutX32(out,d->hash);
            break;
          case '#':
            PutI(out,d->nb_mus);
            break;
          case '?':
            PutI(out,d->def_mus+1);
            break;
          case 'N':
            PutS(out,d->tags.tag.title.val);
            break;
          case 'A':
            PutS(out,d->tags.tag.artist.val);
            break;
          case 'C':
            PutS(out,file68_tag_get(d,0,TAG68_COMPOSER));
            break;
          case 'P':
            PutS(out,file68_tag_get(d,0,TAG68_RIPPER));
            break;
          case 'V':
            PutS(out,file68_tag_get(d,0,TAG68_CONVERTER));
            break;
          case 'T':
            PutI(out, has_time*d->time_ms/1000u);
            break;
          case 'Y':
            PutS(out,strtime68(0, d->nb_mus, has_time*d->time_ms/1000u));
            break;
          case 'H':
            PutS(out,HWflags(d->hwflags));
            break;
          case 'F':
            PutS(out,file68_tag_get(d,0,TAG68_FORMAT));
            break;

            /* TRACK Commands */
          case '&':
            PutI(out,curTrack+1);
            break;
          case 'n':
            PutS(out,m->tags.tag.title.val);
            break;
          case 'a':
            PutS(out,m->tags.tag.artist.val);
            break;
          case 'c':
            PutS(out,file68_tag_get(d,curTrack+1,TAG68_COMPOSER));
            break;
          case 'p':
            PutS(out,file68_tag_get(d,curTrack+1,TAG68_RIPPER));
            break;
          case 'v':
            PutS(out,file68_tag_get(d,curTrack+1,TAG68_CONVERTER));
            break;
          case 'r':
            PutS(out, m->replay ? m->replay : "built-in");
            break;
          case 't':
            PutI(out,m->has.time*m->first_ms/1000u);
            break;
          case 'y':
            PutS(out,strtime68(0, curTrack+1, m->has.time*m->first_ms/1000u));
            break;
          case 'm':
            PutI(out,m->has.time*m->first_fr);
            break;
          case 'h':
            PutS(out, HWflags(m->hwflags));
            break;
          case 'f':
            PutI(out,m->frq);
            break;
          case '@':
            PutX(out,m->a0);
            break;
          case '{': {
            char * key = s+1, * end, * val;
            if (end = strchr(key,'}'), end) {
              int trk = isupper((int)*key) ? 0 : curTrack+1;
              *end = 0;
              val = get_tag(d, trk, key);
              if (val) PutS(out,val);
              free(val);
              *end = '}';
              s = end;
              break;
            }
          }
            /* default: */
            /* PutC(out,'%'); */
            /* PutC(out,c); */
          } /* switch */
        } /* if else */
      } /* for */

        /* Get next track. */
      if (++curTrack > toTrack) {
        int res = ReadTrackList(&trackList, d->nb_mus, &curTrack , &toTrack);
        if (res < 0) {
          return tracklist_error(trackList);
        } else if (!res) {
          toTrack = curTrack = d->def_mus;
          break;
        }
      }
    } /* while track */
  } /* while arg */

  code = 0;
finish:
  vfs68_destroy(out);
  file68_free(d);
  file68_shutdown();

exit:
  return code;
}
