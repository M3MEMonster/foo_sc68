/**
 * @ingroup  sourcer68_prg
 * @file     src68_mbk.h
 * @author   Benjamin Gerard
 * @date     2015-03-18
 * @brief    memory block.
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef SOURCER68_MBK_H
#define SOURCER68_MBK_H

#include "src68_def.h"

/**
 * @anchor sourcer68_mib_value
 */
enum {
  /* Both Even and odd address lines. */
  MIB_READ  = 1,                    /**< Read by disassembler. */
  MIB_ADDR  = 2,                    /**< Symbol/address.       */
  MIB_BYTE  = 4,                    /**< Byte access.          */

  MIB_BOTH  = (MIB_BYTE*2)-1,       /**< Mask previous bits.   */
  MIB_BOTH_BITS = 3,                /**< Common bits.          */

  /* Even address only. */
  MIB_WORD  = MIB_BYTE*2,           /**< word access.             */
  MIB_LONG  = MIB_WORD*2,           /**< Long access.             */
  MIB_ENTRY = 32,                   /**< Disassembly entry point. */
  MIB_WALK  = 64,                   /**< Walked.                  */
  MIB_EXEC  = 128,                  /**< Valid instruction.       */
  MIB_DATA  = 256,                  /**< Data ( > Exec ).         */
  MIB_RELOC = 512,                  /**< Long has been relocated. */

  MIB_OPSZ_BIT = 10,                /**< 3 bits word opcode size. */
  MIB_OPSZ_MSK = 7<<MIB_OPSZ_BIT,   /**< correspoding bit mask.   */

  MIB_ALL   = 0xFFFF>>MIB_BOTH_BITS, /**< Mask for real bits only.  */

  MIB_ODD   = MIB_ALL+1,           /**< Set by odd address reply. */
  MIB_SET   = MIB_ODD*2            /**< Set on success reply.     */
};

typedef struct {
  uint_t   org;                      /**< origin of the memory block. */
  uint_t   len;                      /**< length of the memory block. */
  uint8_t *mem;                      /**< memory data bits. */
  uint8_t *_mib;                     /**< memory info bits. */
  uint8_t  buf[1];
} mbk_t;

/**
 * Create a new memory block.
 *
 * @param  org  origin address of the memory block.
 * @param  len  length in byte of the memory block.
 * @return pointer to a memory block descriptor
 */
mbk_t * mbk_new(uint_t org, uint_t len);

/**
 * Destroy a memory block.
 *
 * @param  mbk  memory block descriptor
 */
void mbk_del(mbk_t * mbk);

/**
 * Test if an address is in block range.
 *
 * @param  mbk  memory block descriptor.
 * @param  adr  address
 * @retval  0   address is not in block range
 * @retval  1   address is in block range
 */
static inline
int mbk_ismyaddress(const mbk_t * mbk, const uint_t adr) {
  return (adr - mbk->org < mbk->len);
}

/**
 * Test if an offset is in block range.
 *
 * @param  mbk  memory block descriptor.
 * @param  off  address offset
 * @retval  0   address is not in block range
 * @retval  1   address is in block range
 */
static inline
int mbk_ismyoffset(const mbk_t * mbk, const uint_t off) {
  return (off < mbk->len);
}

/**
 * Get memory info at that address.
 *
 * @param  mbk  memory block descriptor.
 * @param  adr  address
 * @return MIB_SET|mib-value (MIB_SET ensure the result is not 0)
 * @retval 0 on error (out of range)
 * @see sourcer68_mib_value
 */
int mbk_getmib(const mbk_t * mbk, uint_t adr);

/**
 * Clear and set memory info at that address.
 *
 * @param  mbk  memory block descriptor.
 * @param  adr  address
 * @param  clr  bits to clear
 * @param  set  bits to set
 * @return MIB_SET | updated bits (xor).
 * @retval 0 on error (out of range)
 * @see sourcer68_mib_value
 */
int mbk_setmib(const mbk_t * mbk, uint_t adr, int clr, int set);

/**
 * Get a string description of mib value.
 */
const char * mbk_mibstr(int mib, char * str);

static inline
/**
 * Read a 68k memory long.
 *
 * @param  mbk  memory block descriptor.
 * @param  adr  address
 * @return unsigned 32-bit long value.
 */
uint_t mbk_long(const mbk_t * mbk, uint_t adr)
{
  assert (!(adr&1));
  adr -= mbk->org;
  assert (adr < mbk->len && adr+3 < mbk->len);
  return (mbk->buf[adr]<<24) | (mbk->buf[adr+1]<<16)
    | (mbk->buf[adr+2]<<8) | mbk->buf[adr+3];
}

static inline
/**
 * Read a 68k memory word.
 *
 * @param  mbk  memory block descriptor.
 * @param  adr  address
 * @return unsigned 16-bit word value.
 */
uint_t mbk_word(const mbk_t * mbk, uint_t adr)
{
  assert (!(adr&1));
  adr -= mbk->org;
  assert (adr < mbk->len && adr+1 < mbk->len);
  return (mbk->buf[adr]<<8) | mbk->buf[adr+1];
}

static inline
/**
 * Read a 68k memory word.
 *
 * @param  mbk  memory block descriptor.
 * @param  adr  address
 * @return unsigned byte value.
 */
uint_t mbk_byte(const mbk_t * mbk, uint_t adr)
{
  adr -= mbk->org;
  assert (adr < mbk->len);
  return mbk->buf[adr];
}

#endif
