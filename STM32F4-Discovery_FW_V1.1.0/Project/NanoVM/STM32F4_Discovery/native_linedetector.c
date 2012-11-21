//
//  c't-Bot-NanoVM, a tiny java VM for the c't-Bot
//
//  Copyright (C) 2007 by Nils Springob <nils@nicai-systems.de>
//  Based on work by Benjamin Benz(c't-Bot) and Till Harbaum(NanoVM)
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
//  native_linedetector.c
//

#include "types.h"
#include "debug.h"
#include "config.h"
#include "error.h"

#include "delay.h"
#include "stack.h"
#include "vm.h"
// #include "ctbot/native.h"
// #include "ctbot/native_linedetector.h"
// #include "ctbot/iodefs.h"

// #include "ctbot/shift.h"
// #include "ctbot/adc.h"
#include "native.h"
#include "native_linedetector.h"
#include "iodefs.h"

#include "shift.h"
#include "adc.h"


#define NATIVE_METHOD_updateLeft 1
#define NATIVE_METHOD_updateRight 2
#define NATIVE_METHOD_getLeft 3
#define NATIVE_METHOD_getRight 4
#define NATIVE_METHOD_setEnabled 5
#define NATIVE_METHOD_getEnabled 6

int test_argdata = 5;
	int test_argdata2 = 6;

nvm_int_t adc_data;
uint16_t value_l;
uint16_t value_r;

void native_ctbot_linedetector_init(void) {
//  adc_init(SENS_M_L);
//  adc_init(SENS_M_R);
}

// the ctbot class
void native_ctbot_linedetector_invoke(u08_t mref) {

  // JAVA: void updateLeft()
  if(mref == NATIVE_METHOD_updateLeft) {//getADC
		test_argdata = stack_pop_int();
		adc_data = adc_read(test_argdata);
		stack_push(adc_data>>8);
		stack_push(adc_data&0xFF);
  }
  
  // JAVA: void updateRight()
  else if(mref == NATIVE_METHOD_updateRight) {//setEnable
		test_argdata = stack_pop_int();
		test_argdata2 = stack_pop_int();
    //value_r = stm_adc_init(test_argdata2, test_argdata);*/
  }
  
  // JAVA: int getLeft()
  else if(mref == NATIVE_METHOD_getLeft) {//getEnable
    test_argdata = stack_pop_int();
	//	stm_adc_status(test_argdata);
  }

  // JAVA: int getRight()
  else if(mref == NATIVE_METHOD_getRight) {
    nvm_int_t val = value_r;
    stack_push(val);
  }

  // JAVA: void setEnabled(boolean enabled)
  else if(mref == NATIVE_METHOD_setEnabled) {
    if (stack_pop_int())
      shift_reg_ena &= ~(uint8_t)ENA_MAUS;
    else
      shift_reg_ena |= ENA_MAUS;
    shift_data(shift_reg_ena, SHIFT_REGISTER_ENA);
  }

  // JAVA: boolean getEnabled()
  else if(mref == NATIVE_METHOD_getEnabled) {
    stack_push((shift_reg_ena & ENA_MAUS)?1:0);
  }
}

