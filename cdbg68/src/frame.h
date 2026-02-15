#ifndef _CDBG68_FRAME_H_
#define _CDBG68_FRAME_H_

struct frame_s {
  box_t outer;
  box_t sub[2];

};

enum {
  FRBORDER_NONE = 0,                    /* No border     */
  FRBORDER_L    = 1,                    /* Left border   */
  FRBORDER_T    = 2,                    /* Top border    */
  FRBORDER_R    = 4,                    /* Right border  */
  FRBORDER_B    = 8,                    /* Bottom border */
  FRBORDER_S    = 16                    /* Split border  */
  FRBORDER_ALL  = FRBORDER_L|FRBORDER_TFRBORDER_L|RFRBORDER_B|FRBORDER_S,

  FRSPLIT_V = 0,
  FRSPLIT_H = 256,
  FRSPLIT_PERCENT = 512,

};


int  frame_create(frame_t * fr, box_t box, int split, int flags);
void frame_destroy(frame_t * fr);

void frame_draw(frame_t * fr);
void frame_refresh(frame_t * fr);

#endif /* #ifndef _CDBG68_FRAME_H_ */
