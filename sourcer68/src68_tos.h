/**
 * @ingroup  sourcer68_prg
 * @file     src68_tos.h
 * @author   Benjamin Gerard
 * @date     2015-03-18
 * @brief    Atari TOS executable format.
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef SOURCER68_TOS_H
#define SOURCER68_TOS_H

#include "src68_def.h"

/**
 * TOS executable file structure.
 */
struct tosfile {
  uint8_t bras[2];                      /* 00 60 1a */
  uint8_t txt_sz[4];                    /* 02 */
  uint8_t dat_sz[4];                    /* 06 */
  uint8_t bss_sz[4];                    /* 0A */
  uint8_t sym_sz[4];                    /* 0E */
  uint8_t mint[4];                      /* 12 "MiNT" 0x4d694e54 */
  uint8_t flags[4];                     /* 16 Atari special flags */
  uint8_t noreloc[2];                   /* 18 True if absolute */
  /* 1C TEXT section starts here.  in case of MiNT it's a sequance of
   *    m68k instructions to jump to the real entry point
   *    [0x203a001a4efb08fa]
   */
  uint8_t text[8];
};
typedef struct tosfile tosfile_t;

enum {
  TOS_HEADER_SIZE = 28,
  TOS_MINT_SIZE = 256
};

/**
 * TOS symbols (aka DRI symbols).
 */
struct tossymb {
  char    name[8];
  uint8_t type[2];
  uint8_t addr[4];
};
typedef struct tossymb tossymb_t;

/**
 * Extended symbols
 */
typedef struct {
  char   name[24];
  uint_t type;
  uint_t addr;
} tosxymb_t;

/**
 * TOS symbol type.
 */
enum {
  TOS_SYMB_UNDF   = 0x0000,

  TOS_SYMB_AMASK  = 0xF000,
  TOS_SYMB_DEF    = 0x8000,
  TOS_SYMB_EQU    = 0x4000,
  TOS_SYMB_GLOBAL = 0x2000,
  TOS_SYMB_REG    = 0x1000,

  TOS_SYMB_BMASK  = 0x0F00,
  TOS_SYMB_XREF   = 0x0800,
  TOS_SYMB_DATA   = 0x0400,
  TOS_SYMB_TEXT   = 0x0200,
  TOS_SYMB_BSS    = 0x0100,

  TOS_SYMB_CMASK  = 0x00FF,
  TOS_SYMB_LNAM   = 0x0048, /* GST compatible long name.                 */
  TOS_SYMB_TFILE  = 0x0280, /* Text file corresponding to object module. */
  TOS_SYMB_TFARC  = 0x02C0, /* Text file archive (conflicts).            */
};

/**
 * Decoded TOS executable header.
 */
struct tosexec {
  void * user;                          /**< user data (cookie). */
  uint8_t * img;
  struct section {
    uint_t off,len;
  } txt, dat, bss, sym, rel;
  uint_t flags;
  struct {
    uint_t mint:1;
    uint_t abs:1;
  } has;
};
typedef struct tosexec tosexec_t;

/**
 * Tos relocation callback.
 */
typedef void (*toscall_f)(tosexec_t *, uint_t, uint_t);

/**
 * Error codes.
 */
enum {
  TOS_ERR_SUCCESS = 0,          /* no error */
  TOS_ERR_GENERIC,              /* generic error */
  TOS_ERR_NOT_TOS,              /* not a tos file */
  TOS_ERR_SEC_OOR,              /* section out of range */
  TOS_ERR_SEC_ODD,              /* section odd address */
  TOS_ERR_SYM_SIZ,              /* symbol section size not multiple */
  TOS_ERR_REL_OOR,              /* relocation addr out of range  */
  TOS_ERR_REL_ODD,              /* relocation addr is not even */
  TOS_ERR_REL_IDX,              /* relocation table ran out of data */
  TOS_ERR_INTERNAL = 66
};

/**
 * Reloc TOS loaded executable (aka TPA).
 *
 * @param  tosexec  decoded and loaded executable
 * @param  base     base or origin
 * @param  toscall  user callback for each relocated long word.
 *
 * @return error code
 * @retval 0 on success
 */
int tos_reloc(tosexec_t * tosexec, uint_t base, toscall_f toscall);

/**
 * Decode TOS header.
 *
 * @param  tosexec   decode and check TOS header
 * @param  tosfile   pointer to the TOS file
 * @param  filesize  size in byte of the tosfile
 *
 * @return error code
 * @retval 0 on success
 */
int tos_header(tosexec_t * tosexec, tosfile_t * tosfile, uint_t filesize);

/**
 * Walk tos symbols
 *
 * @return next walk
 * @retval 0  on end
 * @retval -1 on error
 */
int tos_symbol(tosxymb_t * xymb,
               tosexec_t * tosexec,  tosfile_t * tosfile, int walk);

#endif
