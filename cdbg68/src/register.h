#ifndef _CDBG68_REG_H_
#define _CDBG68_REG_H_

extern WINDOW * regwin;

int  reg_init(void);
void reg_destroy(void);
void reg_draw(void);
void reg_refresh(void);
void reg_set(void);
int  reg_key(int key);

#endif /* #ifndef _CDBG68_REG_H_ */
