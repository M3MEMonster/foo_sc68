#ifndef _CDBG68_EVENT_H_
#define _CDBG68_EVENT_H_

enum event_e {
  E_NONE = 0,
  E_REDRAW,

  E_UP,
  E_DOWN,
  E_LEFT,
  E_RIGHT,

  E_PGUP,
  E_PGDOWN,

};

typedef struct event_s event_t;

struct event_s {
  int type;
  union {
    int key;
  } data;
};

#endif /* #ifndef _CDBG68_EVENT_H_ */
