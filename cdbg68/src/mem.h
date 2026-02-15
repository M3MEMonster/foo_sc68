#ifndef _CDBG68_MEM_H_
#define _CDBG68_MEM_H_

#include <curses.h>

extern WINDOW * memwin;

int  mem_init(void);
void mem_destroy(void);
void mem_draw(void);
void mem_refresh(void);

void mem_set(int address);
int  mem_get(void);

int  mem_key(int key);


#endif /* #ifndef _CDBG68_MEM_H_ */
