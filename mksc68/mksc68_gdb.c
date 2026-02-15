/*
 * @file    mksc68_gdb.c
 * @brief   remote gdb functions.
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

#include "mksc68_gdb.h"
#include "mksc68_dsk.h"
#include "mksc68_msg.h"
#include "mksc68_cmd.h"
#include "mksc68_tag.h"
#include "mksc68_str.h"

#include <sc68/file68.h>
#include <sc68/file68_vfs.h>
#include <sc68/sc68.h>
#include <emu68/emu68.h>
#include <emu68/excep68.h>
#include <io68/io68.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#else
# ifdef HAVE_WINSOCK2_H
#  include <winsock2.h>
# endif
# ifdef HAVE_WINSOCK_H
#  include <winsock.h>
# endif
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

/* #include <sys/un.h> */
/* #include <netinet/in.h> */
/* #include <arpa/inet.h> */
/* #include <netdb.h> */

#define STATUS(S,C,M)                                                   \
  do { gdb->run = RUN_##S; gdb->code = CODE_##C; gdb->msg = M; } while(0)
#define SIGNAL(S,C,M)                                                   \
  do { gdb->run = RUN_##S; gdb->code = -C; gdb->msg = M; } while(0)

enum {
  default_port = 6800,
  default_addr = 0x7f000001
};

static uint8_t thex[16] = {
  '0','1','2','3','4','5','6','7',
  '8','9','a','b','c','d','e','f'
};

static inline int xdigit(const int v) {
  if (v >= '0' && v <= '9') return v - '0';
  if (v >= 'a' && v <= 'f') return v - 'a' + 10;
  if (v >= 'A' && v <= 'F') return v - 'A' + 10;
  return -1;
}

static int strtoux(char ** pptr, uint_t * pval)
{
  char * ptr = *pptr; uint_t val = 0; int d;
  if (d = xdigit(*ptr), d >= 0) {
    val = d;
    while (d = xdigit(*++ptr), d >= 0)
      val = (val << 4) | d;
    *pval = val;
    *pptr = ptr;
    return 0;
  }
  return -1;
}

static int strtox(char ** pptr, int * pval) {
  char * ptr = *pptr; int minus = 0;
  uint_t uval;

  if (*ptr == '-') {
    ++ptr;
    minus = 1;
  }
  if (!strtoux(&ptr,&uval)) {
    *pval = minus ? -uval : uval;
    *pptr = ptr;
    return 0;
  }
  return -1;
}

typedef struct serverinfo_s serverinfo_t;

struct serverinfo_s {
  int       fd;                         /* server listening socket */
  uint32_t  addr;                       /* bound ipv4 address      */
  uint16_t  port;                       /* bound port              */
};

serverinfo_t serverinfo = { -1, default_addr, default_port };

struct gdb_s {
  int            fd;                    /* client/server socket        */
  serverinfo_t * server;                /* gdb server                  */
  emu68_t      * emu;                   /* emu68 instance              */
  int            code;                  /* exit or stop code           */
  int            step;                  /* number of step before break */
  int            run;                   /* run mode                    */
  int            vector;                /* last interrupt vector       */
  int            sigval;                /* last signal                 */

  struct {
    uint_t ontimer_break:8;             /* 4-bit:reset | 4-bit: */
  } opt;

  struct {
    uint_t noack:1;                        /* QStartNoAckMode+ */
    uint_t nonstop:1;                      /* QNonStop+ */
    uint_t attached:1;                     /* QAttached */
  } mode;

  const char   * msg;                   /* last message                */
  char           msgbuf[128];           /* for formatted msg           */

  int            bmax;                  /* buffer max - 1              */
  int            blen;                  /* bufferered chars (0:empty)  */
  char           buf[1024];             /* buffer for send/recv        */
};

static inline uint8_t Bpeek(emu68_t * const emu68, addr68_t addr)
{
  return emu68_peek(emu68, addr);
}

static uint16_t Wpeek(emu68_t* const emu68, addr68_t  addr)
{
  return (uint16_t) ( ( (uint16_t) Bpeek(emu68, addr+0) << 8 ) |
                      ( (uint16_t) Bpeek(emu68, addr+1)      ) );
}

static uint32_t Lpeek(emu68_t* const emu68, addr68_t  addr)
{
  return (uint32_t) ( ( (uint32_t) Bpeek(emu68, addr+0) << 24 ) |
                      ( (uint32_t) Bpeek(emu68, addr+1) << 16 ) |
                      ( (uint32_t) Bpeek(emu68, addr+2) <<  8 ) |
                      ( (uint32_t) Bpeek(emu68, addr+3)       ) );
}

struct trap_s {
  const char * name;
  const char * parm;
};

#ifndef HAVE_STPCPY
/* Sometimes we don't have stpcpy. That's weird and it is probably a
 * configuration problem regarding GLIBC features.  Anyway here is a
 * very simple replacement.
 */
static char * stpcpy(char *dst, const char * src)
{
  int c;
  while (c = *src++, c)
    *dst++ = c;
  *dst = c;
  return dst;
}
#endif

/* In case hstrerror() does not exist just return a very generic error
 * message.
 */
static const char * sock_error(void)
{
#ifdef HAVE_hstrerror
  return hstrerror(h_errno);
#else
  return "undefined network error";
#endif
}

static const char * msgf(gdb_t * gdb, const char * fmt, ...)
{
  const int max = sizeof(gdb->msgbuf);
  va_list list;
  va_start(list,fmt);
  if ( max == vsnprintf(gdb->msgbuf,max,fmt,list) )
    gdb->msgbuf[max-1]=0;
  va_end(list);
  return gdb->msgbuf;
}

static int decode_trap(char * s, int trapnum, emu68_t * emu)
{
  const int sig_ok = SIGVAL_0;   /* known traps */
  const int sig_no = SIGVAL_SYS; /* unknown traps (invalid syscall) */
  int fct, sigval = sig_ok;      /* Ok by default */
  char * const msg = s;          /* Keep track of this address */

  switch (trapnum) {

  case 1:
    /***********************************************************************
     * Gemdos (trap #1)
     */
    fct = Wpeek(emu, emu->reg.a[7]+6);
    s += sprintf(s," gemdos(%02x)", fct);
    switch (fct) {
    case 0x09:
      s += sprintf(s, " cconws(0x%x.l)", Lpeek(emu, emu->reg.a[7]+8));
      break;
    case 0x20:
      s += sprintf(s, " super(0x%x.l)" , Lpeek(emu, emu->reg.a[7]+8));
      break;
    case 0x30:
      s += sprintf(s, " version()");
      break;
    case 0x48:
      s += sprintf(s, " malloc(%d.l)", Lpeek(emu, emu->reg.a[7]+8));
      break;
    case 0x49:
      s += sprintf(s, " free(0x%x.l)",  Lpeek(emu, emu->reg.a[7]+8));
      break;
    default:
      sigval = sig_no;
    }
    break;

  case 13:
    /***********************************************************************
     * Bios (trap #13)
     */
    s += sprintf(s," bios(%02x)", Wpeek(emu, emu->reg.a[7]+6));
    sigval = sig_no;
    break;

  case 14:
    /***********************************************************************
     * Xbios (trap #14)
     */
    fct = Wpeek(emu, emu->reg.a[7]+6);
    s += sprintf(s," xbios(%02x)", fct);
    switch (fct) {
    case 0x1f:
      s += sprintf(s, " xbtimer(%d.w,0x%02x.w,0x%02x.w,0x%x.l)",
                   Wpeek(emu, emu->reg.a[7]+8),
                   Wpeek(emu, emu->reg.a[7]+10),
                   Wpeek(emu, emu->reg.a[7]+12),
                   Lpeek(emu, emu->reg.a[7]+14));
      break;
    case 0x20:
      s += sprintf(s, " dosound(0x%x.l)", Lpeek(emu, emu->reg.a[7]+8));
      break;
    case 0x26:
      s += sprintf(s, " superexec(0x%x.l)", Lpeek(emu, emu->reg.a[7]+8));
      break;
    case 0x80:
      s += sprintf(s, " locksnd()");
      break;
    case 0x81:
      s += sprintf(s, " unlocksnd()");
      break;
    default:
      sigval = sig_no;
    }
    break;

  case 0:
    /***********************************************************************
     * sc68 (trap #0)
     */
    fct = Lpeek(emu, emu->reg.a[7]+6);

    if ((fct & 0xFFFFFFF0) == 0x5C68DB60) {
      const char * typestr = 0;
      const u8 * memptr = emu->mem;
      const addr68_t msk  = emu->memmsk;
      /* We Will return from exception manually and correct the stack
       * manually (skip addq #8,a7 */
      const u16 sr = Wpeek(emu, emu->reg.a[7]+0);
      const u32 pc = (Lpeek(emu, emu->reg.a[7]+2) + 2) & msk;
      u32 sp = emu->reg.a[7];

      /* msgdbg("Will set PC/SR/SP = $%06X/$%04X/%06X\n", pc,sr,sp); */

      fct &= 15;
      switch (fct) {
      case 0:                           /* message only */
        typestr = "msg";
        sigval  = SIGVAL_0;             /* Never breaks on message */
        break;
      case 1:                           /* breakpoint on debug */
        typestr = "brk";
        sigval  = SIGVAL_TRAP;          /* breaks on breakpoint */
        emu->reg.pc = pc;               /* restore postion */
        emu->reg.sr = sr;
        sp += 14;
        break;
      case 2:
        typestr = "stp";
        sigval  = SIGVAL_TRAP;          /* break always */
        emu->reg.pc = pc;
        emu->reg.sr = sr;
        sp +=14;
        break;
      case 3:
        typestr = "hlt";
        sigval  = SIGVAL_TRAP;          /* halt */
        emu->reg.pc = pc;
        emu->reg.sr = sr;
        sp +=14;
        break;

      default:
        typestr = "???";
        sigval  = sig_no;               /* invalid sys-call */
      }

      s += sprintf(s," sc68<%s>(%x)", typestr, fct);
      /* Get follow up text message */
      fct = Lpeek(emu, emu->reg.a[7]+10);
      emu->reg.a[7] = sp;
      if (fct) {
        int i;
        char tmp[256];
        for (i=0; i<sizeof(tmp)-1 && (tmp[i]=memptr[(fct+i)&msk]); ++i)
          ;
        tmp[i] = 0;
        s += sprintf(s," \"%s\"", tmp);
        msginf("gdb: %s\n", tmp);
      }
      break;
    }
    /* continue on default */

  default:
    sigval = sig_no;
    *s = 0;
  }
  TRACE("trap_decode(%d) -> %d [%s]\n", trapnum, sigval, msg);

  return sigval;
}


/* Create and bind ipv4/tcp server socket.
 * @return 0
 * @retval -1 on error
 */
static int server_tcp(serverinfo_t * si, int force)
{
  int fd = -1;
  struct sockaddr_in ia;

  if (si->fd != -1) {
    if (!force) {
      msgerr("already have a server socket\n");
      goto error;
    } else {
      msgwrn("forced to close previous server socket\n");
      close(si->fd);
      si->fd = -1;
    }
  }

  /* Prepare for bind */
  ia.sin_family      = AF_INET;
  ia.sin_port        = htons(si->port);
  ia.sin_addr.s_addr = htonl(si->addr);

  /* Create listen socket. */
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    msgerr("socket -- %s\n", strerror(errno));
    goto error;
  }

  /* Bind socket to ip/port address */
  if (bind(fd,(struct sockaddr*)&ia,sizeof(ia))) {
    msgerr("bind -- %s\n", strerror(errno));
    goto error;
  }
  si->fd = fd;

  return 0;

error:
  if (fd != -1)
    close(fd);
  return -1;
}

static int mysend(gdb_t * gdb, char * buf, int len)
{
  int n;
  assert(len >= 0);
  TRACE("SEND <%s> (%d)\n", buf, len);
  n = send(gdb->fd, buf, len, 0);
  if (n == -1) {
    STATUS(EXIT,ERROR,
           msgf(gdb,"send failed (#%d %s)", errno, strerror(errno)));
    msgerr("gdb-stub: socket -- %s\n", gdb->msg);
  } else if (n != len) {
    STATUS(EXIT,ERROR,
           msgf(gdb,"send closed (got %d/%d)", n, len));
    msgerr("gdb-stub: socket -- %s\n", gdb->msg);
  }
  return n;
}

static int myrecv(gdb_t * const gdb, int len, char ** ptr)
{
  const int bmax = sizeof(gdb->buf);
  int n, blen;
  char * buf;
  assert(len >= 0 && len < bmax);

  blen = gdb->blen + len;
  if (blen >= bmax) {
    errno = 0;
    STATUS(EXIT,ERROR,
           msgf(gdb,"recv buffer overflow (%d>=%d)\n", blen, bmax));
    msgerr("gdb-stub: %s\n", gdb->msg);
    return -1;
  }

  buf = gdb->buf+gdb->blen;
  if (ptr)
    *ptr = buf;
  n = recv(gdb->fd, buf, len, 0);
  if (n == -1) {
    STATUS(EXIT,ERROR,
           msgf(gdb,"recv failed (#%d %s)",errno,strerror(errno)));
    msgerr("gdb-stub: %s\n", gdb->msg);
  } else {
    if (n != len) {
      STATUS(EXIT,ERROR,
             msgf(gdb,"recv closed (got %d out of %d)", n, len));
      msgerr("gdb-stub: %s\n", gdb->msg);
      blen = gdb->blen + n;
    }
    gdb->blen = blen;
  }

  assert (gdb->blen <= gdb->bmax);
  gdb->buf[gdb->blen] = 0;
  /* msgdbg("got %d [%s]\n", n, gdb->buf); */
  return n;
}

static int recv_nowait(gdb_t * gdb, char * ptr)
{
  int n = 0;

  if (gdb->blen+1 < gdb->bmax) {
#ifndef MSG_DONTWAIT
# define MSG_DONTWAIT 0
# if defined(FIONREAD) && defined(HAVE_IOCTL)
    if (ioctl(gdb->fd,FIONREAD,&n) < 0) {
      STATUS(EXIT,ERROR,
             msgf(gdb,"recv failed (#%d %s)",errno,strerror(errno)));
      msgerr("gdb-stub: %s\n", gdb->msg);
      return -1;
    }
    if (n < 1)
      return 0;
    n = 0;
# else
    /* No alternative method ... bailing out */
    return 0;
# endif
#endif
    n = recv(gdb->fd, &gdb->buf[gdb->blen], 1, MSG_DONTWAIT);
    if (n > 0) {
      assert(n == 1);
      if (ptr) *ptr = gdb->buf[gdb->blen];
      gdb->buf[++gdb->blen] = 0;
    } else if (n < 0) {
      assert (n == -1);
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        n = 0;                           /* not an error  */
      else {
        STATUS(EXIT,ERROR,
               msgf(gdb,"recv failed (#%d %s)",errno,strerror(errno)));
        msgerr("gdb-stub: %s\n", gdb->msg);
      }
    } else {
      assert(n == 0);
    }

  }
  return n;
}

static int recv_ch(gdb_t * gdb)
{
  return (myrecv(gdb, 1, 0) == 1)
    ? gdb->buf[gdb->blen-1]
    : -1
    ;
}

/* Generate an error on later use of recv/send */
#define send(FD,BUF,LEN,FLAG) err_mysend(gdb,BUF,LEN)
#define recv(FD,BUF,LEN,FLAG) err_myrecv(gdb,BUF,LEN)


/**
 *  @retval  0  on success (packet has been ack or does not need ack)
 *  @retval  1  on error   (packet has not been ack)
 *  @retval -1  on failure (communication failure)
 */
static int send_packet(gdb_t * gdb)
{
  int  n, i, j, sum = 0;
  char c;
  char * const buf = gdb->buf;

  buf[0] = '$';
  for (i=1, sum=0; !!(c = buf[i]); ++i)
    sum += c;
  j = i;                              /* '#' marker index */
  buf[i++] = '#';                     /* write '#' marker */
  buf[i++] = thex[ (sum>>4) & 15 ];   /* write checksum   */
  buf[i++] = thex[  sum     & 15 ];   /* write checksum   */
  buf[i] = 0;

  errno = 0;
  if ( (n = mysend(gdb, buf, i)) != i )
    return -1;
  buf[j] = 0;                         /* remove '#' marker */

  if (gdb->mode.noack)
    return 0;

  /* Waiting for ACK (either '-' or '+') */
  gdb->blen = 0;
  do {
    char * ptr;
    if (myrecv(gdb, 1, &ptr) != 1) /* recv ACK */
      return -1;
    c = *ptr;
    if (c == '+')
      return 0;
  } while (c != '-');
  return 1;
}

/**
 *  @retval  0  on success (packet has been ack)
 *  @retval -1  on failure (communication failure)
 */
static int send_or_fail(gdb_t * gdb)
{
  int ret;
  while ( (ret = send_packet(gdb)) > 0)
    ;
  /* TRACE("send_or_fail -> [%d]\n", ret); */
  return ret;
}

/**
 *  @retval  0  on success (packet has been ack)
 *  @retval -1  on failure (communication failure)
 */
static int send_reply(gdb_t * gdb, int type, int code)
{
  if (code < 0)
    code = -code;
  gdb->buf[1] = type;
  gdb->buf[2] = thex [ (code >> 4) & 15 ];
  gdb->buf[3] = thex [  code       & 15 ];
  gdb->buf[4] = 0;
  return send_or_fail(gdb);
}

/**
 *  @retval  0  on success
 *  @retval  1  on failure
 */
static int rsp_event(gdb_t * const gdb)
{
  char * const buf = gdb->buf;
  char * const max = gdb->buf + gdb->bmax;
  char * ptr;
  int v;

  assert(gdb->run == RUN_IDLE);
  assert(gdb->code == CODE_IDLE);

  errno = 0;
  gdb->code = CODE_IDLE;
  gdb->msg = "waiting for command";

  /* Read a packet */

  /* Look for packet header '$' */
  do {
    gdb->blen = 0;
    if ( (v = recv_ch(gdb)) < 0)
      return -1;
  } while (v != '$');

  /* Fill the buffer to the footer '#' */
  for (;;) {
    if ( (v = recv_ch(gdb)) < 0)
      return -1;
    if (v == '#') break;
    if (v == '$') {
      msgwrn("gdb-stub: unexpected header at #%d\n",gdb->blen);
      buf[0] = '$';
      buf[1] = 0;
      gdb->blen = 1;
    }
  }
  /* Skip check sum, our line is reliable :) */
  if ( myrecv(gdb, 2 , 0) != 2 ) return -1;
  TRACE("<<<< %s\n",buf);
  /* Kill checksum */
  gdb->buf[gdb->blen-3] = 0;

  /* Unless noAck mode, send ACK '+' */
  if (!gdb->mode.noack) {
    gdb->msg = "sending ACK";
    if ( mysend(gdb, "+", 1) != 1 ) return -1;
  }

  gdb->msg = "parsing command";
  ptr = buf+1;
  switch(*ptr) {

    /* Minimal commands for a stub to implement are
     * 'g' 'G' 'm' 'M' 'c' 's'
     */

  case '?': {
    /***********************************************************************
     * ? request status (why target is halted)
     */
    TRACE("RECV <%s>\n",ptr);
    ptr = stpcpy(buf+1,"S05");          /* $$$ TODO */
  } break;

  case 'p': case 'P': {
    /***********************************************************************
     * P n...=r... (Write register)
     * p n (Read register)
     *
     * Register mapping for 68k targets:
     *  00-07 D0-D7     32-bit
     *  08-15 A0-A7     32-bit
     *  16    SR/PS     16-bit
     *  17    PC        32-bit
     *  18-25 FP0-FP7   80-bit but 64-bit for gdb
     *  26    FPC       32-bit
     *  27    FPS       32-bit
     *  28    FPI       32-bit
     */
    uint_t n, v, l, read = *ptr++ == 'p';

    if (strtoux(&ptr, &n))
      break;
    if (read) {
      TRACE("RECV <read-register,%02d>\n", n);
      v = 0;                          /* default value */
      l = 8;                          /* default length of 8 xdigit */
      if (n < 8)
        v = gdb->emu->reg.d[n];
      else if (n < 16)
        v = gdb->emu->reg.a[n-8];
      else if (n == 16) {
        v = gdb->emu->reg.sr;
        l = 4;
      } else if (n == 17) {
        v = gdb->emu->reg.pc;
      } else if (0 && n < 26) {
        l = 16;                         /* 64-bit ? */
      } else {
        /* Don't support FP or extra registers but do not reply just
         * ignore to tell gdb we do not support this register. */
        ptr = 0; /* stpcpy(buf+1,"E05"); */
        break;
      }
      ptr = buf + 1 + sprintf(buf+1,"%0*x",l,v);
      break;
    } else if (*ptr == '=') {
      char * end=++ptr;
      if (!strtoux(&end, &v)) {
        TRACE("RECV <write-register,%02d,0x%x>\n",n,v);
        ptr = stpcpy(buf+1,"OK");
        if (n < 8)
          gdb->emu->reg.d[n] = v;
        else if (n < 16)
          gdb->emu->reg.a[n-8] = v;
        else if (n == 16)
          gdb->emu->reg.sr = v & 0xFFFF;
        else if (n == 17)
          gdb->emu->reg.pc = v;
        else
          ptr = stpcpy(buf+1,"E05");
        break;
      }
    }
    ptr = stpcpy(buf+1,"E01");
  } break;

  case 'g': {
    /***********************************************************************
     * g (read General Registers)
     */
    int i,j;

    TRACE("RECV <%s>\n","read-general-registers");
    ptr = buf+1;
    for (i=0;i<8;++i)
      for (j=0; j<8; ++j)
        *ptr++ = thex[ ( gdb->emu->reg.d[i] >> ((7-j)<<2)) & 15];
    for (i=0;i<8;++i)
      for (j=0; j<8; ++j)
        *ptr++ = thex[ ( gdb->emu->reg.a[i] >> ((7-j)<<2)) & 15];
    for (j=0; j<8; ++j)
      *ptr++ = thex[ ( gdb->emu->reg.sr >> ((7-j)<<2)) & 15];
    for (j=0; j<8; ++j)
      *ptr++ = thex[ ( gdb->emu->reg.pc >> ((7-j)<<2)) & 15];
    *ptr = 0;
  } break;

  case 'm': case 'M': {
    /***********************************************************************
     * m addr,length (read memory)
     */
    uint_t adr, len, read = *ptr++ == 'm';
    if ( !strtoux(&ptr, &adr) && *ptr++ == ',' && !strtoux(&ptr, &len)) {
      u8 * mem;
      int i;
      TRACE("RECV <memory-%cet,0x%06x,+%u>\n",read["sg"],adr,len);

      /* Despite the PacketSize being know by gdb it still sends
       * queries which full reply would overflow the buffer. Clamping
       * len So it's ourClamping len to avoid such overflow. gdb will
       * send subsequent commands to get what the remaining data.
       */
      if (buf+1+len*2 > max)
        len = (max-(buf+1)) >> 1;

      mem = emu68_memptr(gdb->emu, adr, len);
      if (!mem) {
        gdb->msg = emu68_error_get(gdb->emu);
        msgnot("gdb tried to access invalid memory [$%x..$%x]\n",
               adr,adr+len-1);
      } else if (read && !*ptr) {
        for (i=0, ptr = buf+1; i<len; ++i) {
          *ptr++ = thex[ mem[i] >> 4 ];
          *ptr++ = thex[ mem[i] & 15 ];
        }
        assert(ptr <= max);
        *ptr = 0;
        break;
      } else if (!read && *ptr == ':') {
        for (i=0; i<len; ++i) {
          int a,b;
          if ( (a = xdigit(*++ptr)) >= 0 && (b = xdigit(*++ptr)) >= 0)
            mem[i] = (a<<4) + b;
          else
            break;
        }
        if (i == len) {
          stpcpy(buf+1, "OK");
          break;
        }
      }
    }
    ptr = stpcpy(buf+1,"E01");
  } break;

  case 'k': {
    /***********************************************************************
     * k (kill)
     */
    TRACE("RECV <%s>\n","kill");
    STATUS(EXIT,KILL,"properly killed");
    return 0;
  } break;

  case 'D': {
    /***********************************************************************
     *  D[:pid] (detach)
     */
    char * end;
    int pid = 0;

    end = strchr(ptr+1,':');
    if (end) {
      *end = 0;
      ptr = end+1;
      pid = -1;
      strtox(&ptr, &pid);
    }
    TRACE("RECV <detach,%d>\n", pid);
    ptr = stpcpy(buf+1, "OK");
    STATUS(SKIP,DETACH,"detached");
  } break;

  case 'T': {
    /***********************************************************************
     * T thread-id (is thread alive ?)
     */
    int tid;
    ptr = buf+1;
    TRACE("RECV <thread-status> <%s>\n", ptr);
    if (strtox(&ptr,&tid))
      ptr = stpcpy(buf+1,"E01");
    else if (tid != 1)                  /* my thread ? */
      ptr = stpcpy(buf+1,"E02");
    else
      ptr = stpcpy(buf+1, "OK");
  } break;

  case 'H': {
    /***********************************************************************
     * H op thread-id (Thread Operation)
     */
    int op  = *++ptr;
    const char * tid = op ? ++ptr : "";

    switch (op) {
    case 'g':
      TRACE("RECV <thread-op-general> <%s>\n", tid);
      ptr = stpcpy(buf+1,"OK");
      break;
    case 'c':
      TRACE("RECV <thread-op-continue> <%s>\n", tid);
      ptr = stpcpy(buf+1,"OK");
      break;
    default:
      ptr = stpcpy(buf+1,"E01");
    }
  } break;

  case 'Q': case 'q': {
    /***********************************************************************
     * q Query (General Query)
     * Q Query (General Set)
     */
    const int query = *ptr == 'q';      /* query or set ? */
    char * name, * parm;

    name = ptr + 1;
    parm = strchr(name,':');
    if (parm)
      *(ptr = parm++) = 0;
    else
      ptr = name+strlen(name);

    TRACE("%s <%s>%s\n",query?"query":"set", name, parm ? parm : "<n/a>");


    /* The new Syntax is !q Name : parm, parm; ,,, . Some older
     * commands does not follow that syntax including qC,QL,qP. The
     * documentation states that New packets should not begin with
     * those 3. However there is a qCRC packet ! */

    if (query) {
      if (name[0] == 'C' && !name[1]) {
        /* <qC> query current thread-id */
        TRACE("RECV <%s>\n","query-thread-id");
        ptr = stpcpy(buf+1,"QC1");
      }
      /* else if (name[0] == 'P' && !parm && xdigit(name[1]) >= 0) { */
      /* <qP mode thread-id> query thread info
       * Deprecated by qThreadExtraInfo
       */
      /*   ptr = 0; */
      /* } */
      /* else if (name[0] == 'L' && !parm) { */
      /* qL start count next
       * Deprecated by qfThreadInfo
       */
      /*   ptr = 0; */
      /*   break; */
      /* } */
      else if (!strcmp(name,"fThreadInfo")) {
        /* qfThreadInfo: list of active threads */
        ptr = stpcpy(buf+1,"m1");
        break;
      }
      else if (!strcmp(name,"sThreadInfo")) {
        /* qsThreadInfo: list of active thread (next) */
        ptr = stpcpy(buf+1,"l");         /* end of list */
        break;
      }
      else if (!strcmp(name,"Offsets")) {
        /* query section relocations */
        TRACE("RECV <%s>\n","query-offsets");
        ptr = buf+1+sprintf(buf+1, "Text=%x;Data=%x;Bss=0",0,0);
      }
      else if (!strcmp(name,"Supported")) {
        static const char supported[] =
          "QStartNoAckMode+"            /* + noAck mode  */
          ";" "multiprocess-"           /* - multi process */
          ";" "xmlRegisters-"           /* - */
          ";" "qRelocInsn-"             /* - */
          ";" "QNonStop-"               /* - nonStop mode */
          ;
        TRACE("RECV <query-supported,%s>\n", ptr);
        ptr = stpcpy(buf+1, supported);
        ptr += sprintf(ptr,";PacketSize=%u", gdb->bmax);
      }
      else if (!strcmp(name,"Attached")) {
        /* qAttached(:pid) -- created or attached process
         * Change the behavior on gdb session end to either kill or detach.
         */
        buf[1] = '0' + gdb->mode.attached;
        buf[2] = 0;
        ptr = buf+2;
      }
      else if (!strcmp(name,"TStatus")) {
        /* TStatus -- Tracepoints status ( )*/
        TRACE("RECV <%s>\n","query-tracepoint-status");
        ptr = stpcpy(buf+1, "T0;tnotrun:0");
        ptr = 0;                        /* force unsupported */
      }
      else {
        TRACE("RECV *UNSUPPORTED* <query-%s,%s>\n",name,ptr);
        ptr = 0; /* stpcpy(buf+1,"E55"); */
      }
    } else {
      /*************************************************************
       * Set
       */
      if (!strcmp(name,"StartNoAckMode")) {
        TRACE("RECV <set-%s>\n", name);
        gdb->mode.noack = 1;
        ptr = stpcpy(buf+1,"OK");
      } else if (!strcmp(name,"NonStop")) {
        switch (*ptr) {
        case '0':                       /* all-stop */
          TRACE("RECV <%s>\n","set-all-stop");
          ptr = stpcpy(buf+1,"OK");
          break;
        case '1':                       /* non-stop */
          TRACE("RECV <%s>\n","set-non-stop");
          ptr = stpcpy(buf+1,"E01");
          break;
        default:
          ptr = 0;
        }
      }  else {
        TRACE("RECV *UNSUPPORTED* <set-%s,%s>\n",name,ptr);
        ptr = 0; /* stpcpy(buf+1,"E55"); */
      }
    }
  } break;

  case 'z': case 'Z': {
    /***********************************************************************
     * z type,addr,kind (remove breakpoint)
     * Z type,addr,kind (insert breakpoint)
     */

    /* static const char * types[5] = { */
    /*   "software-breakpoint", */
    /*   "hardware-breakpoint", */
    /*   "write-watchpoint", */
    /*   "read-watchpoint", */
    /*   "access-watchpoint" }; */


    const int remove = *ptr++ == 'z';
    const int type = *ptr++;
    uint_t adr  = gdb->emu->reg.pc;    /* $$$ default to pc ? */
    uint_t kind = -1;                  /* undef */

    if (*ptr == ',') {
      ++ptr;
      if (strtoux(&ptr, &adr)) {
        ptr = stpcpy(buf+1,"E01");
        break;
      }
    }

    if (*ptr == ',') {
      ++ptr;
      if (strtoux(&ptr, &kind)) {
        ptr = stpcpy(buf+1,"E02");
        break;
      }
    }
    kind = kind;

    TRACE("RECV <%s-brkp-type%c,%x,%d%s>\n",
          remove?"remove":"insert", type,
          adr, kind, ptr);

    switch (type) {
    case '0':
      /* software breakpoint. using hardware right now */
    case '1':
      /* hardware breakpoint. Unsupported by target according to gdb
       * which is really annoying if you ask me. gdb should not
       * assume such things when connected to any kind of remote
       * target.
       */
      if (remove) {
        int id = emu68_bp_find(gdb->emu,adr);
        if (id < 0) {
          msgnot("could not find breakpoint @ 0x%x\n", adr);
          ptr = stpcpy(buf+1,"E03");
        } else {
          emu68_bp_del(gdb->emu, id);
          ptr = stpcpy(buf+1,"OK");
          TRACE("removed breakpoint #%02d @ 0x%x\n", id, adr);
        }
      } else {
        int id = emu68_bp_set(gdb->emu, -1, adr, 1, 1);
        if (id < 0) {
          msgnot("could not add breakpoint @ 0x%x\n", adr);
          ptr = stpcpy(buf+1,"E03");
        } else {
          TRACE("insert breakpoint #%02d @ 0x%x\n", id, adr);
          ptr = stpcpy(buf+1,"OK");
        }
      }
      break;
    default:
      TRACE("unsupported breakpoint type '%c' @ 0x%x\n", type, adr);
      ptr = 0;                          /* unsupported */
      break;
    }
  } break;

  case 's': case 'c': case 'C': {
    /***********************************************************************
     * c [addr] (continue)
     * s [addr] (step)
     */
    int cmd  = *ptr;
    int step = cmd == 's';
    int sig  = -1;

    if (!*++ptr) {
      TRACE("RECV <%s>\n", step ? "step" : "cont");
    } else {
      uint_t adr;
      errno = 0;
      if (cmd == 'C') {
        ++ptr;
        if (strtox(&ptr, &sig)) {
          ptr = stpcpy(buf+1,"E02");
          break;
        }
        if (*ptr != ';') *ptr = 0;
      }
      ++ptr;
      if (strtoux(&ptr, &adr)) {
        ptr = stpcpy(buf+1,"E01");
        break;
      }
      TRACE("RECV <%s-at,S%02x,$%06x>\n",
            step ? "step" : "cont", sig, adr);
      gdb->emu->reg.pc = adr;
    }

    if (step) {
      gdb->step = 1;                    /* 1 */
    } else {
      gdb->step = 0;                    /* 0 is unlimited */
    }
    gdb->run  = RUN_CONT;

    switch (sig) {
    case 1: case 2: case 3: case 6:
      SIGNAL(EXIT,sig,"signal");
      sprintf(ptr = buf+1, "X%02u", sig % 100u);
      break;
    default:
      /* Do we or do we not send a reply here ?  If we don't gdb will
       * wait passively for the next packet we send, that would be for
       * instance a S## or whatever.
       */
      if (0)
        ptr = stpcpy(buf+1,"OK");       /* send OK */
      else
        return 0;                       /* skip sending reply */
    }
  } break;

  default:
    ptr = 0;
  }

  /* Any command not supported by the stub, an empty response
   * (‘$#00’) should be returned.
   */
  if (!ptr)
    buf[1] = 0;
  return send_or_fail(gdb);
}


int gdb_event(gdb_t * gdb, int vector, void * emu)
{
  int  pc, sr, st;
  char irqname[32], fctname[128];

  assert(gdb);
  assert(emu);
  assert(!gdb->emu || emu == gdb->emu);

  /* assert failed when interrupt by keyboard (RUN_STOP) */
  assert(gdb->run == RUN_CONT);

  errno = 0;

  switch ( recv_nowait(gdb, 0) ) {
  case -1:
    return -1;
  case 1:
    if (gdb->buf[gdb->blen-1] == 3) {
      /* gdb interrupt special char */
      --gdb->blen;
      SIGNAL(STOP,SIGVAL_INT,"interrupt by gdb");
      goto skip;
    }
  }

  /* Escape this as fast as possible */
  if (vector == HWTRACE_VECTOR) {
    if (!gdb->step || --gdb->step)
      return gdb->run;
    gdb->sigval = SIGVAL_TRAP;
    SIGNAL(STOP,gdb->sigval,"trace");
  }

  gdb->emu = emu;
  emu68_exception_name(vector,irqname);
  fctname[0] = 0;
  st = gdb->emu->status;
  if (vector < 0x100) {
    /***********************************************************************
     * 68k exceptions
     */
    int sigval = SIGVAL_0;

#if 0
    sr = Wpeek(gdb->emu, gdb->emu->reg.a[7]);
    pc = Lpeek(gdb->emu, gdb->emu->reg.a[7]+2);
#else
    sr = gdb->emu->inst_sr;
    pc = gdb->emu->inst_pc;
#endif

    switch (vector) {
    case BUSERR_VECTOR: case ADRERR_VECTOR:
      sigval = SIGVAL_BUS; break;
    case ILLEGAL_VECTOR:
      sigval = SIGVAL_ILL; break;
    case DIVIDE_VECTOR: case TRAPV_VECTOR:
      sigval  = SIGVAL_FPE; break;
    case LINEA_VECTOR: case LINEF_VECTOR: case CHK_VECTOR:
      sigval = SIGVAL_EMT; break;
    case TRACE_VECTOR:
      sigval = SIGVAL_TRAP; break;
    case PRIVV_VECTOR:
      sigval = SIGVAL_SEGV; break;

      /* $$$ FIXME: Timer interruptions: not always right as it
       * depends on MFP VR register bits 4-7
       */
    case 0X134 >> 2: /* Timer-A */
      strcpy(fctname," timer-A");
      if ( gdb->opt.ontimer_break & 1) {
        gdb->opt.ontimer_break = (gdb->opt.ontimer_break & ~1)
          | ((gdb->opt.ontimer_break>>4) & 1);
        sigval = SIGVAL_TRAP;
      }
      break;
    case 0X120 >> 2: /* Timer-B */
      strcpy(fctname," timer-B");
      if ( gdb->opt.ontimer_break & 2) {
        gdb->opt.ontimer_break = (gdb->opt.ontimer_break & ~2)
          | ((gdb->opt.ontimer_break>>4) & 2);
        sigval = SIGVAL_TRAP;
      }
      break;
    case 0X114 >> 2: /* Timer-C */
      strcpy(fctname," timer-C");
      if ( gdb->opt.ontimer_break & 4) {
        gdb->opt.ontimer_break = (gdb->opt.ontimer_break & ~4)
          | ((gdb->opt.ontimer_break>>4) & 4);
        sigval = SIGVAL_TRAP;
      }
      break;
    case 0X110 >> 2: /* Timer-D */
      strcpy(fctname," timer-D");
      if ( gdb->opt.ontimer_break & 8) {
        gdb->opt.ontimer_break = (gdb->opt.ontimer_break & ~8)
          | ((gdb->opt.ontimer_break>>4) & 8);
        sigval = SIGVAL_TRAP;
      }
      break;

    case SPURIOUS_VECTOR:
    default:
      if  (vector >= TRAP_VECTOR(0) && vector <= TRAP_VECTOR(15)) {
        sigval = decode_trap(fctname, vector-TRAP_VECTOR(0), gdb->emu);
      } else
        sigval = SIGVAL_TRAP;
    }
    gdb->sigval = sigval;
    if (sigval != SIGVAL_0) {
      SIGNAL(STOP,sigval,"68k exception");
    }
  } else {
    /***********************************************************************
     * Private exceptions
     */
    sr = gdb->emu->inst_sr;
    pc = gdb->emu->inst_pc;

    if (vector < HWTRACE_VECTOR) {
      /* Got a breakpoint ! */
      gdb->sigval = SIGVAL_TRAP;
      SIGNAL(STOP,gdb->sigval,"breakpoint");
    } else if (vector == HWINIT_VECTOR) {
      msgnot("%s\n","68K has been reset\n");
      STATUS(IDLE,IDLE,"Starting");
    } else if (vector == HWHALT_VECTOR) {
      gdb->sigval = SIGVAL_ABRT;
      STATUS(EXIT, HALT, "halted");
    } else if (vector == HWSTOP_VECTOR) {
      if ( (sr & 0x3F00) == 0x2F00 ) {
        const int num = sr & 0xFF;
        /* non-catch exception !!! */
        strcpy(irqname,"NC-");
        emu68_exception_name(num,irqname+3);
        pc = Lpeek(gdb->emu, gdb->emu->reg.a[7]+2);
        msgnot("non-catch exception #%d (%s) from 0x%X\n",
               num, irqname, pc);
      } else {
        sprintf (irqname,"stop-#%04x",sr);
        if ((sr & 0x0700) == 0x0700 && !(gdb->emu->inst_sr & SR_T)) {
          /* Stop a IPL 7, trace mode not set. It's a lock */
          msgnot("stop #0x%04x processor will not quit stop mode !\n", sr);
        }
        gdb->sigval = SIGVAL_STOP;
        SIGNAL(STOP, gdb->sigval, "stopped");
      }
    } else if (vector == HWINSTOV_VECTOR) {
      msgnot("catch instruction overflow @$%06x\n", pc);
      gdb->emu->status = EMU68_NRM;
      gdb->sigval = SIGVAL_TRAP;
      SIGNAL(STOP,gdb->sigval,"instruction-overflow");
    } else if (vector != HWTRACE_VECTOR) {
      /* Unhandled special exception */
      gdb->sigval = SIGVAL_0;
      SIGNAL(STOP,gdb->sigval,"private exception");
    }
  }

  if (gdb->run != RUN_CONT) {
    u8 * memptr = emu68_memptr(gdb->emu,0,0);
    const addr68_t adr = gdb->emu->reg.a[7] & gdb->emu->memmsk;
    const addr68_t bot = gdb->emu->memmsk+1-16;
    char line[16 * 3];
    int i,j;

    msgnot("68k exception in <%s> %s\n"
           "  vector : %02x (%d)\n"
           "  type   : %s%s\n"
           "  from pc:%08x sr:%04x\n"
           "       pc:%08x sr:%04x\n"
           "       d0:%08x d1:%08x d2:%08x d3:%08x\n"
           "       d4:%08x d5:%08x d6:%08x d7:%08x\n"
           "       a0:%08x a1:%08x a2:%08x a3:%08x\n"
           "       a4:%08x a5:%08x a6:%08x a7:%08x\n",
           gdb->emu->name,emu68_status_name(st),
           vector, gdb->sigval,
           irqname, fctname,
           pc, sr,
           gdb->emu->reg.pc, gdb->emu->reg.sr,
           gdb->emu->reg.d[0], gdb->emu->reg.d[1],
           gdb->emu->reg.d[2], gdb->emu->reg.d[3],
           gdb->emu->reg.d[4], gdb->emu->reg.d[5],
           gdb->emu->reg.d[6], gdb->emu->reg.d[7],
           gdb->emu->reg.a[0], gdb->emu->reg.a[1],
           gdb->emu->reg.a[2], gdb->emu->reg.a[3],
           gdb->emu->reg.a[4], gdb->emu->reg.a[5],
           gdb->emu->reg.a[6], gdb->emu->reg.a[7]);

    /* dump stack */
    line[0] = 0;
    for (i = 0, j = 0; i<256 && adr+i < bot; ++i) {
      int v = memptr[adr+i];

      line[j+0] = thex[v >> 4];
      line[j+1] = thex[v & 15];
      if (j != 3*15) {
        line[j+2] = '-';
        j += 3;
      } else {
        line[j+2] = 0;
        msgnot(" %-6x: %s\n", (unsigned)adr + (i & ~15), line);
        j = 0;
      }
    }
    if (j) {
      line[j-1] = 0;
      msgnot(" %-6x: %s\n", (unsigned)adr + (i & ~15), line);
    }
  }


skip:
  for (;;) {
    switch (gdb->run) {

    case RUN_STOP:
      if (send_reply(gdb, 'S', gdb->code) < 0) {
        msgerr("gdb-stub: failed send reply *STOP* (%d) \"%s\"\n",
               gdb->code, gdb->msg);
        goto exit;
      }
      STATUS(IDLE,IDLE,"waiting for gdb commands");
      msgnot("waiting for gdb commands\n");
    case RUN_IDLE:
      if (rsp_event(gdb) < 0) {
        msgerr("gdb-stub: failed to handle event *IDLE* (%d) \"%s\"\n",
               gdb->code, gdb->msg);
        goto exit;
      }
      break;

    case RUN_SKIP:
      if (gdb->code == CODE_DETACH)
        goto exit;
    case RUN_EXIT:
      send_reply(gdb, 'X', gdb->code);
    case RUN_CONT:
      goto exit;

    default:
      assert(!"unexpected run status");
    }
  }

exit:
  return gdb->run;
}


void gdb_destroy(gdb_t * gdb)
{
  if (gdb) {

    if (gdb->fd != -1)
      close(gdb->fd);

    if (gdb->server->fd != -1) {
      close(gdb->server->fd);
      gdb->server->fd = -1;
    }

    free(gdb);
  }
}


gdb_t * gdb_create(void)
{
  gdb_t * gdb = calloc(sizeof(*gdb),1);

  gdb_get_conf(0,0);                    /* trick: load default */

  if (gdb) {
    gdb->fd     = -1;
    gdb->server = &serverinfo;
    if (server_tcp(gdb->server, 1) < 0)
      goto error;
    gdb->bmax = sizeof(gdb->buf) - 1;

    STATUS(CONT,IDLE,"started");
    gdb->step = 1;

    /* Make it passive. */
    if (listen(gdb->server->fd,1)) {
      msgerr("listen -- %s\n", strerror(errno));
      goto error;
    }
    gdb->msg = "waiting for connection";

    /* Wait for a client to connect */
    msginf("waiting for gdb to connect:\n"
           "       *** Try gdb command 'target remote %d.%d.%d.%d:%d'\n",
           (gdb->server->addr >> 24) & 255,
           (gdb->server->addr >> 16) & 255,
           (gdb->server->addr >>  8) & 255,
           (gdb->server->addr >>  0) & 255,
           gdb->server->port);
    gdb->fd = accept(gdb->server->fd , 0, 0);
    /* Don't need that socket anymore */
    close(gdb->server->fd);
    gdb->server->fd = -1;

    if (gdb->fd == -1)
      goto error;

    /* $$$ TEMP: options need to be set with gdb command
       ATM breaks on all timers but C (system timer)
    */
    gdb->opt.ontimer_break = (uint8_t)~0x44;
    /* $$$ only once */
    gdb->opt.ontimer_break = (uint8_t)~0xf4;
    gdb->msg = "ready";
    TRACE("gdb-stub: %s\n",gdb->msg);
  }
  return gdb;

error:
  gdb_destroy(gdb);
  return 0;
}


int gdb_conf(char * uri)
{
  int ret = 0;
  uint16_t port = default_port;
  uint32_t addr = default_addr;
  /* struct in_addr ia; */

  if (uri) {
    char * s = strchr(uri,':');

    if (s) {
      unsigned long p;
      *s = 0;
      p = strtoul(s+1,0,10);
      if (!p || p > 0xffff) {
        p = 0;
        msgerr("%s -- invalid port number\n",s+1);
        ret = -2;
      }
      port = p;
    }

    /* convert host part */
    if (!*uri || !strcmp(uri,"*")) {
      addr = ntohl(INADDR_ANY);
    } else {
      struct hostent *he = gethostbyname(uri);
      if (he) {
        addr = ntohl(((struct in_addr *)he->h_addr)->s_addr);
      } else {
        msgerr("%s -- '%s'\n", uri, sock_error());
        ret = -1;
      }
    }
  }

  if (!ret) {
    serverinfo.addr = addr;
    serverinfo.port = port;
  }
  /* ia.s_addr = htonl(serverinfo.addr); */

  return ret;
}

void gdb_get_conf(unsigned char * addr, int * port)
{
  if (!serverinfo.port || serverinfo.addr == INADDR_NONE)
    gdb_conf(0);

  if (port)
    *port = serverinfo.port;

  if (addr) {
    addr[0] = serverinfo.addr >> 24;
    addr[1] = serverinfo.addr >> 16;
    addr[2] = serverinfo.addr >>  8;
    addr[3] = serverinfo.addr;
  }
}

int gdb_error(gdb_t * gdb, const char ** errp)
{
  int ret = -1;

  if (gdb) {
    ret = gdb->code;
    if (errp)
      *errp = gdb->msg;
  }
  return ret;
}
