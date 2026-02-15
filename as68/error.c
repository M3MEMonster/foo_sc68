/*
 * @file    error.c
 * @brief   68000 macro assembler - error messages
 * @author  http://sourceforge.net/users/benjihan
 * @author  http://sourceforge.net/users/vincentp
 *
 * Copyright (c) 1993 Vincent Penne
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

/* generated config include */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

char *error_list[] = {
  /*00*/ "",
  /*01*/ "syntax error",
  /*02*/ "unknown symbol",
  /*03*/ "')' expected",
  /*04*/ "divide by zero",
  /*05*/ "undefined word",
  /*06*/ "',' expected",
  /*07*/ "unknown mnemonic",
  /*08*/ "'.S', '.B', '.W' or '.L' expected",
  /*09*/ "invalid value",
  /*10*/ "address register expected",
  /*11*/ "invalid '.B'",
  /*12*/ "unexecpected addressing mode",
  /*13*/ "invalid operand size",
  /*14*/ "data register expected",
  /*15*/ "'.W' expected",
  /*16*/ "expression must be evaluate in 1st pass",
  /*17*/ "register  expected",
  /*18*/ "unexpected end of file",
  /*19*/ "unexpected in BSS section",
  /*20*/ "can't open file",
  /*21*/ "no macro of this name",
  /*22*/ "internal error",
  /*23*/ "odd address",
  /*24*/ "'[' expected",
  /*25*/ "'{' expected",
  /*26*/ "invalid size",
  /*27*/ "invalid addressing mode",
  /*28*/ "already defined",
  /*29*/ "value out of range",
  /*30*/ "expected",
  /*31*/ "can't modify symbol",
  /*32*/ "macro too many argument",
  /*33*/ "numeric type expected",
  /*34*/ "could not incbin",
  /*35*/ "undefined",
  /*36*/
};

char *warn_list[] = {
  /*0*/ "",
  /*1*/ "word too long : truncated",
  /*2*/ "non integer value : truncated",
  /*3*/ "position dependant code",
  /*4*/ "ORG may bug -P option ... sorry",
};

int error_max = sizeof(error_list) / sizeof(error_list[0]);
int warn_max = sizeof(warn_list) / sizeof(warn_list[0]);
