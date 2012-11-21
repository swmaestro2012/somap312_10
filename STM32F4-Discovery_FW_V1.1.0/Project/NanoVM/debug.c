//
//  NanoVM, a tiny java VM for the Atmel AVR family
//  Copyright (C) 2005 by Till Harbaum <Till@Harbaum.org>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 

//
//  debug.c
//

#include "types.h"
#include "config.h"
#include "debug.h"

#ifdef DEBUG


bool_t debug_isprint(u08_t chr) {
  return(chr >= 32 && chr <= 127);
}

void debug_hexdump(void *data, u16_t size) {
  u16_t i,n = 0, b2c;
  u08_t *ptr = data;

  if(!size) return;

  DEBUGF("---- dump %d bytes\n", size);

  while(size>0) {
    DEBUGF(DBG16": ", n);

    b2c = (size>16)?16:size;

    for(i=0;i<b2c;i++)
      DEBUGF(DBG8" ", ptr[i]);

    DEBUGF("  ");

    for(i=0;i<(16-b2c);i++)
      DEBUGF("   ");

    for(i=0;i<b2c;i++)
      DEBUGF("%c", debug_isprint(ptr[i])?ptr[i]:'.');

    DEBUGF("\n");

    ptr  += b2c;
    size -= b2c;
    n    += b2c;
  }
  DEBUGF("\n");
}

#endif // DEBUG
