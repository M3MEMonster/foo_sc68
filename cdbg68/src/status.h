#ifndef _CDBG68_STATUS_H_
#define _CDBG68_STATUS_H_

extern WINDOW * stawin;

int  sta_init(void);
void sta_destroy(void);
void sta_draw(void);
void sta_refresh(void);
void sta_set(const char * fmt, ...);
int  sta_key(int key);

#endif /* #ifndef _CDBG68_STATUS_H_ */
