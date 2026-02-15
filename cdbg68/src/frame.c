#include "frame.h"

int frame_create(frame_t * fr, box_t * box, int split, int flags)
{
  int err = -1;
  box_t inner;
  
  *fr->outer = *box;                    /* Copy the box */

  inner.w = box->x + box->w - !!(flags & FRBORDER_R);
  inner.h = box->y + box->h - !!(flags & FRBORDER_B);
  inner.x = box->x + !!(flags & FRBORDER_L);
  inner.y = box->y + !!(flags & FRBORDER_T);
  inner.w -= inner.x;
  inner.h -= inner.y;


  return err;
}
