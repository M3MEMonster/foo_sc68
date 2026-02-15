/*
 * @file    mksc68_snd.c
 * @brief   sndh creation.
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

/* generated config include */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mksc68_snd.h"
#include "mksc68_msg.h"
#include "mksc68_str.h"

#include <sc68/file68.h>
#include <sc68/file68_tag.h>
#include <sc68/file68_str.h>
#include <sc68/file68_uri.h>
#include <sc68/file68_vfs.h>
#include <sc68/sc68.h>
#ifdef USE_UNICE68
#include <unice68.h>
#endif

/* #include <assert.h> */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <ctype.h>


union sndh_boot {
  int off[4];
  struct sym {
    int init, kill, play, data;
  } sym;
};
typedef union sndh_boot sndh_boot_t;

struct sndh_header {
  sndh_boot_t boot;
  int hlen;    /* header length */
  int dlen;    /* data length */
  int ntune;   /* number of sub tunes */
  int dtune;   /* default tune */
  int rate;    /* sndh only allow same replay rate for all subtunes */
  int timer;   /* one of 'V', 'A', 'B', 'C', 'D' */
};
typedef struct sndh_header sndh_header_t;

/* sndh boot field name */
static const char what[4][5] = { "init", "kill", "play", "data" };

/* 68k instruction type */
enum {
  INST_NOP,
  INST_RTS,
  INST_JMP,
  INST_ERR = -1
};

/* Even alignment */
static int w_E(uint8_t * dst, int off) {
  if (off & 1) {
    if (dst)
      dst[off] = 0;
    ++off;
  }
  return off;
}

/* Write data */
static int w_D(uint8_t * dst, int off, const void * src, int len) {
  if (dst)
    memcpy(dst+off,src,len);
  return off + len;
}

/* Write zero string */
static int w_S(uint8_t * dst, int off, const char * src) {
  return w_D(dst, off, src, strlen(src)+1);
}

/* Write zero string without trailing zero */
static int w_Z(uint8_t * dst, int off, const char * src) {
  return w_D(dst, off, src, strlen(src));
}

/* write 68k word */
static int w_W(uint8_t * dst, int off, const uint16_t val) {
  if (dst) {
    dst[off+0] = val>>8;
    dst[off+1] = val;
  }
  return off+2;
}

/* write 68k long */
static int w_L(uint8_t * dst, int off, const uint32_t val) {
  if (dst) {
    dst[off+0] = val>>24;
    dst[off+1] = val>>16;
    dst[off+2] = val>>8;
    dst[off+3] = val;
  }
  return off+4;
}

/* write 4 bytes in order */
static int w_4(uint8_t * dst, int off, const void * src) {
  return w_D(dst, off, src, 4);
}

/* Write tag and string value, only if defined */
static int w_TS(uint8_t * dst, int off, const char * t, const char *s) {
  if (t && s && *s) {
    off = w_Z(dst,off,t);
    off = w_S(dst,off,s);
  }
  return off;
}

static int w_T99(uint8_t * dst, int off, const char * t, int v) {
  if (t && v > 1) {
    char tmp[3];
    off = w_Z(dst,off,t);               /* write the tag */
    while (v >= 99) {
      v -= 99;
      off = w_D(dst,off,"99",2);        /* write '99' */
    }
    tmp[0] = '0' + v / 10u;             /* write '99' */
    tmp[1] = '0' + v % 10u;
    tmp[2] = 0;
    off = w_D(dst, off, tmp, 3);
  }
  return off;
}

static int isvalidstr(const char * s) {
  return s && *s;
}

static int16_t get_W(const uint8_t * buf) {
  return (((int16_t)(int8_t)buf[0]) << 8) | buf[1];
}

static int decode_68k(const uint8_t * buf, int * _off)
{
  int ret, w, off = *_off;

  if (off < 0 || off >= 10)
    return INST_ERR;                    /* Out of range */

  w = get_W(buf+off);
  off += 2;

  switch (w) {
  case 0X4E71:                          /* NOP */
    ret = INST_NOP;
    break;
  case 0x4E00:                          /* Illegal fix */
    if (off != 6)
      return INST_ERR;
    /* continue on rts */
  case 0x4E75:                          /* RTS */
    ret = INST_RTS;
    off -= 2;
    break;
  case 0x6000:                          /* BRA */
  case 0x4efa:                          /* JMP(PC) */
    if (off > 10)
      return INST_ERR;                  /* Out of range */
    off += get_W(buf+off);
    ret = INST_JMP;
    break;
  default:
    if ( (w & 0xff00) != 0x6000 )       /* BRA.S */
      return INST_ERR;
    off += -2 + (int8_t) w;
    ret = INST_JMP;
  }
  *_off = off;
  return ret;
}

static int decode_inst(const uint8_t * buf, sndh_boot_t * sb, int i, int msk)
{
  int off = sb->off[i];

  if (off != -1)
    return 0;
  off = i << 2;

  for (;;) {
    assert( !(off&1) );
    assert( off >= 0 );
    assert( off < 12 );

    if (msk & (1<<off)) {
      msgerr("sndh: <%s> infinite loop -- +$%04x\n",
             what[i], off);
      return -1;
    }
    msk |= 1<<off;

    switch (decode_68k(buf,&off)) {

    case INST_RTS:
      sb->off[i] = 0;
      return 0;

    case INST_JMP:
      if ((off & 1) || off < 0 || off >= sb->sym.data) {
        msgerr("sndh: <%s> invalid jump location -- +$%04x >= $%04x\n",
               what[i], off, sb->sym.data);
        return -1;
      }
      if (off < 12)
        break;
      sb->off[i] = off;
      return 0;

    case INST_NOP:
      if (off == 12) {
        msgerr("sndh: <%s> ran out range -- +$%04x\n",
               what[i], off);
        return -1;
      }
      break;

    default:
      assert(!"should not happen");
      /* continue on error */
    case INST_ERR:
      msgerr("sndh: unable to decode <%s> instruction at +$%04x\n",
             what[i], off);
      return -1;
    }
  }
  assert(!"should not reach that point");
  return -1;
}

static int decode_boot(const uint8_t * buf, int len, sndh_boot_t * sb)
{
  int i;

  /* init */
  for (i=0; i<3; ++i)
    sb->off[i] = -1;
  sb->off[i] = len;                     /* use this for range */

  /* decode 3 boot instructions */
  for (i=0; i<3; ++i)
    if (decode_inst(buf,sb,i,0) < 0)
      return -1;
    else
      msgdbg("sndh: <%s> offset is +$%04x\n", what[i], sb->off[i]);

  /* locate data */
  for (i=0; i<3; ++i) {
    int off = sb->off[i];

    if (!off)
      continue;                         /* RTS */

    if (off < 16) {
      msgerr("sndh: <%s> offset +$%04x is invalid\n", what[i], off);
      return -1;
    }

    if (off < sb->sym.data)
      sb->sym.data = off;
  }

  /* ensure data is in range */
  if (sb->sym.data < 16 || sb->sym.data >= len) {
    msgerr("sndh: unable to locate sndh data +$%04x\n", sb->sym.data);
    return -1;
  }

  /* looking for 'HDNS' tag */
  for (i=sb->sym.data-4; i>=16; --i)
    if (!memcmp(buf+i,"HDNS",4)) {
      msgdbg("sndh: 'HDNS' marker found at +$%04x\n",i+4);
      sb->sym.data = i+4;
      break;
    }

  if (i<16)
    msgwrn("sndh: no 'HDNS' marker; output might be corrupt\n");

  msgdbg("sndh: decode-boot: $%04x $%04x $%04x $%04x\n",
         sb->sym.init, sb->sym.kill, sb->sym.play, sb->sym.data);

  return 0;
}

/* decode sndh header */
static int decode_header(const uint8_t * buf, int len, sndh_header_t * hd)
{
  int over, upos, know, i;

  if (decode_boot(buf, len, &hd->boot))
    return -1;
  if (!memcmp(buf+hd->boot.sym.data-4,"HDNS",4))
    /* Need to decode sndh header if the end marker is missing */
    return 0;

  len = hd->boot.sym.data;
  for (upos=over=0, know=1, i=16; !over && i+3<len; ) {
    static const char strs[][4] = {
      { 'T','I','T','L' },            /* title     */
      { 'C','O','M','M' },            /* composer  */
      { 'R','I','P','P' },            /* ripper    */
      { 'C','O','N','V' },            /* converter */
      { 'Y','E','A','R' },            /* year      */
    };
    int tag = i, j;

    msgdbg("sndh: decode-header: pos:%04x/%04x '%c%c%c%c'\n",
           i,len,
           isgraph(buf[i+0])?buf[i+0]:'.',
           isgraph(buf[i+1])?buf[i+1]:'.',
           isgraph(buf[i+2])?buf[i+2]:'.',
           isgraph(buf[i+3])?buf[i+3]:'.');

    if (know) {
      upos = 0;
      know = 0;
    }

    /* End marker 'HDNS' */
    know = !memcmp(buf+i,"HDNS",4);
    if (know) {
      over = i += 4;
      continue;
    }

    /* Most string tags (see table above) */
    for (j=0; !know && j<sizeof(strs)/sizeof(*strs); ++j)
      know = !memcmp(buf+i,strs[j],4);
    if (know) {
      for(i+=4; i<len && buf[i++];);
      continue;
    }

    /* '##xx' and '!#xx' */
    know = (buf[i] == '#' || buf[i] == '!') && buf[i+1] == '#'
      && isdigit(buf[i+2]) && isdigit(buf[i+3]);
    if (know) {
      int n = 0, * ptr;
      for (i+=2; i<len && isdigit(buf[i]); ++i)
        n = n*10+buf[i]-'0';
      if (i >= len)
        continue;
      if (!buf[i])
        ++i;
      else
        msgwrn("sndh: tag '%c%c' not zero terminated\n",
               buf[tag], buf[tag+1]);

      ptr = (buf[tag+0] == '#') ? &hd->ntune : &hd->dtune;
      if (!n) n = 1;         /* Fix dodgy file */
      if (*ptr && n != *ptr) {
        msgerr("sndh: already have a differnt value for '#%c' -- %d != %d\n",
               buf[tag+1], *ptr, n);
        return -1;
      }
      *ptr = n;
      continue;
    }

    /* Timer */
    know = ( (buf[i+0] == '!' && buf[i+1] == 'V') ||
             (buf[i+0] == 'T' && buf[i+1]>='A' && buf[i+1]<='D') )
      && isdigit(buf[i+2]) && isdigit(buf[i+3]);
    if (know) {
      hd->rate = 0;
      hd->timer = buf[i+1];
      for (i+=2; i<len && isdigit(buf[i]); ++i)
        hd->rate = hd->rate*10+buf[i]-'0';
      if (i >= len)
        continue;
      if (!buf[i])
        ++i;
      else
        msgwrn("sndh: tag '%c%c' not zero terminated\n",
               buf[tag], buf[tag+1]);
      if (hd->rate < 25 || hd->rate > 1000)
        msgwrn("sndh: tag '%c%c' unlikely timer rate %dhz\n",
               buf[tag], buf[tag+1], hd->rate);
      continue;
    }

    /* Song names or song flags */
    know = !memcmp("!#SN", buf+i,4) || !memcmp("FLAG", buf+i,4);
    if (know) {
      int base = i, max;
      if (!hd->ntune) {
        msgwrn("sndh: '%s' before '##'\n",
               buf[i] == 'F' ? "FLAG" : "!#SN");
        hd->ntune = 1;
      }
      i += 4;
      max = i + (hd->ntune << 1);
      if (!get_W(buf+i+4))      /* Fix dodgy file */
        base = max;
      for (j=0; j<hd->ntune; ++j) {
        int str = base + get_W(buf+i+(j<<1));
        if (str < 16 || str >= len) {
          msgerr("sndh: song name string offset out of range\n");
          return -1;
        }
        if (str > max) max = str;
      }
      for (i=max; i<len && buf[i++];);
      continue;
    }

    know = !memcmp(buf+i,"TIME",4);
    if (know) {
      if (i&1) {
        msgwrn("sndh: 'TIME' is not aligned\n");
        ++i;
      }
      if (!hd->ntune) {
        msgwrn("sndh: 'TIME' before '##'\n");
        hd->ntune = 1;
      }
      i += 4 + (hd->ntune<<1);
      continue;
    }

    if (!upos)
      upos = i;
    if (i++ - upos > 7) {
      over = i = upos;
      msgdbg("sndh: no more tag found at +$%04x\n", over);
      continue;
    }
  }
  msgdbg("sndh: exit parser: pos:$%04x/$%04x unknown:$%04x over:$%04x\n",
         i,len,upos,over);

  if (over) {
    msgdbg("sndh: over at +$%04x\n", over);
    if (over < hd->boot.sym.data) {
      msgdbg("sndh: over %d byte(s) before data +$%04x\n",
             hd->boot.sym.data - over, over);
      hd->boot.sym.data = over;
    } else if (over == hd->boot.sym.data)
      msgdbg("sndh: over at data +$%04x\n", over);
  } else if (upos) {
    msgwrn("sndh: set data to last known valid position +$%04x\n",upos);
    hd->boot.sym.data = upos;
  } else if (i != hd->boot.sym.data) {
    hd->boot.sym.data = i;
    msgdbg("sndh: data now at +$%04x\n", hd->boot.sym.data);
  }
  msgdbg("sndh: decode-header: $%04x $%04x $%04x $%04x\n",
         hd->boot.sym.init, hd->boot.sym.kill,
         hd->boot.sym.play, hd->boot.sym.data);
  return 0;
}

static const char * tag(disk68_t *d, int track, const char * key)
{
  const char * s = file68_tag_get(d,track,key); /* try track */
  if ((!s || !*s) && track)
    s = file68_tag_get(d,0,key);        /* default to disk */

  if (s && !*s)
    s = 0;
  if (s)
    msgdbg("tag '%s' => '%s'\n", key, s);
  else
    msgdbg("tag '%s' missing\n", key);

  return s;
}

static int sndh_save_buf(const char *uri, const void *buf, int len , int gz)
{
  int ret = -1;
  if (!gz) {
    vfs68_t * vfs = uri68_vfs(uri, SCHEME68_WRITE, 0);
    if (vfs) {
      ret = vfs68_open(vfs);
      if (!ret) {
        ret = -(vfs68_write(vfs,buf,len) != len);
        vfs68_close(vfs);
        if (ret)
          msgerr("sndh: failed to save -- %s\n", uri);
        else
          msgnot("sndh: saved (%d bytes) -- %s\n", len, uri);
      }
      vfs68_destroy(vfs);
    }
  } else {
#ifndef USE_UNICE68
    msgerr("sndh: ICE! packer not supported\n");
#else
    int max = len + (len>>1);
    char * dst = malloc(max);
    if (!dst) {
      msgerr("sndh: %s\n", strerror(errno));
      ret = -1;
    } else {
      ret = unice68_packer(dst, max, buf, len);
      if (ret > 0) {
        dst[1]='C'; dst[2]='E';         /* fix ICE header */
        ret = sndh_save_buf(uri, dst, ret, 0);
      } else
        ret = -1;
      free(dst);
    }
#endif
  }
  return ret;
}

/* Build sndh 'FLAGS' string (alphabetical order is not required but
 * it might be easier for comparing things later). */
static int mk_flags(char * tmp, const hwflags68_t hw)
{
  int j = 0;
  if (hw & SC68_XTD) {
    if (hw & SC68_MFP_TA) tmp[j++] = 'a';
    if (hw & SC68_MFP_TB) tmp[j++] = 'b';
    if (hw & SC68_MFP_TC) tmp[j++] = 'c';
    if (hw & SC68_MFP_TD) tmp[j++] = 'd';
  }
  if (hw & (SC68_DMA|SC68_LMC)) tmp[j++] = 'e';
  if (hw & SC68_PSG) tmp[j++] = 'y';
  tmp[j++] = 0;

  msgdbg("mksndh: flags:0x%x %s (%d)\n",hw,tmp,j);
  return j;
}

/* Look up for a matching substring in the previous string table
 * entries. */
static int substr_lookup(const char *what, int l, const uint8_t *data, int n)
{
  int i;
  for (i=0; i<n; ++i) {
    const int off = get_W(data+4+i*2);
    const char * str = (const char *) data + off;
    const int len = strlen(str)+1;
    if (len < l)
      continue;
    if (!memcmp(what,str+len-l,l)) {
      msgdbg("sndh: re-using string table entry #%d:%d:'%s' for '%s'\n",
             i,len-l,data+off,what);
      assert(off >= 4);
      return off+len-l;
    }
  }
  /* not found */
  return 0;
}

static int create_sndh(uint8_t **_dbuf, sndh_header_t *hd, disk68_t *d)
{
  uint8_t * dbuf = 0;

  /* Process in 2 pass:
   * - pass #1 compute required space and allocate buffer
   * - pass #2 do the real stuff
   */
  for (;;) {
    char tmp[512];   /* large enough for flags string table */
    const char *s;
    int off , i;

    /* SNDH */
    off = w_4(dbuf, 12, "SNDH");        /* sndh header  */

    /* Title */
    s = tag(d,0,TAG68_ALBUM);
    off = w_TS(dbuf, off, "TITL", s);   /* album -> title */

    /* Author */
    s = tag(d,0,TAG68_ARTIST);
    off = w_TS(dbuf, off, "COMM", s);   /* artist -> composer */

    /* Ripper */
    s = tag(d, 1, TAG68_RIPPER);
    off = w_TS(dbuf, off, "RIPP", s);   /* ripper -> ripper */

    /* Converter */
    s = tag(d, 1, TAG68_CONVERTER);
    off = w_TS(dbuf, off, "CONV", s);   /* converter -> converter */

    /* Year */
    s = tag(d, 1, TAG68_YEAR);
    off = w_TS(dbuf, off, "YEAR", s);   /* converter -> converter */

    /* Number of song */
    off = w_T99(dbuf, off, "##", d->nb_mus);

    /* Default song */
    off = w_T99(dbuf, off, "!#", d->def_mus+1);

    /* Replay method and frequency */
    if (d->mus[0].frq != 50) {
      /* $$$ FIXME: Replay method and frequency */
      sprintf(tmp,"TC%i",d->mus[0].frq);
      off = w_S(dbuf, off, tmp);
    }

    /* Flags */
    if (d->hwflags) {
      int diff;

      /* check if all tracks share the same flags */
      for (diff=i=0; i<d->nb_mus; ++i)
        diff += d->hwflags != d->mus[i].hwflags;

      if (!diff) {
        /* All the same ! simple string "FLAG" */
        tmp[0] = '~';
        mk_flags(tmp+1, d->hwflags);
        off = w_TS(dbuf, off, "FLAG", tmp);
      } else {
        /* some flags differs */
        int loc, luo;

        memcpy(tmp,"FLAG",4);           /* write TAG */
        loc = 4+2*d->nb_mus;            /* Skip the offset table */

        for (i=0; i<d->nb_mus; ++i) {
          char flag[16];
          int l = mk_flags(flag, d->mus[i].hwflags);

          luo = substr_lookup(flag, l, (uint8_t *)tmp, i);
          if (!luo) {
            if (loc+l > sizeof(tmp)) {
              msgerr("sndh: overflow computing FLAG tag (%d > %d)\n",
                     (int)loc+l, (int)sizeof(tmp));
              return -1;
            }
            memcpy(tmp+loc, flag, l);
            luo = loc;
            loc += l;
          }
          /* Write string offset */
          assert( luo >= 4);
          w_W((uint8_t *)tmp, 4+i*2, luo);       /* Fill offset to string */
        }
        assert(loc <= sizeof(tmp));

        off = w_E(dbuf,off);
        if (dbuf)
          memcpy(dbuf+off, tmp, loc);   /* copy the whole tag data */
        off += loc;
      }
    }

    /* Disk time should be set only if all tracks duration is set.  */
    if (d->time_ms > 0 ) {
      off = w_E(dbuf,off);
      off = w_D(dbuf,off,"TIME",4);
      for (i=0; i<d->nb_mus; ++i) {
        unsigned int sec = (d->mus[i].first_ms+999u)/1000u;
        off = w_W(dbuf, off, sec);
        msgdbg("track #%03d %s\n",
               i+1, str_timefmt(tmp, sizeof(tmp), sec * 1000u));
      }
    }

    /* Song titles */

    /* looking for a track that have a valid title that differs from
     * the album title. */
    for (i=0; i<d->nb_mus; ++i)
      if (isvalidstr(d->mus[i].tags.tag.title.val) &&
          strcmp68(d->tags.tag.title.val, d->mus[i].tags.tag.title.val))
        break;

    if (i != d->nb_mus) {
      int tbl, bas;
      bas = off = w_E(dbuf,off);        /* Even / base string offset */
      tbl = w_D(dbuf,off,"!#SN",4);     /* Table after the tag */
      off = tbl+2*d->nb_mus;            /* Skip the table */
      for (i=0; i<d->nb_mus; ++i) {
        const char * s;
        w_W(dbuf,tbl+i*2,off-bas);      /* Fill offset to string */
        char tmp[12];

        if (isvalidstr(d->mus[i].tags.tag.title.val))
          s = d->mus[i].tags.tag.title.val;
        else if (isvalidstr(d->mus[i].tags.tag.title.val))
          s = d->mus[i].tags.tag.title.val;
        else {
          sprintf(tmp, "Track #%u",i+1);
          s = tmp;
        }
        off = w_S(dbuf,off,s);
      }
    }

    /* Footer */
    off  = w_4(dbuf, off, "HDNS");      /* sndh footer   */
    /* That's a bit tricky: the header and data must be aligned
     * together. */
    if ( (off ^ hd->boot.sym.data) & 1) {
      if (dbuf) dbuf[off] = 0;
      off += 1;
    }
    hd->hlen = off;                     /* header length */
    msgdbg("sndh: create-header: +$%04x/$%06x/$%06x\n",
           hd->hlen, hd->dlen, hd->hlen + hd->dlen);

    /* Rewrite boot */
    if (dbuf) {
      int * bt = hd->boot.off, data = hd->boot.sym.data;
      for (i=0; i<3; ++i) {
        if (!bt[i])
          w_L(dbuf, i*4, 0x4e754e75);
        else {
          w_W(dbuf, i*4+0, 0x6000);
          w_W(dbuf, i*4+2, (bt[i] - data + hd->hlen) - (i*4+2));
        }
      }
      *_dbuf = dbuf;
      return 0;
    }

    /* Finally alloc buffer and ready for pass 2 */
    dbuf = malloc(hd->hlen+hd->dlen);
    if (!dbuf) {
      msgerr("sndh: %s\n", strerror(errno));
      break;
    }
  }
  return -1;
}

static int save_sndh(const char *uri, disk68_t *d, int vers, int gz)
{
  int ret = -1;

  const uint8_t * sbuf = (const uint8_t *)d->mus[0].data;
  int             slen = d->mus[0].datasz;
  uint8_t       * dbuf = 0;
  sndh_header_t   hd;
  memset(&hd, 0, sizeof(hd));

  ret = decode_header(sbuf, slen, &hd);
  if (!ret) {
    hd.dlen = slen - hd.boot.sym.data;
    ret = create_sndh(&dbuf, &hd, d);
  }
  if (!ret)
    memcpy(dbuf+hd.hlen, sbuf+hd.boot.sym.data, hd.dlen);
  if (!ret)
    ret = sndh_save_buf(uri, dbuf, hd.hlen+hd.dlen, gz);
  free(dbuf);
  return ret;
}


static int save_sc68(const char *uri, disk68_t *d, int vers, int gz)
{
  int ret = -1;
  msgerr("sndh: save '%s' version:%d compress:%d"
         " -- sc68 -> sndh unsupported\n", uri, vers, gz);
  return ret;
}

int sndh_save(const char *uri, void *dsk, int vers, int gz)
{
  int ret;
  disk68_t * d = (disk68_t *)dsk;

  if (!strcmp68(d->tags.array[TAG68_ID_FORMAT].val,"sc68"))
    ret = save_sc68(uri, d, vers, gz);
  else
    ret = save_sndh(uri, d, vers, gz);

  return ret;
}
