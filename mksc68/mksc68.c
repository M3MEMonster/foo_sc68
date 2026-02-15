/*
 * @file    mksc68.c
 * @brief   sc68 maker program
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
# include "config.h"
#endif

#include "mksc68_cli.h"
#include "mksc68_cmd.h"
#include "mksc68_dsk.h"
#include "mksc68_eva.h"
#include "mksc68_msg.h"
#include "mksc68_opt.h"
#include "mksc68_tag.h"
#include "mksc68_gdb.h"

#include <sc68/sc68.h>
#include <sc68/file68_opt.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif

int is_interactive   = 0;               /* running in a terminal? */
int no_readline      = 0;               /* disable readline       */
int exit_on_fail     = 0;               /* exit on command fail   */

static int exit_code;                   /* last command exit code */
static int exit_flag;                   /* break interpreter loop */

static void print_option(void * data,
                         const char * option,
                         const char * envvar,
                         const char * values,
                         const char * desc)
{
  printf("  %s", option);
  if (values && *values)
    printf("=%s", values);
  printf("\n"
         "    %-16s %c%s\n", envvar,
         (desc[0]>='a' && desc[0]<='z') ? desc[0]-'a'+'A' : desc[0],
         desc+1);
}

static void cmd_list(const cmd_t * cmd, int tabs);

static int print_usage(int more)
{
  puts
    ("Usage: " PACKAGE_NAME " [Options] [--] [URI ...]\n"
     "\n"
     "  sc68 file toolbox - to create or modify sc68 files\n"
     "\n"
     "Options:\n"
     "  -h --help           Display this message and exit (-hh for more)\n"
     "  -V --version        Display mksc68 version x.y.z and licence\n"
     "  -v --verbose        Increase verbosity level\n"
     "  -q --quiet          Decrease verbosity level\n"
     "  -x --fail-exit      Exit if a command fails\n"
     "  -n --no-readline    Don't use readline in interactive mode\n"
     "  -i --interactive    Force interactive mode (default autodetect tty)\n"
      );

  if (more > 1) {
    const cmd_t * cmd;
    puts
      ("sc68-options:");
    option68_help(stdout,print_option);
    puts
      ("");

    puts
      ("Commands:");
    for (cmd = cmd_lst(); cmd; cmd = cmd->nxt) {
      cmd_list(cmd,2);
      if (more > 2) {
        char buf[767], *s;
        const int max = sizeof(buf) - 1;
        strncpy(buf, cmd->hlp, max); buf[max] = 0;
        /* puts */
        /*   (""); */
        for (s = strtok(buf,"\n"); s; s=strtok(0,"\n")) {
          printf("  %c %s\n",more>3?' ':'|',s);
        }
        puts
          ("");
      }
    }
    puts
      ("");
  }

  puts
    ("Copyright (c) 1998-2015 Benjamin Gerard\n"
     "Visit <" PACKAGE_URL ">\n"
     "Report bugs to <" PACKAGE_BUGREPORT ">");
  return 1;
}

/* Display version number. */
static int print_version(void)
{
  puts
    (PACKAGE_STRING "\n"
     "\n"
     "Copyright (c) 1998-2015 Benjamin Gerard.\n"
     "License GPLv3+ or later <http://gnu.org/licenses/gpl.html>\n"
     "This is free software: you are free to change and redistribute it.\n"
     "There is NO WARRANTY, to the extent permitted by law.\n"
     "\n"
     "Written by Benjamin Gerard <" PACKAGE_BUGREPORT ">"
      );
  return 0;
}

static void cmd_list(const cmd_t * cmd, int tabs)
{
  int coml = 8, coma = 3, comu = 25;
  printf("%*s%-*s %-*s %-*s : %s.\n",
         tabs, "",
         coml, cmd->com,
         coma, cmd->alt?cmd->alt:"",
         comu, cmd->use?cmd->use:"",
         cmd->des);
}

static void cmd_usage(const cmd_t * cmd)
{
  int nl = 1, tab = 3;
  char *s;
  printf("Usage: %s %s\n", cmd->com, cmd->use?cmd->use:"");
  fputc('\n',stdout);
  for (s = cmd->hlp; *s; ++s) {
    if (*s == '\n')
      nl = 1;
    else if (nl) {
      nl = 0;
      fprintf(stdout, "%*s", tab,"");
    }
    fputc(*s,stdout);
  }
  if (!nl) fputc('\n',stdout);
  fputc('\n',stdout);
}


int eval(char * expr, int * status)
{
  int ret = 0, dummy;
  if (!status)
    status = &dummy;
  errno = 0;
  ret = strtol(expr,0,0);
  *status = -!!errno;
  return ret;
}

static int run_exit(cmd_t * me, int argc, char ** argv)
{
  int code = exit_code;

  if (argc > 1) {
    int n, status;
    n = eval(argv[1], &status);
    if ( ! status ) {
      code = n;
    }
  }
  exit_flag = 1;
  msgdbg("exit: exit with %d\n", code);
  return code;
}

static int run_help(cmd_t * me, int argc, char ** argv)
{
  int code = 0;
  cmd_t * cmd;

  if (argc < 2) {
    /* list all commands */
    for (cmd = cmd_lst(); cmd; cmd = cmd->nxt) {
      cmd_list(cmd,0);
    }
  } else {
    int i;
    for (i=1; i<argc; ++i) {
      for (cmd = cmd_lst();
           cmd && strcmp(argv[i],cmd->com) &&
             (!cmd->alt || strcmp(argv[i],cmd->alt));
           cmd = cmd->nxt)
        ;
      if (cmd) {
        cmd_usage(cmd);
      } else {
        ++code;
        msgerr("unknown command (%s)\n",argv[i]);
      }
    }
  }
  return code;
}

void help(char *com)
{
  char * argv[2] = { "help", com };
  run_help(0, 2, argv);
}

static int run_echo(cmd_t * me, int argc, char ** argv)
{
  int i;
  errno = 0;
  /* FIXME: which print function should I use ?
   * printf() or msginf()
   */
  for (i=1; i<argc; ++i) {
    const char * sep = " ";
    if (i==argc-1) {
      int len = strlen(argv[i]);
      sep = (len > 0 && argv[i][len-1] == '\n') ? "" : "\n";
    }
    printf("%s%s",argv[i], sep);
  }
  if (!errno)
    fflush(stdout);
  return -!!errno;
}

static int run_error(cmd_t * me, int argc, char ** argv)
{
  int i;
  /* FIXME: which print function should I use ?
   * printf() or msgerr()
   */
  for (i=1; i<argc; ++i)
    msgerr("%s%s",argv[i], i==argc-1?"\n":" ");
  fflush(stderr);
  return exit_code;
}

extern cmd_t cmd_new, cmd_load, cmd_play, cmd_stop, cmd_msg, cmd_tag;
extern cmd_t cmd_time, cmd_save, cmd_info, cmd_gdb, cmd_extract;
static cmd_t
cmd_exit = {
  run_exit, "exit",  "x", "[exit-code]", "exit command interpreter" },
cmd_help = {
  run_help, "help",  "?", "[cmd ...]", "print command(s) usage",
  "The `help' command prints command list or usage.\n"
  "Without argument it prints the command list with a short description.\n"
  "Else prints a complete usage for all given commands.\n"
  "Returns the number of error (unknown commands).\n"
},
cmd_echo = {
  run_echo, "echo",  "p", "[...]",       "print informative message",          },
cmd_error = {
  run_error, "error",   0, "[...]",      "print error message"     };

static cmd_t *commands[] = {
  &cmd_msg,
  &cmd_error,
  &cmd_echo,

  &cmd_time,
  &cmd_stop,
  &cmd_play,
  &cmd_gdb,

  &cmd_new,
  &cmd_load,
  &cmd_save,
  &cmd_extract,

  &cmd_info,
  &cmd_tag,

  &cmd_exit,
  &cmd_help,

  0
};

static int add_commands(void)
{
  int i, err;
  for (err=i=0; commands[i] ; ++i)
    err += !!cmd_add(commands[i]);
  return -!!err;
}


#if 0
/* Display various track info depending on level
 *  level 1 : track No, music name
 *  level 2 : author, composer
 *  level 4 : IO-chip, replay rate , cycles/pass
 *  level 8 : Load address, External replay
 */
static int trackinfo(int trk, int infolevel)
{
  int err;
  music68_t *mus;

  if (err = checktrack(trk), err < 0) {
    return err;
  }
  if (err != trk) {
    return derr(-1,"track #%d out of range", trk+1);
  }
  mus = disk->mus + trk;

  /* level 1 : time, music name */
  if (infolevel & 1) {
    dinf("%c%-8s %-40s",
         (trk==track) ? '*' : ' ',
         strtime68(0, trk+1, mus->time_ms/1000),
         mus->name);
  }

  /* level 2 : authors, composer */
  if (infolevel & 2) {
    if (!strcmp68(mus->author,mus->composer)) {
      dinf(" (%s)",mus->author);
    } else {
      dinf(" (%s/%s)",mus->author,mus->composer);
    }
  }

  if (infolevel & 3) {
    dinf("\n");
  }

  /* level 4 : IO-chip, replay rate , cycles/pass */
  if (infolevel&4) {
    if (mus->hwflags.bit.ym) {
      dinf("  YM-2149 (Atari STF/E sound chip) required\n");
    }
    if (mus->hwflags.bit.ste) {
      dinf("  MicroWire (Atari STE sound chip) required\n");
    }
    if (mus->hwflags.bit.amiga) {
      dinf("  Paula (Amiga 1000/500/1200... sound chip) required\n");
    }
    dinf("  replay rate: %d Hz (%u cpf)\n",
         mus->frq, cycle_per_frame(mus->frq));
  }

  /* level 8 : Load address, External replay */
  if (infolevel&8) {
    if (!mus->replay) {
      if (mus->a0 != SC68_LOADADDR) {
        dinf("  Absolute position built-in replay @ $%X-%X (%d bytes)\n",
             mus->a0,mus->a0+mus->datasz-1,mus->datasz);
      } else {
        dinf("  Relocatable built-in replay (%d bytes)\n",
             mus->datasz);
      }
    } else {
      if (mus->a0 != SC68_LOADADDR) {
        dinf("  Absolute external replay <%s> @ $%X-%X (%d bytes)\n",
             mus->replay,
             mus->a0,mus->a0+mus->datasz-1,mus->datasz);
      } else {
        dinf("  Relocatable external replay <%s> (%d bytes)\n",
             mus->replay,
             mus->datasz);
      }
    }
  }
  return 0;
}

/* compute disk total time */
static void totaltime(void)
{
  if (disk) {
    int i;
    for (i=0, disk->time_ms=0; i<disk->nb_six; ++i) {
      disk->time_ms += disk->mus[i].time_ms;
    }
    dmsg("totaltime: %ums for %d track\n", disk->time_ms, disk->nb_six);
  } else {
    dmsg("totaltime: no disk\n");
  }
}


/******************
 * LOAD A NEW DISK *
 ******************/
static int loaddisk(char *fname, int mergemode)
{
  disk68_t *newdisk = 0;

  dmsg("loaddisk: %s '%s'\n",
       mergemode ? "merging" : "loading", fname);

  if (!fname) {
    return derr(-1, "no disk to load");
  }

  /* Desactif merge if no disk */
  mergemode &= -( disk != NULL );

  /* Load SC68 file */
  if (newdisk = file68_load_url(fname), newdisk == NULL) {
    return derr(-1, "failed to load '%s'");
  }

  /* If not merge mode : Free previous disk */
  if (!mergemode) {
    killdisk();
    disk = newdisk;
    lastload_fname = strdup68(fname);
  } else {
    int i,j,nb = disk->nb_six + newdisk->nb_six;

    /* Clear 68K memory */
    memset(emu68->mem,0,emu68->memmsk+1);
    memset(emu68->chk,0,emu68->memmsk+1);

    if (nb > SC68_MAX_TRACK) {
      warn("too many musix; skipping %d track(s)\n",nb-SC68_MAX_TRACK);
      nb = SC68_MAX_TRACK;
    }
    for (i=disk->nb_six, j=0; i<nb; i++, j++) {
      disk->mus[i] = newdisk->mus[j];
      disk->nb_six = nb;
    }
    /* $$$ MEMORY LEAK HERE , can't free newdisk (in used) $$$ */
  }

  /* Auto transform very known names !!! */
  checknames();

  /* Set current track */
  if (settrack(disk->default_six)<0) {
    return derr(-1,"File corrupted : Default track out of disk bound.");
  }
  totaltime();

  return 0;
}

static int loaddata(char *fname, int d0, int a0, int frq, int flags,
                    char *mname, char *aname, char *rname)
{
  disk68_t * newdisk;
  istream68_t * is;
  int  flen,size;

  if (!fname) {
    return derr(-1,"NULL pointer as data file !!");
  }

  is = istream68_file_create(fname,1);
  if (!is) {
    return derr(-1,"Can't load data file : see previous");
  }
  if (istream68_open(is)) {
    istream68_destroy(is);
    return derr(-1,"Can't load data file : see previous");
  }
  flen = istream68_length(is);
  if (flen == -1) {
    istream68_destroy(is);
    return derr(-1,"Can't load data file : see previous");
  }

  printf("Load data file : %s (%d bytes)\n"
         " d0=%d a0=$%x frq=%d flags=%c%c%c (%x)\n",
         fname, flen, d0, a0, frq,
         'y'-((!!(flags&SC68_YM))<<5),
         's'-((!!(flags&SC68_STE))<<5),
         'a'-((!!(flags&SC68_AMIGA))<<5),
         flags);

  size = flen + sizeof(*newdisk);
  if (newdisk=(disk68_t *)alloc68(size), !newdisk) {
    istream68_destroy(is);
    return derr(-1,"Can't load data file : see previous");
  }
  memset(newdisk,0,size);

  if (istream68_read(is, newdisk->data, flen) != flen) {
    free68(newdisk);
    istream68_destroy(is);
    return derr(-1,"Can't load data file : see previous");
  }
  istream68_destroy(is);

  killdisk();
  disk = newdisk;

  /* At least 1 track for a valid disk */
  disk->nb_six    = 1;
  disk->mus[0].d0 = d0;
  disk->mus[0].a0 = a0;
  disk->mus[0].frq = frq;
  disk->mus[0].hwflags.all = 0;
  disk->mus[0].hwflags.bit.ym = !!(flags&SC68_YM);
  disk->mus[0].hwflags.bit.ste = !!(flags&SC68_STE);
  disk->mus[0].hwflags.bit.amiga = !!(flags&SC68_AMIGA);
  disk->mus[0].data = disk->data;
  disk->mus[0].datasz = flen;

  /* Name settup */
  disk->name = strdup68(mname);
  disk->mus[0].replay = strdup68(rname);
  disk->mus[0].author = strdup68(aname);
  disk->mus[0].composer = strdup68(disk->mus[0].author);

  /* Set current track */
  if (settrack(disk->default_six) < 0) {
    return derr(-1,"File corrupted : Default track out of disk bound.");
  }

  return 0;
}

/********************
 * SAVE CURRENT DISK *
 ********************/
static int savedisk(char *fname, int gzlevel)
{
  int err;
  if (err=checkdisk(), err<0) {
    return err;
  }
  if (!fname) {
    return derr(-1,"Missing filename");
  }
  if (err = checktrack(disk->default_six = track), err<0) {
    return err;
  }
  if (err=file68_save_url(fname, disk, gzlevel), !err) {
    printf("%s saved [gzip level:%d]\n",fname, gzlevel);
  }
  return err;
}

/* Kill previous external replay.
 * Load new one ( NULL is none ).
 */
static int setreplay(char *newname)
{
/*   istream68_t * is; */
  char * tmpname;

  /* Kill previous replay */
  free68(replayname);
  replayname = 0;

/*   free68(replay); */
/*   replay = 0; */
/*   replaysz = 0; */

  if (!newname) {
    return 0;
  }
  tmpname = strdup68(newname);
  if (!tmpname) {
    return derr(-1,"could not alloc replay name <%s>", newname);
  }

/*   is = rsc68_open(rsc68_replay, newname, 1, 0); */
/*   if (!is) { */
/*     free68(tmpname); */
/*     return derr(-1,"Could not find external replay <%s>", newname); */
/*   } */

/*   replay = load_external_replay(is, &replaysz); */
/*   istream68_destroy(is); */
/*   if (!replay) { */
/*     free68(tmpname); */
/*     derr(-1,"Failed to load external replay <%s>", newname); */
/*     return -1; */
/*   } */
  replayname = tmpname;
  return 0;
}

/* Set current track. */
static int settrack(int newtrack)
{
  int err;
  char *thisreplay;

  if (err=checktrack(newtrack), err<0) {
    return err;
  }

  track    = newtrack;
  mus      = disk->mus+track;
  playaddr = a0 = forced_a0 ? forced_a0 : mus->a0;
  time_ms  = mus->time_ms;
  memlow   = (u32)-1;
  memhi    = (u32)-1;
  if (mus->frq==0) mus->frq = 50;
  cycleperpass = cycle_per_frame(mus->frq);
  thisreplay = (forced_replay) ? forced_replay : mus->replay ;
  if (err=setreplay(thisreplay), err<0) {
    return derr(-1,"Failed loading track %d external replay <%s>\n",
                             track,thisreplay);
  }
  return 0;
}

static inline const char * ok_int(const int const err) {
  return !err ? "success" : "failure";
}
#endif

#if 0
/*  68K First time init;
 */
static int init68K(int size)
{
  int err;
  emu68_init_t emu68_parms;
  /*   io68_init_t io68_parms; */

  sc68_debug(sc68,"init_emu68() {\n");

  /* Initialize emu68 */
  sc68_debug(sc68,"init_emu68() : Initialize emu68 (68k emulator)\n");
  emu68_parms.alloc = sc68_alloc;
  emu68_parms.free  = sc68_free;
  err = emu68_init(&emu68_parms);
  sc68_debug(sc68,"init_emu68() : emu68 library init %s\n", ok_int(err));
  if (err) {
    goto error;
  }

  /* Initialize chipset */
  /*   sc68_debug(sc68,"init_emu68() : Initialise io68 (chipset)\n"); */
  /*   memset(&io68_parms, 0, sizeof(io68_parms)); */
  /*   err = io68_init(&io68_parms); */
  /*   sc68_debug(sc68,"init_emu68() : io68 library init %s\n", ok_int(err)); */

error:
  sc68_debug(sc68,"} init_emu68() => [%s]\n", ok_int(err));
  return err;
}
#endif

#if 0
static int prepare_reg68(void)
{
  int err;
  addr68_t A0=a0;

  if (err = checktrack(track), err < 0) {
    return err;
  }

  dinf("Prepare 68000:\n"
       " - Disk [%s]\n"
       " - Track #%02d [%s]\n"
       " - Replay [%s]\n"
       " - Frq:%uHz Cpf:%u\n"
       , disk->name, track+1, mus->name,
       replayname ? replayname : "built-in",
       mus->frq, cycleperpass);

  /* Must be call because of some _DEBUG test */
  emu68_reset(emu68);

  /* Clear 68K mem */
  emu68_memset(emu68, 0, 0, 0);
  emu68_poke(emu68,0,0x4e);
  emu68_poke(emu68,1,0x73);

  if (setreplay(mus->replay) < 0) {
    return derr(-1,"Failed loading external replay <%s>\n",mus->replay);
  }

  if (replay) {
    u32 data;
    if (emu68_memput(emu68,A0,replay,replaysz)<0) {
      derr(emu68_error_get());
      return
        derr(-1,"Failed copying external replay into 68K memory");
    }
    printf(" - External replay <%s> loaded @$%X-%X\n",
           replayname, A0, A0+replaysz-1);
    data = A0 + ((replaysz+1)&-2);
    /* Check memory overflow */

    if (EMU68_memvalid(data,mus->datasz+1024)) {
      EMU68error_get(); /* Pop error message !!! */
      data = A0 - ((mus->datasz+1025)&-2);
      if (data<0x1000) {
        return derr(-1,"No room for copying data into 68K memory");
      }
    }
    A0 = data;
  }

  /* Copy Data into 68K memory */
  if (EMU68_memput(A0,(u8*)mus->data,mus->datasz)) {
    return derr(EMU68error_get());
  }

  /* $$$ NOTE: Absolute/Indepenadnt test is broken */
  printf(" - %s position %s loaded @$%X-%X\n",
         (a0 == SC68_LOADADDR) ? "Independant" : "Absolute",
         replay ? "music data" : " music replay and data",
         A0,A0+mus->datasz-1);

  /* 68K registers init */
  REG68.pc   = playaddr;
  REG68.sr   = 0x2300;
  REG68.d[0] = mus->d0;
  REG68.d[1] = !mus->hwflags.bit.ste;
  REG68.d[2] = mus->datasz;
  REG68.a[0] = A0;
  REG68.a[7] = REG68.memsz-16;
  REG68.cycle = 0;
  memset(REG68.chk,0,REG68.memsz);

  printf("68000 Ready!\n");



#if 0
  u32 A0=a0;
  int err;
  io68_t *io;
  int i;

  printf("Prepare 68000:\n"
         " - Disk [%s]\n"
         " - Track #%02d [%s]\n"
         " - Replay [%s]\n"
         " - Frq:%uHz Cpf:%u\n"
         , disk->name, track+1, mus->name,
         replayname ? replayname : "built-in",
         mus->frq, cycleperpass);

  /* Must be call because of some _DEBUG test */
  emu68_reset(emu68);

  /* Clear 68 K mem */
  emu68_memset(emu68, 0, 0, 0);

  emu68_poke(emu68,0,0x4e);
  emu68_poke(emu68,1,0x73);
  emu68_poke(emu68,0x41a,0);         /* Zound Dragger */
  emu68_poke(emu68,0x41b,0x10);      /* Zound Dragger */
  emu68_poke(emu68,TRAP_VECTOR(14)+0,(u8) (TRAP_14_ADDR >> 24));
  emu68_poke(emu68,TRAP_VECTOR(14)+1,(u8) (TRAP_14_ADDR >> 16));
  emu68_poke(emu68,TRAP_VECTOR(14)+2,(u8) (TRAP_14_ADDR >>  8));
  emu68_poke(emu68,TRAP_VECTOR(14)+3,(u8) (TRAP_14_ADDR));
  emu68_memput(emu68, TRAP_14_ADDR, trap14, sizeof(trap14));

  /*$$$*/
  /*   EMU68ioplug(&paula_io); */
  /*   EMU68ioplug(&ym_io); */
  /*   EMU68ioplug(&mw_io); */
  /*   EMU68ioplug(&sh_io); */

  /* Display IO list */
  /*   for(i=0, io=REG68.iohead; io; io=io->next, i++) { */
  /*     printf(" - io #%d/%d : '%s' %x-%x\n", */
  /*         i+1,REG68.nio,io->name,io->addr_low,io->addr_high); */
  /*   } */

  if (setreplay(mus->replay) < 0) {
    return derr(-1,"Failed loading external replay <%s>\n",mus->replay);
  }

  if (replay) {
    u32 data;
    if (emu68_memput(emu68,A0,replay,replaysz)<0) {
      derr(emu68_error_get());
      return
        derr(-1,"Failed copying external replay into 68K memory");
    }
    printf(" - External replay <%s> loaded @$%X-%X\n",
           replayname, A0, A0+replaysz-1);
    data = A0 + ((replaysz+1)&-2);
    /* Check memory overflow */

    if (EMU68_memvalid(data,mus->datasz+1024)) {
      EMU68error_get(); /* Pop error message !!! */
      data = A0 - ((mus->datasz+1025)&-2);
      if (data<0x1000) {
        return derr(-1,"No room for copying data into 68K memory");
      }
    }
    A0 = data;
  }

  /* Copy Data into 68K memory */
  if (EMU68_memput(A0,(u8*)mus->data,mus->datasz)) {
    return derr(EMU68error_get());
  }

  /* $$$ NOTE: Absolute/Indepenadnt test is broken */
  printf(" - %s position %s loaded @$%X-%X\n",
         (a0 == SC68_LOADADDR) ? "Independant" : "Absolute",
         replay ? "music data" : " music replay and data",
         A0,A0+mus->datasz-1);

  /* 68K registers init */
  REG68.pc   = playaddr;
  REG68.sr   = 0x2300;
  REG68.d[0] = mus->d0;
  REG68.d[1] = !mus->hwflags.bit.ste;
  REG68.d[2] = mus->datasz;
  REG68.a[0] = A0;
  REG68.a[7] = REG68.memsz-16;
  REG68.cycle = 0;
  memset(REG68.chk,0,REG68.memsz);

  printf("68000 Ready!\n");
#endif
  return 0;
}

#endif

static char * prompt(void)
{
  char * ret;
  const char * dsktitle, * trktitle;
  int has = dsk_has_disk();
  int trk = dsk_trk_get_current();

  if (!has)
    dsktitle = 0;
  else if(dsktitle = tag_get(0,"title"), !dsktitle)
    dsktitle ="unamed";

  if (trk <= 0) {
    ret = dsktitle
      ? cli_prompt("(%s)>",dsktitle)
      : cli_prompt("mksc68>")
      ;
  } else {
    trktitle = tag_get(trk,"title");
    if (dsktitle && trktitle && strcmp(dsktitle,trktitle))
      ret = cli_prompt("(%02d - %s - %s)>", trk, dsktitle, trktitle);
    else
      ret = cli_prompt("(%02d - %s)>", trk, dsktitle ? dsktitle : trktitle);
  }

  return ret;
}


static int interactive(void)
{
  char * argx[32], ** argv = argx;
  int    argc;

  msgdbg("mksc68 is going %sinteractive\n",is_interactive?"":"non ");

  for(;;) {
    if (!is_interactive && feof(stdin)) {
      msgdbg("Exit on EOF\n");
      break;
    }
    prompt();
    argc = cli_read(argv, 32);
    if (argc <= 0 || argv[0][0] == '#')
      continue;

    if ( argv[0][0] == '~' || argv[0][0] == '=') {
      int test = (argv[0][0] == '~') ^ !!exit_code;
      argv[0] += 1;
      if (!argv[0][0]) {
        ++argv;
        if (!--argc)
          continue;
      }
      if (test)
        continue;
    }
    exit_code = cmd_run(argc, argv);
    if (exit_code && exit_on_fail) {
      msgdbg("Exit on fail (%s => %d)\n",argv[0],exit_code);
      break;
    }
    if (exit_flag) {
      msgdbg("Exit requested (%s => %d)\n",argv[0],exit_code);
      break;
    }
  }
  dsk_stop();

  return exit_code;
}

static int opt_vers, opt_help, opt_verb;

int main(int argc, char *argv[])
{
  int err = -1, i, ifd;
  sc68_init_t   init68;
  char exe_name[] = "mksc68"; /* not read-only or basename() might fault */

  static const opt_t longopts[] = {
    {"help",        0, 0, 'h'},
    {"version",     0, 0, 'V'},
    {"verbose",     0, 0, 'v'},
    {"quiet",       0, 0, 'q'},
    {"no-readline", 0, 0, 'n'},
    {"fail-exit",   0, 0, 'x'},
    {"interactive", 0, 0, 'i'},
    {0,0,0,0}
  };
  char shortopts[(sizeof(longopts)/sizeof(*longopts))*3];
  opt_create_short(shortopts,longopts);

  argv[0] = exe_name;

  /* Initialize sc68 api. */
  memset(&init68, 0, sizeof(init68));
  init68.argc = argc;
  init68.argv = argv;
  init68.msg_handler = msg_callback;
  if (sc68_init(&init68)) {
    return -1;
  }
  argc = init68.argc;
  argv = init68.argv;

  /* Set default interactive mode based on tty detection */
#if defined(HAVE__FILENO)
  ifd = _fileno(stdin);
#elif defined(HAVE_FILENO)
  ifd = fileno(stdin);
#elif defined(STDIN_FILENO)
  ifd = STDIN_FILENO;
#else
  ifd = -1;
#endif

#ifndef MYISATTY
# if defined(HAVE__ISATTY)
#  define MYISATTY(FD) _isatty(FD)
# elif defined(HAVE_ISATTY)
#  define MYISATTY(FD) isatty(FD)
# endif
#endif

  /* printf("isatty(%d)=%d\n", ifd, MYISATTY(ifd)); */

#ifdef MYISATTY
  if (ifd != -1 && MYISATTY(ifd) > 0)
    is_interactive = 1;
  else
    no_readline = 1;
#endif

  while (1) {
    int longindex;
    int val =
      opt_get(argc, argv, shortopts, longopts, &longindex);

    switch (val) {
    case  -1: break;                      /* Scan finish   */
    case 'h': opt_help++;       break;    /* --help        */
    case 'V': opt_vers = 1;     break;    /* --version     */
    case 'v': opt_verb++;       break;    /* --verbose     */
    case 'q': opt_verb--;       break;    /* --quiet       */
    case 'n': no_readline = 1;  break;    /* --no-readline */
    case 'x': exit_on_fail = 1;   break;  /* --exit-fail   */
    case 'i':                             /* --interactive   */
      is_interactive = 1;
      break;
    case '?':                       /* Unknown or missing parameter */
      goto error;
    default:
      msgerr("%s: unexpected getopt return value (%d)\n", argv[0], val);
      goto error;
    }
    if (val == -1) break;
  }
  i = optind;
  i = i;

  if (opt_help) {
    if (add_commands()) opt_help = 1;
    return print_usage(opt_help);
  }

  if (opt_vers) {
    return print_version();
  }

  if (!is_interactive)
    no_readline = 1;

  msg_init(opt_verb);
  if (err = add_commands(), err)
    goto error;

  for (; i<argc; ++i) {
    static char fix[] = "ld\0-a";
    char * v[3];
    v[0] = fix; v[1] = fix+3; v[2] = argv[i];
    err = cmd_run(3, v);
    if (err) goto error;
  }

  err = interactive();

error:
  sc68_shutdown();
  return err;
}
