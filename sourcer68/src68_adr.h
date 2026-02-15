/**
 * @ingroup  sourcer68_prg
 * @file     src68_adr.h
 * @author   Benjamin Gerard
 * @date     2015-03-18
 * @brief    address container
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef SOURCER68_ADR_H
#define SOURCER68_ADR_H

#include "src68_vec.h"

typedef struct {
  obj_t  obj;                     /**< interface (must be first). */
  uint_t adr;                     /**< address. */
} adr_t;

/**
 * Create an address container.
 *
 * @param  max  Initial maximum number of address
 *
 * @return address container.
 * @retval 0 on error
 */
vec_t * address_new(uint_t max);

/**
 * Create a new address object.
 *
 * @param adr   address value.
 *
 * @return address entry.
 * @retval 0 on error
 */
adr_t * addr_new(uint_t adr);

/**
 * Get address at index.
 *
 * @param adrs address container.
 * @param idx  index
 *
 * @return address entry
 * @retval 0 on error
 */
adr_t * addr_get(vec_t * adrs, int idx);

/**
 * Create and add an address entry.
 *
 * @param  adrs  address container.
 * @param  adr   address value to add.
 *
 * @return added relocation index
 * @retval -1 on error
 */
int addr_add(vec_t * adrs, uint_t adr);

/**
 * Find if an address is in used.
 *
 * @param  adrs  address container.
 * @param  adr   address to lookup.
 *
 * @retval 0 address does not exist
 * @retval 1 address exists
 */
int addr_exists(vec_t * adrs, uint_t adr);

#endif
