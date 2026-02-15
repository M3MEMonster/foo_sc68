/*
 * Demux sc68 'QUAR'tet files
 *
 * Time-stamp: <2014-07-03 07:11:32 ben>
 * Init-stamp: <2013-09-13 21:01:17 ben>
 *
 * Copyright (c) 2013-2015 Ben(jamin) Gerard
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static char prg[] = "unquar";

#define THROW(N) do { ret = N; goto bye; } while(1)

static char fname[2048];

static unsigned lpeek(char * b)
{
  return
    ( (b[0] & 255) << 24 ) |
    ( (b[1] & 255) << 16 ) |
    ( (b[2] & 255) <<  8 ) |
    ( (b[3] & 255) <<  0 ) ;
}

struct song_s {
  unsigned pos;
  unsigned off;
  unsigned len;
} song[100];

static int by_pos(const void * a, const void * b) {
  return
    ( (struct song_s *) a ) -> pos - ( (struct song_s *) b ) -> pos;
}

static int by_off(const void * a, const void * b) {
  return
    ( (struct song_s *) a ) -> off - ( (struct song_s *) b ) -> off;
}

int main(int argc, char ** argv)
{
  int ret = 2, i;
  FILE * inp = 0;
  char * buf = 0, * ext, * sla, * iname = 0;
  fpos_t len;
  unsigned offv, songs;
  int verb=0, usage=0;

  argv[0] = prg;

  /* Half lazy option parsing */
  for (i=1; i<argc; ++i) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == '-') {
        if (argv[i][2] == 0) {
          /* -- */
          ++i; break;
        }
        else if (!strcmp(argv[i],"--help") ||
                 !strcmp(argv[i],"--usage"))
          usage=1;
        else if (!strcmp(argv[i],"--verbose") ||
                 !strcmp(argv[i],"-v"))
          verb++;
        else if (!strcmp(argv[i],"--quiet") ||
                 !strcmp(argv[i],"--silent"))
          verb--;
        else {
          fprintf(stderr,"%s: invalid option -- `%s'\n", prg, argv[i]);
          exit(1);
        }
      } else {
        int j;
        for (j=1; argv[i][j]; ++j)
          switch (argv[i][j]) {
          case 'q': verb--; break;
          case 'v': verb++; break;
          case 'h': usage=1; break;
          default:
            fprintf(stderr,"%s: invalid option -- `-%c'\n", prg, argv[i][j]);
            exit(1);
          }
      }
    }
    else if (!iname)
      iname = argv[i];
    else {
      fprintf(stderr,"%s: too many argument -- `%s'\n", prg, argv[i]);
      exit(1);
    }
  }
  if (i <argc && !iname)
    iname = argv[i++];

  if (usage || !iname) {
    printf("Usage: [-v|-q] %s <FILE.quar>\n", argv[0]);
    return usage ? 0 : 1;
  }

  if (i != argc) {
    fprintf(stderr,"%s: too many argument -- `%s'\n", prg, argv[i]);
    exit(1);
  }

  inp = fopen(iname,"rb");
  if (!inp) THROW(2);

  fseek(inp,0L,SEEK_END);
  len = ftell(inp);
  fseek(inp,0L,SEEK_SET);

  if (len < 1024)
    THROW(3);

  buf = malloc(len);
  if (!buf)
    THROW(4);

  if (len != fread(buf,1,len,inp))
    THROW(5);

  fclose(inp); inp = 0;
  if (verb >= 1)
    printf("Loaded '%s' (%u bytes)\n", iname, (unsigned)len);

  if (verb >= 3)
    printf("4cc: %c%c%c%c\n",buf[0],buf[1],buf[2],buf[3]);

  if (memcmp(buf,"QUAR",4))
    THROW(6);

  songs = lpeek(buf+8);
  /* Song #0 is actually the voice set. */
  song[0].pos  = 0;
  song[0].off  = lpeek(buf+4);
  for (i=1; i<=songs; ++i) {
    song[i].pos  = i;
    song[i].off  = lpeek(buf+8+i*4);
  }
  /* Order the song by offset. It should be most of the time but it's
   * not mandatory so it's much more safer this way.
   */
  qsort(song, songs+1, sizeof(*song), by_off);
  if (verb >= 3)
    printf("%-2s %-7s %-7s\n", "##", "OFFSET", "LENGTH");

  for (i=0; i<=songs; ++i) {
    song[i].len  = (i==songs ? len : song[i+1].off) - song[i].off;
    if (verb >= 3)
      printf("%02d %07u %07u\n", song[i].pos, song[i].off, song[i].len);
  }
  /* Now that length has been calculated just re-order the track in
   * the original order.
   */
  qsort(song, songs+1, sizeof(*song), by_pos);
  if (verb >= 2)
    printf("%-2s %-7s %-7s\n", "##", "OFFSET", "LENGTH");
  for (i=0; i<=songs; ++i) {
    song[i].len  = (i==songs ? len : song[i+1].off ) - song[i].off;
    if (verb >= 2)
      printf("%02d %07u %07u\n", song[i].pos, song[i].off, song[i].len);
  }

  strcpy(fname,iname);
  sla = strrchr(fname,'/');
  ext = strrchr(fname,'.');
  if (ext <= sla)
    ext = fname+strlen(fname);

  for (i=0; i<=songs; ++i) {
    if (!i)
      strcpy(ext,".set");
    else if (songs > 1)
      sprintf(ext,"-%0*d.4v", 1 + (songs >= 10) + (songs >= 100), i);
    else
      strcpy(ext,".4v");

    inp = fopen(fname,"wb");
    if (!inp)
      THROW(100+i);
    fwrite(buf+song[i].off,1,song[i].len,inp);
    if (verb >= 0)
      printf("saved '%s' (%d bytes)\n", fname, song[i].len);
    fclose(inp); inp = 0;
  }
  ret = 0;

 bye:
    free(buf);
    if (inp) fclose(inp);

    if (errno)
      perror(iname);
    else if (ret)
      fprintf(stderr,"%s: error %d\n",argv[0],ret);
    return ret;
}
