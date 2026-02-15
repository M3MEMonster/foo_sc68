#include "msg.h"

#include <stdio.h>
#include <stdlib.h>

int msglst_create(msglst_t * m, int max)
{
  int err = -1;

  if ( m ) {
    m->n   = 0;
    m->max = 0;
    m->msg = 0;
      
    if ( max ) {
      m->msg = malloc(sizeof(m->msg[0])*max);
      if ( m->msg ) {
        int i;
        for ( i = 0; i < max; ++i ) {
          m->msg[i] = 0;
        }
        m->max = i;
      }
    }
    err = -( max != m->max );
  }

  return err;
}

void msglst_destroy(msglst_t * m)
{
  if ( m ) {
    int i;

    for ( i = 0; i < m->n; ++i ) {
      free( m->msg[i] );
      m->msg[i] = 0;
    }
    free( m->msg );
    m->msg = 0;
    m->n = m->max = 0;
  }
}

int msglst_alloc(msglst_t * m)
{
  int i = -1;

  if ( m ) {
    if ( m->n == m->max ) {
      const int max = !m->max ? 128 : m->max*2;
      char ** newbuf = realloc( m->msg, sizeof(m->msg[0]) * max );
      if (newbuf) {
        int i;
        for ( i = m->max; i < max; ++i )
          newbuf[i] = 0;
        m->max = i;
        m->msg = newbuf;
      }
    }
    if ( m->n < m->max ) {
      i = m->n++;
    }
  }
  return i;
}
