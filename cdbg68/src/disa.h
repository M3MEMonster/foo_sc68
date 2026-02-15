#ifndef _CDBG68_DISA_H_
#define _CDBG68_DISA_H_

#include <curses.h>

/** Disassembler window ID */
#define DISWIN 1

/** Disassembler curses window */
extern WINDOW * diswin;

/** Init dissassembler windows. */
int dis_init(void);

/** Destroy disassembler windows. */
void dis_destroy(void);

/** Draw window decoration. */
void dis_draw(void);

/** Refresh disassebler window content. */
void dis_refresh(void);

/** Set disassembler window address.
 *
 *  @param  address  new address (-1 for PC register).
 */
void dis_set(int address);

/** Get disassembler window address.
 *
 *  @return  current disassemble address.
 */
int dis_get(void);

/** Key handler.
 *
 *  @param  mod  key modifier.
 *  @param  key  key code.
 *
 *  @retval  0  key was used
 *  @retval  1  key not used
 */
int dis_key(int mod, int key);

#endif /* #ifndef _CDBG68_DISA_H_ */
