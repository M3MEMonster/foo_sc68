/*
 * @file    oplen.c
 * @brief   Compute 68k opcode length compact table
 * @author  http://sourceforge.net/users/benjihan
 *
 * Copyright (c) 2015 Benjamin Gerard
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
 *
 */

#include <desa68.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char ** argv)
{
  uint8_t tmp[64]; char str[256];
  desa68_t desa;
  int i, max = 0, cur, cnt, acu, n;

  memset(tmp,0,sizeof(tmp));
  memset(&desa,0,sizeof(desa));
  desa.memptr = tmp;
  desa.memlen = 64;
  desa.str = str;
  desa.strmax = sizeof(str);

  for (acu=cur=cnt=n=i=0; i<0x10000; ++i) {
    int len;
    tmp[0] = i>>8;
    tmp[1] = i;
    desa.pc = 0;
    desa68(&desa);
    len = desa.pc;
    if (len > max) max = len;
    if (len == cur && cnt < 32)
      cnt++;
    else {
      if (cur) {
        printf("%sN(%d,%d)%s",(n&7)?"":"\t", cnt, cur,(n&7)==7?",\n":",");
        ++n;
      }
      acu += cnt;
      cnt = 1;
      cur = len;
    }
  }
  acu += cnt;
  printf("%sN(%d,%d)%s",n?"":"\t", cnt, cur,(n==7)?",\n":",");
  return 0;
}
