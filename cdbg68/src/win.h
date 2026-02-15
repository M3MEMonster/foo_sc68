#ifndef _CDBG68_WIN_H_
#define _CDBG68_WIN_H_

#include "event.h"
#include "box.h"

typedef struct win_s win_t;

enum {
  BORDER_NONE   = 0,
  BORDER_SIMPLE = 1,
  BORDER_DOUBLE = 2,
};

struct win_s {
  WINDOW * win;                     /* curses window,.              */
  win_t  * root;                    /* root window (parent window). */
  win_t  * next;                    /* previous window (tab order). */
  win_t  * prev;

  struct {
    unsigned int is_focus : 1;          /* Window has keyboard focused. */
    unsigned int no_focus : 1;          /* Window do not accept focus.  */

    unsigned int border_l : 2;          /*  */
    unsigned int border_t : 2;          /*  */
    unsigned int border_r : 2;          /*  */
    unsigned int border_b : 2;          /*  */

  } flags; 

  struct {
    void      ( * create  )(win_t * win);
    void      ( * destroy )(win_t * win);
    void      ( * getbox  )(win_t * win, box_t * box);
    void      ( * setbox  )(win_t * win, box_t * box);
    void      ( * refresh )(win_t * win);
    void      ( * redraw  )(win_t * win);
    event_t * ( * event   )(win_t * win, event_t * event);
  } cb;
} win_t;

int  win_isroot(win_t * win);
void win_getpos(win_t * win, int * p_x, int * p_y);
void win_getdim(win_t * win, int * p_w, int * p_h);
void win_setpos(win_t * win, int x, int y);
void win_setdim(win_t * win, int w, int h);
void win_refresh(win_t * win);
void win_redraw(win_t * win);

#endif /* #ifndef _CDBG68_WIN_H_ */
