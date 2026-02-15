/*
 * @file    insttest68.c
 * @brief   68k instruction test
 * @author  http://sourceforge.net/users/benjihan
 *
 * Copyright (c) 1998-2015 Benjamin Gerard
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef WORDSZ
# define WORDSZ 32
#endif

#if WORDSZ == 64
# define WORD   int64_t
# define UWORD  uint64_t
#elif WORDSZ == 32
# define WORD   int32_t
# define UWORD  uint32_t
#elif WORDSZ == 16
# define WORD   int16_t
# define UWORD  uint16_t
#else
# define WORD   int8_t
# define UWORD  uint8_t
#endif

typedef  WORD  int_t;
typedef UWORD uint_t;

#define SIGN_BIT ( (sizeof(int_t)*8-1) )
#define NORM_FIX ( (sizeof(int_t)-1)*8 )
#define SIGN_MSK ( (int_t)( (int_t) 0x80 << NORM_FIX ) )
#define NORM_MSK ( (int_t)((int_t)1 << SIGN_BIT) >> 7 )
/* #define NORM_ONE ( (int_t)*((int_t) 0xFF << NORM_FIX ) ) */

static int verbose = 1;

typedef union sr_def_u {
  int bits;
  struct {
    unsigned int C:1;                 /* 0 */
    unsigned int V:1;                 /* 1 */
    unsigned int Z:1;                 /* 2 */
    unsigned int N:1;                 /* 3 */
    unsigned int X:1;                 /* 4 */
  } bit;
} sr_def_t;

enum {
  SR_C_BIT = 0,
  SR_V_BIT = 1,
  SR_Z_BIT = 2,
  SR_N_BIT = 3,
  SR_X_BIT = 4
};

enum {
  SR_C = 1 << SR_C_BIT,
  SR_V = 1 << SR_V_BIT,
  SR_Z = 1 << SR_Z_BIT,
  SR_N = 1 << SR_N_BIT,
  SR_X = 1 << SR_X_BIT
};

const char * srtostr(char * str, const sr_def_t * sr)
{
  static const char tpl[] = "CVZNX";
  if (!str) {
    return tpl;
  } else {
    int i;
    for (i=0; i<5; ++i) {
      str[i] = (sr->bits & (1<<i)) ? tpl[i] : '.';
    }
    str[i] = 0;
  }
  return str;
}

static
int_t get_X(const sr_def_t * sr)
{
  return (int_t) sr->bit.X << NORM_FIX;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static
void add_sr_ref(int_t S, int_t D, int_t R, sr_def_t * sr)
{
  sr->bits  = 0;
  sr->bit.V = (S<0 && D<0 && R>=0) || (S>=0 && D>=0 && R<0);
  sr->bit.C = (S<0 && D<0) || (R>=0 && D<0) || (S<0 && R>=0);
  sr->bit.X = sr->bit.C;
  sr->bit.Z = !R;
  sr->bit.N = R < 0;
}

static
int_t add_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = S + D;
  add_sr_ref(S, D, R, sr);
  return R;
}

static
int_t adx_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = S + D + get_X(sr);
  add_sr_ref(S, D, R, sr);
  return R;
}

/* ---------------------------------------------------------------------- */

static
void add_sr_opt(int_t S, int_t D, int_t R, sr_def_t * sr)
{
  int_t Re = R;

  R  = ( ( R >> SIGN_BIT ) & ( SR_V | SR_C | SR_X | SR_N ) ) ^ SR_V;
  S  = ( ( S >> SIGN_BIT ) & ( SR_V | SR_C | SR_X ) ) ^ R;
  D  = ( ( D >> SIGN_BIT ) & ( SR_V | SR_C | SR_X ) ) ^ R;
  R &= ~SR_N;
  R |= SR_V | ( !Re << SR_Z_BIT );
  R ^= S | D;
  sr->bits = R;
}

static
int_t add_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = S + D;
  add_sr_opt(S, D, R, sr);
  return R;
}

static
int_t adx_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = S + D + get_X(sr);
  add_sr_opt(S, D, R, sr);
  return R;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static
void sub_sr_ref(int_t S, int_t D, int_t R, sr_def_t * sr)
{
  sr->bits  = 0;
  sr->bit.V = (S>=0 && D<0 && R>=0) || (S<0 && D>=0 && R<0);
  sr->bit.C = (S<0 && D>=0) || (R<0 && D>=0) || (S<0 && R<0);
  sr->bit.X = sr->bit.C;
  sr->bit.Z = !R;
  sr->bit.N = R<0;
}

static
int_t sub_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = D - S;
  sub_sr_ref(S, D, R, sr);
  return R;
}

static
int_t sbx_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = D - S - get_X(sr);
  sub_sr_ref(S, D, R, sr);
  return R;
}

/* ---------------------------------------------------------------------- */

static
void sub_sr_opt(int_t S, int_t D, int_t R, sr_def_t * sr)
{
  int ccr = (!R << SR_Z_BIT)            /* 2 */
    | (( R >> SIGN_BIT ) & SR_N)        /* 2 */
    | (((((D ^ ~R) & (S ^ R)) ^ R) >> SIGN_BIT) & (SR_C+SR_X)) /* 6 */
    | ((((D ^ R) & (~S ^ R)) >> SIGN_BIT) & SR_V) /* 5 */
    ;
  sr->bits = ccr;
  /* 18 ops */
}

static
int_t sub_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = D - S;
  sub_sr_opt(S, D, R, sr);
  return R;
}

static
int_t sbx_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = D - S - get_X(sr);
  sub_sr_opt(S, D, R, sr);
  return R;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static
void neg_sr_ref(int_t S, int_t D, int_t R, sr_def_t * sr)
{
  sr->bits = 0;
  sr->bit.V = (D<0) & (R<0);
  sr->bit.C = (D<0) | (R<0);
  sr->bit.X = sr->bit.C;
  sr->bit.Z = !R;
  sr->bit.N = R < 0;
}

static
int_t neg_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = 0 - D;
  neg_sr_ref(S, D, R, sr);
  return R;
}

static
int_t ngx_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = 0 - D - get_X(sr);
  neg_sr_ref(S, D, R, sr);
  return R;
}

/* ---------------------------------------------------------------------- */

static
void neg_sr_opt(int_t S, int_t D, int_t R, sr_def_t * sr)
{
  int ccr = !R << SR_Z_BIT;            /* 2 */
  D >>= SIGN_BIT;                      /* 1 */
  R >>= SIGN_BIT;                      /* 1 */
  ccr |= R & SR_N;                     /* 2 */

  ccr |= (D|R) & (SR_C|SR_X);         /* 3 */
  ccr |= (D&R) & SR_V;                /* 3 */

  sr->bits = ccr;
  /* 12 ops */
}

static
int_t neg_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = 0 - D;
  neg_sr_opt(S, D, R, sr);
  return R;
}

static
int_t ngx_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = 0 - D - get_X(sr);
  neg_sr_opt(S, D, R, sr);
  return R;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static
int_t mlu_res(int_t S, int_t D, const sr_def_t * sr)
{
  uint_t uS = S, uD = D;
  uS >>= (NORM_FIX+4);
  uD >>= (NORM_FIX+4);
  return (uS * uD) << NORM_FIX;
}

static
int_t mls_res(int_t S, int_t D, const sr_def_t * sr)
{
  int_t sS = S, sD = D;
  sS >>= (NORM_FIX+4);
  sD >>= (NORM_FIX+4);
  return (sS * sD) << NORM_FIX;
}

static
void mul_sr_ref(int_t S, int_t D, int_t R, sr_def_t * sr)
{
  sr->bit.C = 0;
  sr->bit.V = 0;
  sr->bit.Z = !R;
  sr->bit.N = R<0;
}

static
int_t mls_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = mls_res(S, D, sr);
  mul_sr_ref(S, D, R, sr);
  return R;
}

static
int_t mlu_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = mlu_res(S, D, sr);
  mul_sr_ref(S, D, R, sr);
  return R;
}

/* ---------------------------------------------------------------------- */

static
void mul_sr_opt(int_t S, int_t D, int_t R, sr_def_t * sr)
{
  sr->bits = (sr->bits & SR_X)
    | (!R << SR_Z_BIT)
    | ((R >> (SIGN_BIT - SR_N_BIT)) & SR_N)
    ;
}

static
int_t mls_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = mls_res(S, D, sr);
  mul_sr_opt(S, D, R, sr);
  return R;
}

static
int_t mlu_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = mlu_res(S, D, sr);
  mul_sr_opt(S, D, R, sr);
  return R;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static
int_t bts_res(int_t S, int_t D, sr_def_t * sr)
{
  return D;
}

static
int_t bse_res(int_t S, int_t D, sr_def_t * sr)
{
  const int bit =  ( (S >> NORM_FIX) & 7 ) + NORM_FIX;
  return D | ( (int_t) 1 << bit );
}

static
int_t bcl_res(int_t S, int_t D, sr_def_t * sr)
{
  const int bit =  ( (S >> NORM_FIX) & 7 ) + NORM_FIX;
  return D & ~( (int_t) 1 << bit );
}

static
int_t bch_res(int_t S, int_t D, sr_def_t * sr)
{
  const int bit =  ( (S >> NORM_FIX) & 7 ) + NORM_FIX;
  return D ^ ( (int_t) 1 << bit );
}

/* ---------------------------------------------------------------------- */

static
void bop_sr_ref(int_t S, int_t D, int_t R, sr_def_t * sr)
{
  const int bit =  ( (S >> NORM_FIX) & 7 ) + NORM_FIX;
  sr->bit.Z = ( (D >> bit) & 1 ) ^ 1;
}

static
int_t bts_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = bts_res(S, D, sr);
  bop_sr_ref(S, D, R, sr);
  return R;
}

static
int_t bse_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = bse_res(S, D, sr);
  bop_sr_ref(S, D, R, sr);
  return R;
}

static
int_t bcl_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = bcl_res(S, D, sr);
  bop_sr_ref(S, D, R, sr);
  return R;
}

static
int_t bch_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = bch_res(S, D, sr);
  bop_sr_ref(S, D, R, sr);
  return R;
}

/* ---------------------------------------------------------------------- */

static
void bop_sr_opt(int_t S, int_t D, int_t R, sr_def_t * sr)
{
  const int bit =  ( (S >> NORM_FIX) & 7 ) + NORM_FIX;
  sr->bits = (sr->bits & ~SR_Z)
    | ( -( (~D >> bit) & 1 ) & SR_Z );
}

static
int_t bts_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = bts_res(S, D, sr);
  bop_sr_opt(S, D, R, sr);
  return R;
}

static
int_t bse_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = bse_res(S, D, sr);
  bop_sr_opt(S, D, R, sr);
  return R;
}

static
int_t bcl_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = bcl_res(S, D, sr);
  bop_sr_opt(S, D, R, sr);
  return R;
}

static
int_t bch_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = bch_res(S, D, sr);
  bop_sr_opt(S, D, R, sr);
  return R;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static
int_t lsr_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  uint_t R = (uint_t) D >> NORM_FIX;
  int cnt;
  S = (S >> NORM_FIX) & 63;

  sr->bit.C = sr->bit.V = 0;
  for (cnt = S; cnt; --cnt) {
    sr->bit.C = sr->bit.X = R & 1;
    R >>= 1;
  }
  sr->bit.Z = !R;
  sr->bit.N = ( R >> 7 ) & 1;

  return R << NORM_FIX;
}

static
int_t lsr_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  uint_t R = D;
  int ccr;
  S = (S >> NORM_FIX) & 63;

  if (--S < 0) {
    ccr = sr->bits & SR_X;             /* no shift: X is unaffected */
  } else if (S > l) {
    R   = 0;
    ccr = 0;
  } else {
    const uint_t m = NORM_MSK;

    R >>= S;
    ccr = -( ( R >> ( SIGN_BIT - l ) ) & 1 ) & ( SR_X | SR_C );
    R = ( R >> 1 ) & m;
  }
  ccr |= ( -!R & SR_Z ) | ( ( R >> ( SIGN_BIT - SR_N_BIT ) ) & SR_N );
  sr->bits = ccr;
  return R;
}

/* ---------------------------------------------------------------------- */

static
int_t asr_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = D >> NORM_FIX;
  int cnt;
  S = (S >> NORM_FIX) & 63;

  sr->bit.C = sr->bit.V = 0;
  for (cnt = S; cnt; --cnt) {
    sr->bit.C = sr->bit.X = R & 1;
    R >>= 1;
  }
  sr->bit.Z = !R;
  sr->bit.N = ( R >> 7 ) & 1;
  return R << NORM_FIX;
}

static
int_t asr_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = D;
  int ccr;
  S = (S >> NORM_FIX) & 63;

  if (--S < 0) {
    ccr = sr->bits & SR_X;             /* no shift: X is unaffected */
  } else if (S > l) {
    R >>= SIGN_BIT;
    ccr = R & ( SR_C | SR_X );
  } else {
    const uint_t m = NORM_MSK;
    R >>= S;
    ccr = -( ( R >> ( SIGN_BIT - l ) ) & 1 ) & ( SR_X | SR_C );
    R = ( R >> 1 ) & m;
  }
  ccr |= ( -!R & SR_Z ) | ( ( R >> ( SIGN_BIT - SR_N_BIT ) ) & SR_N );
  sr->bits = ccr;
  return R;
}

/* ---------------------------------------------------------------------- */

static
int_t lsl_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = D;
  int cnt;
  S = (S >> NORM_FIX) & 63;

  sr->bit.C = sr->bit.V = 0;
  for (cnt = S; cnt; --cnt) {
    sr->bit.C = sr->bit.X = R < 0;
    R <<= 1;
  }
  sr->bit.Z = !R;
  sr->bit.N = R < 0;
  return R;
}

static
int_t lsl_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int ccr;
  int_t R = D;
  S = (S >> NORM_FIX) & 63;
  if (--S < 0) {
    ccr = sr->bits & SR_X;             /* no shift: X is unaffected */
  } else if (S > SIGN_BIT) {
    R = 0;
    ccr = 0;
  } else {
    R <<= S;
    ccr = ( R >> SIGN_BIT ) & ( SR_X | SR_C );
    R <<= 1;
  }
  ccr |= ( -!R & SR_Z ) | ( ( R >> ( SIGN_BIT - SR_N_BIT ) ) & SR_N );
  sr->bits = ccr;
  return R;
}

/* ---------------------------------------------------------------------- */

static
int_t asl_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = D;
  int cnt;
  S = (S >> NORM_FIX) & 63;

  sr->bit.C = sr->bit.V = 0;
  for (cnt = S; cnt; --cnt) {
    sr->bit.C = sr->bit.X = R < 0;
    R <<= 1;
    sr->bit.V |= sr->bit.C ^ (R < 0);
  }
  sr->bit.Z = !R;
  sr->bit.N = R < 0;
  return R;
}

static
int_t asl_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int_t R = D;
  int ccr;

  S = (S >> NORM_FIX) & 63;
  if (--S < 0) {
    ccr = sr->bits & SR_X;             /* no shift: X is unaffected */
  } else if (S > l) {
    ccr  = -!!R & SR_V;         /* unless no bit set overflow occurs */
    R  = 0;
  } else {
    R <<= S;
    ccr = ( R >> SIGN_BIT ) & ( SR_X | SR_C );
    R <<= 1;
    ccr |= -( D != ( (R >> 1) >> S ) ) & SR_V;
  }
  ccr |= ( -!R & SR_Z ) | ( ( R >> ( SIGN_BIT - SR_N_BIT ) ) & SR_N );
  sr->bits = ccr;
  return R;
}

/* ---------------------------------------------------------------------- */

static
int_t ror_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  uint_t R = (uint_t) D >> NORM_FIX;
  int cnt;
  S = (S >> NORM_FIX) & 63;

  sr->bit.C = sr->bit.V = 0;
  for (cnt = S; cnt; --cnt) {
    sr->bit.C = R & 1;
    R = ( R >> 1 ) | ( (int_t) sr->bit.C << ( SIGN_BIT - NORM_FIX ) );
  }
  sr->bit.Z = !R;
  sr->bit.N = R >> ( SIGN_BIT - NORM_FIX );

  return R << NORM_FIX;
}

static
int_t ror_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int ccr = sr->bits & SR_X;            /* X is unaffected */
  uint_t R = D;

  S = (S >> NORM_FIX) & 63;
  if (S) {
    const uint_t m = NORM_MSK;
    R = ( ( R >> ( S & l ) ) | ( R << ( -S & l ) ) ) & m;
    ccr |= ( R >> ( SIGN_BIT - SR_C_BIT ) ) & SR_C; /* carry is sign bit */
  }
  ccr |= ( -!R & SR_Z ) | ( ( R >> ( SIGN_BIT - SR_N_BIT ) ) & SR_N );
  sr->bits = ccr;
  return R;
}

/* ---------------------------------------------------------------------- */

static
int_t rol_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  uint_t R = (uint_t)D;
  int cnt;
  S = ( S >> NORM_FIX ) & 63;

  sr->bit.C = sr->bit.V = 0;
  for (cnt = S; cnt; --cnt) {
    sr->bit.C = R >> SIGN_BIT;
    R = ( R << 1 ) | ( (int_t) sr->bit.C << NORM_FIX );
  }
  sr->bit.Z = !R;
  sr->bit.N = R >> SIGN_BIT;

  return R;
}

static
int_t rol_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
  int ccr = sr->bits & SR_X;            /* X is unaffected */
  uint_t R = D;

  S = (S >> NORM_FIX) & 63;
  if (S) {
    const uint_t m = NORM_MSK;
    R = ( ( R << ( S & l ) ) | ( R >> ( -S & l ) ) ) & m;
    ccr |= -( ( R >> ( SIGN_BIT - l ) ) & 1 ) & SR_C; /* carry is sign bit */
  }
  ccr |= ( -!R & SR_Z ) | ( ( R >> ( SIGN_BIT - SR_N_BIT ) ) & SR_N );
  sr->bits = ccr;
  return R;
}


/* ---------------------------------------------------------------------- */

static
int_t rrx_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  uint_t R = (uint_t) D >> NORM_FIX;
  int cnt;
  S = (S >> NORM_FIX) & 63;

  sr->bit.V = 0;
  sr->bit.C = sr->bit.X;
  for (cnt = S; cnt; --cnt) {
    sr->bit.X = R & 1;
    R = ( R >> 1 ) | ( (int_t) sr->bit.C << ( SIGN_BIT - NORM_FIX ) );
    sr->bit.C = sr->bit.X;
  }
  R <<= NORM_FIX;
  sr->bit.Z = !R;
  sr->bit.N = (int_t)R < 0;

  return R;
}

static
int_t rrx_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
/*
  X 01234567  // ROXR
  - --------
  x 01234567  // 0 n/a
  7 x0123456  // 1 0
  6 7x012345  // 2 1
  5 67x01234  // 3 2
  4 567x0123  // 4 3
  3 4567x012  // 5 4
  2 34567x01  // 6 5
  1 234567x0  // 7 6
  0 1234567x  // 8 7
*/
  int ccr = sr->bits & SR_X;            /* X is unaffected */
  uint_t R = D;
  int cnt;

  cnt = (S >> NORM_FIX) & 63;
  if (cnt) {
    const uint_t m = NORM_MSK;
    cnt %= (l+2);                       /* cnt := [0 .. l+1] */

    if (--cnt >= 0) {                   /* S := [0 .. l] */
      uint_t x, c;

      R >>= cnt;
      c   = (ccr >> SR_X_BIT) & 1;            /* old X */
      x   = (R >> (SIGN_BIT-l)) & 1; /* new X */
      ccr = -(int)x & SR_X;
      R >>= 1;
      R |= c << (SIGN_BIT-cnt);
      D <<= 1;
      D <<= l-cnt;
      R = ( R | D ) & m;

    }
  }

  ccr |= (ccr & SR_X) >> (SR_X_BIT - SR_C_BIT); /* C is X whatever shift */
  ccr |= -!R & SR_Z;
  ccr |= ( R >> ( SIGN_BIT - SR_N_BIT ) ) & SR_N;
  sr->bits = ccr;
  return R;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static
int_t rlx_ref(int_t S, int_t D, sr_def_t * sr, const int l)
{
  uint_t R = (uint_t) D >> NORM_FIX;
  int cnt;
  S = (S >> NORM_FIX) & 63;

  sr->bit.V = 0;
  sr->bit.C = sr->bit.X;
  for (cnt = S; cnt; --cnt) {
    sr->bit.X = R >> ( SIGN_BIT - NORM_FIX );
    R = ( R << 1 ) | sr->bit.C;
    sr->bit.C = sr->bit.X;
  }
  R <<= NORM_FIX;
  sr->bit.Z = !R;
  sr->bit.N = (int_t)R < 0;

  return R;
}

/* ---------------------------------------------------------------------- */

static
int_t rlx_opt(int_t S, int_t D, sr_def_t * sr, const int l)
{
/*
  X 01234567  // ROXL
  - --------
  x 01234567  // 0 n/a
  0 1234567x  // 1 0
  1 234567x0  // 2 1
  2 34567x01  // 3 2
  3 4567x012  // 4 3
  4 567x0123  // 5 4
  5 67x01234  // 6 5
  6 7x012345  // 7 6
  7 x0123456  // 8 7
*/
  int ccr = sr->bits & SR_X;            /* X is unaffected */
  uint_t R = D;
  int cnt;

  cnt = (S >> NORM_FIX) & 63;
  if (cnt) {
    const uint_t m = NORM_MSK;
    cnt %= (l+2);                       /* cnt := [0 .. l+1] */

    if (--cnt >= 0) {                   /* S := [0 .. l] */
      uint_t x, c;

      D <<= cnt;
      c   = (ccr >> SR_X_BIT) & 1;            /* old X */
      x   = (uint_t) D >> SIGN_BIT;           /* new X */
      ccr = -(int)x & SR_X;
      D <<= 1;
      D |= c << (SIGN_BIT-l+cnt);
      R >>= 1;
      R >>= l-cnt;
      R = ( R | D ) & m;
    }
  }

  ccr |= (ccr & SR_X) >> (SR_X_BIT - SR_C_BIT); /* C is X whatever shift */
  ccr |= -!R & SR_Z;
  ccr |= ( R >> ( SIGN_BIT - SR_N_BIT ) ) & SR_N;
  sr->bits = ccr;
  return R;
}

int main(int argc, char ** argv)
{
  int k,j,i;
  int k_step = SR_X;
  struct {
    const char * name;
    int sz;
    struct {
      int_t (*fct)(int_t, int_t, sr_def_t *, const int);
      sr_def_t sr;
      char str[8];
    } ref, opt;
    sr_def_t sr;
    char str[8];
    int err;
  } insts[] =
      {
        { "ADD",  7, { add_ref }, { add_opt } },
        { "SUB",  7, { sub_ref }, { sub_opt } },
        { "NEG",  7, { neg_ref }, { neg_opt } },

        { "ADDX", 7, { adx_ref }, { adx_opt } },
        { "SUBX", 7, { sbx_ref }, { sbx_opt } },
        { "NEGX", 7, { ngx_ref }, { ngx_opt } },

        { "MULS", 7, { mls_ref }, { mls_opt } },
        { "MULU", 7, { mlu_ref }, { mlu_opt } },

        { "BTST", 5, { bts_ref }, { bts_opt } },
        { "BSET", 5, { bse_ref }, { bse_opt } },
        { "BCLR", 5, { bcl_ref }, { bcl_opt } },
        { "BCHG", 5, { bch_ref }, { bch_opt } },

        { "LSR",  7, { lsr_ref }, { lsr_opt } },
        { "LSL",  7, { lsl_ref }, { lsl_opt } },

        { "ASR",  7, { asr_ref }, { asr_opt } },
        { "ASL ", 7, { asl_ref }, { asl_opt } },

        { "ROR",  7, { ror_ref }, { ror_opt } },
        { "ROL",  7, { rol_ref }, { rol_opt } },

        { "ROXR", 7, { rrx_ref }, { rrx_opt } },
        { "ROXL", 7, { rlx_ref }, { rlx_opt } },

        { 0 }
      };

  /* parse options */
  for (i=k=1; i<argc; ++i) {
    if (argv[i][0] == '-') {
      for (j=1; argv[i][j]; ++j) {
        switch (argv[i][j]) {
        case 'v': ++verbose; break;
        case 'q': --verbose; break;
        case 'a': k_step = 1; break;
        }
      }
    } else {
      argv[k++] = argv[i];
    }
  }
  argc = k;

  for (j=0; insts[j].name; ++j) {
    unsigned int s;

    insts[j].err = 0;
    if (argc > 1) {
      for (k=1; k<argc && strcasecmp(argv[k],insts[j].name) ; ++k)
        ;
      if (k == argc) continue;
    }

    for (s = 0; s < 0x10000; ++s) {   /* all operands */
      uint_t S = (uint_t)(s & 255) << NORM_FIX;
      uint_t D = (uint_t)(s >>  8) << NORM_FIX;

      for (k=0; k < 32; k += k_step) { /* all sr bits */
        int err;
        int_t ref, opt;
        static char * estr[4] = { "OK","ERR RES","ERR SR","ERR BOTH" };
        const int l = 8-1;

        insts[j].sr.bits = k;       /* Set SR bit before op. */
        srtostr(insts[j].str,&insts[j].sr);

        insts[j].ref.sr = insts[j].sr;
        ref = (uint_t)
          insts[j].ref.fct(S, D, &insts[j].ref.sr, l) >> NORM_FIX;
        srtostr(insts[j].ref.str, &insts[j].ref.sr);

        insts[j].opt.sr = insts[j].sr;
        opt = (uint_t)
          insts[j].opt.fct(S, D, &insts[j].opt.sr, l) >> NORM_FIX;
        srtostr(insts[j].opt.str, &insts[j].opt.sr);

        err = ( ref != opt ) |
          ( insts[j].ref.sr.bits != insts[j].opt.sr.bits ) * 2;
        insts[j].err += err;

        if ( verbose > 1 || (verbose > 0 && err) )
          printf("%-4s %s %02x %02x -> %02x %s -> %02x %s | %s\n",
                 insts[j].name, insts[j].str,
                 (int)(S >> NORM_FIX), (int)(D >> NORM_FIX),
                 (int)ref & 255, insts[j].ref.str,
                 (int)opt & 255, insts[j].opt.str,
                 estr[err]
            );
      } /* sr bits  */
    }   /* all op   */

    if (!insts[j].err) {
      if (verbose > 0)
        printf("%-4s SUCCESS\n", insts[j].name);
    } else if (verbose >= 0) {
      printf("%-4s ERROR (%d)\n", insts[j].name, insts[j].err);
    }

  }     /* all inst */

  /* */
  for (i = j = 0; insts[j].name; ++j) {
    if (insts[j].err) {
      i = 127;
      break;
    }
  }
  return i;
}
