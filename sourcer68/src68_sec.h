/**
 * @ingroup  sourcer68_prg
 * @file     src68_sec.h
 * @author   Benjamin Gerard
 * @date     2015-03-18
 * @brief    program sections
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef SOURCER68_SEC_H
#define SOURCER68_SEC_H

#include "src68_vec.h"

/**
 * section flags. @anchor src68_section_flags
 */
enum {
  SECTION_N = 0,                 /**< no flag. */
  SECTION_X = 1<<1,              /**< section is executable (text). */
  SECTION_0 = 1<<2,              /**< section is zeroed (bss). */
};

/**
 * section object.
 */
typedef struct {
  obj_t  obj;                           /* must be first */
  char * name;                          /**< section name.  */
  uint_t addr;                          /**< section origin address. */
  uint_t size;                          /**< section size in byte. */
  uint_t flag;                          /**< @see src68_section_flags. */
} sec_t;

/**
 * Create a section container.
 *
 * @param  max  Initial maximum number of section
 * @retval 0 on error
 * @return section container.
 */
vec_t * sections_new(uint_t max);

/**
 * Add a new section. Adding a section might invalidated other section
 * index and/or values.
 *
 * @param  name  a name for the section (to be copied)
 * @param  addr  address of section start
 * @param  size  size of the section (in byte)
 * @param  flag  set of flags for the section.
 *
 * @return section
 * @retval 0 on error
 */
sec_t * section_new(const char * name, uint_t addr, uint_t size, uint_t flag);


/**
 * Get section at index.
 */
sec_t * section_get(vec_t * sections, int index);

/**
 * Create and add a section.
 *
 * @param  sections  the section container
 * @param  name  a name for the section (to be copied)
 * @param  addr  address of section start
 * @param  size  size of the section (in byte)
 * @param  flag  set of flags for the section.
 *
 * @return added section index
 * @retval -1 on error
 */
int section_add(vec_t * sections, const char * name,
                uint_t addr, uint_t size, uint_t flag);

#endif
