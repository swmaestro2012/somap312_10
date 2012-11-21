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
//  vm.c
//

//#include "stdio.h"

#include "types.h"
#include "debug.h"
#include "config.h"
#include "error.h"

#include "vm.h"
#include "opcodes.h"
#include "native_impl.h"
#include "native.h"
#include "heap.h"
#include "nvmfile.h"
#include "stack.h"
#include "nvmfeatures.h"
#include "uart.h"
#include "kernel.h"
#include "mutex.h"

#ifdef NVM_USE_ARRAY
#include "array.h"
#endif

#ifdef NVM_USE_32BIT_WORD
# define DBG_INT "0x" DBG32
#else
# define DBG_INT "0x" DBG16
#endif

#define RUNNABLE_TABLE_SIZE	10
#define RUNNABLE_HEAP_ID 0
#define RUNNABLE_METHOD_ID 1
//u16_t runnable_table_count = 0;
//u32_t runnable_table[RUNNABLE_TABLE_SIZE][2];

void vm_init(void) {
  DEBUGF("vm_init() with %d static fields\n", nvmfile_get_static_fields());

  // init heap
  heap_init();

  // get stack space from heap and setup stack
  stack_init(nvmfile_get_static_fields());
 
  stack_push(0); // args parameter to main (should be a string array)
}

void *vm_get_addr(nvm_ref_t ref) {
  if(!(ref & NVM_IMMEDIATE_MASK))
    error(ERROR_VM_ILLEGAL_REFERENCE);

  if((ref & NVM_TYPE_MASK) == NVM_TYPE_HEAP)
    return heap_get_addr(ref & ~NVM_TYPE_MASK);

  // return nvmfile address and set marker indicating
  // that this is inside the nvm file (and may have
  // to be accessed in a special manner)
  return NVMFILE_SET(nvmfile_get_addr(ref & ~NVM_TYPE_MASK));
}

// expand 15 bit immediate to 16 bits (or 31 to 32)
nvm_int_t nvm_stack2int(nvm_stack_t val) {
  //if(val & (NVM_IMMEDIATE_MASK>>1))
  //  val |= NVM_IMMEDIATE_MASK;   // expand sign bit
  return val;
}

#ifdef NVM_USE_FLOAT
nvm_stack_t nvm_float2stack(nvm_float_t val)
{
	uint8_t msb;
  nvm_union_t v;
  v.f[0]=val;
  //printf("float = %f == 0x%x", v.f[0], v.i[0]);
  msb = (v.b[3]&0x80)?0x40:0x00;
  v.b[3] &= 0x7f;
  if (v.b[3]==0x7f && (v.b[2]&0x80)==0x80)
    msb |= 0x3f;
  else if (v.b[3]!=0x00 || (v.b[2]&0x80)!=0x00)
    msb |= v.b[3]-0x20;
  v.b[3]=msb;
  //printf(" -> encoded = 0x%x\n", v.i[0]);
  return v.i[0];
}

nvm_float_t nvm_stack2float(nvm_stack_t val)
{
	
	uint8_t msb;
  nvm_union_t v;
  v.i[0]=val;
  //printf("encoded = 0x%x", v.i[0]);
  msb = (v.b[3]&0x40)?0x80:0x00;
  v.b[3] &= 0x3f;
  if (v.b[3]==0x3f && (v.b[2]&0x80)==0x80)
    msb |= 0x7f;
  else if (v.b[3]!=0x00 || (v.b[2]&0x80)!=0x00)
    msb |= v.b[3]+0x20;
  v.b[3]=msb;
  //printf(" -> float = %f == 0x%x\n", v.f[0], v.i[0]);
  return v.f[0];
}
#endif

//YHC
nvm_stack64_t nvm_long2stack(nvm_long_t val)
{
	nvm_stack64_t ret;
	ret = val;
//	ret.l = val.l;
//	ret.h = val.h;
	return ret;
}
// YHC
nvm_long_t nvm_stack2long(nvm_stack_t* pVal) {
	nvm_long_t ret;
	ret = (*((nvm_long_t*)pVal--)) << 32;
	ret += *(pVal);
  //if(val & (NVM_IMMEDIATE_MASK>>1))
  //  val |= NVM_IMMEDIATE_MASK;   // expand sign bit
  return ret;
}

nvm_double_t nvm_stack2double(nvm_stack_t* pVal)
{
	nvm_double_t t;
	/*
	uint8_t msb;
  nvm_union2_t v;
  v.i[0]=val;
  //printf("encoded = 0x%x", v.i[0]);
  msb = (v.b[3]&0x40)?0x80:0x00;
  v.b[3] &= 0x3f;
  if (v.b[3]==0x3f && (v.b[2]&0x80)==0x80)
    msb |= 0x7f;
  else if (v.b[3]!=0x00 || (v.b[2]&0x80)!=0x00)
    msb |= v.b[3]+0x20;
  v.b[3]=msb;
  //printf(" -> float = %f == 0x%x\n", v.f[0], v.i[0]);
  return v.f[0];*/
	return t;
}



// pc/methodref/localsoffset
#define VM_METHOD_CALL_REQUIREMENTS 3

// create an instance of a class. check if it's local (within 
// the nvm file) or native (implemented by the runtime environment)
void vm_new(u16_t mref) {
	heap_id_t h;
  if(NATIVE_ID2CLASS(mref) < NATIVE_CLASS_BASE) {
    DEBUGF("local new #%d\n", NATIVE_ID2CLASS(mref));

    DEBUGF("non static fields: %d\n",
       nvmfile_get_class_fields(NATIVE_ID2CLASS(mref)));

    // create object with
    h = heap_alloc(TRUE, sizeof(nvm_word_t) *
     (VM_CLASS_CONST_ALLOC+nvmfile_get_class_fields(NATIVE_ID2CLASS(mref))));
		//if(mref & 0xFF) //this means runnable method exist
		//{
		//	runnable_table[runnable_table_count][RUNNABLE_HEAP_ID] = NVM_TYPE_HEAP | h;
		//	runnable_table[runnable_table_count][RUNNABLE_METHOD_ID] = mref & 0xFF;
		//	runnable_table_count++;
		//}
    stack_push(NVM_TYPE_HEAP | h);

    // store reference in object, so we can later determine which kind
    // of object this is. this is required for inheritance
    ((nvm_ref_t*)heap_get_addr(h))[0] = mref;

    return;
  }

  native_new(mref);
}

// we prefetch arguments from the program storage
// and this is the type it is stored into

typedef union {
  s16_t w;
  struct {
    s08_t bl, bh;
  } z;
  nvm_int_t tmp;
} vm_arg_t;

int vm_run(void *ref) {
	u16_t mref = (u16_t)ref;
  u08_t instr, pc_inc, *pc;
	//u16_t exc_st,exc_ed,exc_hdl,exc_cat;
  nvm_int_t tmp1=0;
  nvm_int_t tmp2;
  nvm_long_t tmpl1=0;
  nvm_long_t tmpl2=0;
  vm_arg_t arg0;
  nvm_method_hdr_t mhdr, *mhdr_ptr;
#ifdef NVM_USE_FLOAT
  nvm_float_t f0;
  nvm_float_t f1;
#endif
	u08_t instr_count=0;
//	u08_t jpc;

#ifdef NVM_USE_STACK_CHECK
  stack_save_sp();
#endif	
  DEBUGF("Running method %d\n", mref);

  // load method header into ram
  mhdr_ptr = nvmfile_get_method_hdr(mref);
  nvmfile_read(&mhdr, mhdr_ptr, sizeof(nvm_method_hdr_t));

  // determine method description address and code
  pc = (u08_t*)mhdr_ptr + mhdr.code_index;

  // make space for locals on the stack
  DEBUGF("Allocating space for %d local(s) and %d "
	     "stack elements - %d args\n", 
	     mhdr.max_locals, mhdr.max_stack, mhdr.args);
  
  // increase stack space. locals will be put on the stack as 
  // well. method arguments are part of the locals and are 
  // already on the stack
  heap_steal(sizeof(nvm_stack_t) * (mhdr.max_locals + mhdr.max_stack + mhdr.args));	
  // determine address of current locals (stack pointer + 1)
  current_task->j_locals = stack_get_sp() + 1;
  stack_add_sp(mhdr.max_locals);
  stack_save_base();
  
  do {
		instr_count++;
		if(instr_count == 10)
		{
			//Task_yield();
			instr_count = 0;
		}
//		jpc = (u08_t)pc;
    instr = nvmfile_read08(pc);
    pc_inc = 1;
    
    DEBUGF("%d/(sp:%d) - "DBG8" (%d): ", 
	       (pc-(u08_t*)mhdr_ptr) - mhdr.code_index, 
	       stack_get_depth(), instr, instr);
    
    // prefetch next args (in big endian order)
    arg0.z.bh = nvmfile_read08(pc+1);
    arg0.z.bl = nvmfile_read08(pc+2);

    if(instr == OP_NOP) {
      DEBUGF("nop\n");
    }
    else if(instr == OP_L2I) {
			tmpl1 = stack_pop_long();
			tmp1 = tmpl1 & 0xFFFFFFFF;
			stack_push(nvm_int2stack(tmp1));
		}
    else if(instr == OP_BIPUSH) {
      stack_push(arg0.z.bh); pc_inc = 2;
      DEBUGF("bipush #%d\n", stack_peek(0));
    } 

    else if(instr == OP_SIPUSH) {
      stack_push(~NVM_IMMEDIATE_MASK & (arg0.w)); pc_inc = 3;
      DEBUGF("sipush #"DBG16"\n", stack_peek_int(0));
    } 
    
    else if((instr >= OP_ICONST_M1) && (instr <= OP_ICONST_5)) {
      stack_push(instr - OP_ICONST_0);
      DEBUGF("iconst_%d\n", stack_peek(0));
    }

		else if((instr >= OP_LCONST_0) && (instr <= OP_LCONST_1)) {
      stack_push(instr - OP_LCONST_0);
			stack_push(0);
      DEBUGF("iconst_%d\n", stack_peek(0));
    }		
    
    // move integer from stack into locals
    else if(instr == OP_ISTORE) {
      current_task->j_locals[arg0.z.bh] = stack_pop(); pc_inc = 2;
      DEBUGF("istore %d (%d)\n", arg0.z.bh, nvm_stack2int(locals[arg0.z.bh]));
    } 
    
    // move integer from stack into locals
    else if((instr >= OP_ISTORE_0) && (instr <= OP_ISTORE_3)) {
      current_task->j_locals[instr - OP_ISTORE_0] = stack_pop();
      DEBUGF("istore_%d (%d)\n", instr - OP_ISTORE_0, nvm_stack2int(locals[instr - OP_ISTORE_0]));
    } 
		// YHC
		// move long from stack into locals
    else if(instr == OP_LSTORE) {
      current_task->j_locals[arg0.z.bh+1] = stack_pop(); 
			current_task->j_locals[arg0.z.bh  ] = stack_pop(); pc_inc = 2;
      DEBUGF("istore %d (%d)\n", arg0.z.bh, nvm_stack2int(locals[arg0.z.bh]));
    } 
    // YHC
    // move long from stack into locals
    else if((instr >= OP_LSTORE_0) && (instr <= OP_LSTORE_3)) {
      current_task->j_locals[instr - OP_LSTORE_0 + 1] = stack_pop();
			current_task->j_locals[instr - OP_LSTORE_0    ] = stack_pop();
      DEBUGF("lstore_%d (%d)\n", instr - OP_LSTORE_0, nvm_stack2int(locals[instr - OP_LSTORE_0]));
    } 

    // load int from local variable (push local var)
    else if(instr == OP_ILOAD) {
      stack_push(current_task->j_locals[arg0.z.bh]); pc_inc = 2;
      DEBUGF("iload %d (%d, "DBG_INT")\n", locals[arg0.z.bh], stack_peek_int(0), stack_peek_int(0));
    } 

    // push local onto stack
    else if((instr >= OP_ILOAD_0) && (instr <= OP_ILOAD_3)) {
      stack_push(current_task->j_locals[instr - OP_ILOAD_0]);
      DEBUGF("iload_%d (%d, "DBG_INT")\n", instr-OP_ILOAD_0, stack_peek_int(0), stack_peek_int(0));
    } 
		// YHC
		// load int from local variable (push local var)
    else if(instr == OP_LLOAD) {
      stack_push(current_task->j_locals[arg0.z.bh    ]);
			stack_push(current_task->j_locals[arg0.z.bh + 1]);			pc_inc = 2;
      DEBUGF("iload %d (%d, "DBG_INT")\n", locals[arg0.z.bh], stack_peek_int(0), stack_peek_int(0));
    } 
		// YHC
    // push local onto stack
    else if((instr >= OP_LLOAD_0) && (instr <= OP_LLOAD_3)) {
      stack_push(current_task->j_locals[instr - OP_LLOAD_0    ]);
			stack_push(current_task->j_locals[instr - OP_LLOAD_0 + 1]);
      DEBUGF("iload_%d (%d, "DBG_INT")\n", instr-OP_LLOAD_0, stack_peek_int(0), stack_peek_int(0));
    } 

    // immediate comparison / comparison with zero
    else if((instr >= OP_IFEQ) && (instr <= OP_IF_ICMPLE)) {
      DEBUGF("if");

      if((instr >= OP_IFEQ) && (instr <= OP_IFLE)) {
				// comparision with zero
				tmp2 = 0;
				instr -= OP_IFEQ - OP_IF_ICMPEQ;
      } else {
				// comparison with second argument
				DEBUGF("_cmp");
				tmp2 = stack_pop_int();
      }

      tmp1 = stack_pop_int();

      switch(instr) {
        case OP_IF_ICMPEQ: DEBUGF("eq (%d %d)", tmp1, tmp2);
          tmp1 = (tmp1 == tmp2); break;
        case OP_IF_ICMPNE: DEBUGF("ne (%d %d)", tmp1, tmp2);
          tmp1 = (tmp1 != tmp2); break;
        case OP_IF_ICMPLT: DEBUGF("lt (%d %d)", tmp1, tmp2);
          tmp1 = (tmp1 <  tmp2); break;
        case OP_IF_ICMPGE: DEBUGF("ge (%d %d)", tmp1, tmp2);
          tmp1 = (tmp1 >= tmp2); break;
        case OP_IF_ICMPGT: DEBUGF("gt (%d %d)", tmp1, tmp2);
          tmp1 = (tmp1 >  tmp2); break;
        case OP_IF_ICMPLE: DEBUGF("le (%d %d)", tmp1, tmp2);
          tmp1 = (tmp1 <= tmp2); break;
      }
      
      // change pc if jump has been taken
      if(tmp1) { DEBUGF(" -> taken\n"); pc += arg0.w; pc_inc = 0; }
      else     { DEBUGF(" -> not taken\n"); pc_inc = 3; }
    } 

    else if(instr == OP_GOTO) {
      pc_inc = 3;
      DEBUGF("goto %d\n", arg0.w); 
      pc += (arg0.w-3);
    } 

    // two operand arithmetic
    else if((instr >= OP_IADD) && (instr <= OP_IINC)) {
      // single operand arithmetic
      if(instr == OP_INEG) {
				tmp1 = -stack_pop_int();
				stack_push(nvm_int2stack(tmp1));
        DEBUGF("ineg(%d)\n", -stack_peek_int(0));

      } else if(instr == OP_LNEG) {
				tmpl1 = -stack_pop_long();
				stack_push64(nvm_long2stack(tmpl1));
        DEBUGF("lneg(%d)\n", -stack_peek_long(0));

      } else if(instr == OP_IINC) {
				DEBUGF("iinc %d,%d\n", arg0.z.bh, arg0.z.bl); 
				current_task->j_locals[arg0.z.bh] = (nvm_stack2int(current_task->j_locals[arg0.z.bh]) + arg0.z.bl) & ~NVM_IMMEDIATE_MASK; 
				pc_inc = 3;

#ifdef NVM_USE_FLOAT
      } else if(((instr & 0x03) == 0x02) && (instr <= OP_FNEG)) {
        if (instr == OP_FNEG) {
          f0 = -stack_pop_float();
          stack_push(nvm_float2stack(f0));
          DEBUGF("fneg (%f)\n", stack_peek_float(0));
        }
        else {
          f0 = stack_pop_float();  // fetch operands from stack
          f1 = stack_pop_float();
          switch(instr) {
            case OP_FADD:  DEBUGF("fadd(%f,%f)", f1, f0);
              f1  += f0; break;
            case OP_FSUB:  DEBUGF("fsub(%f,%f)", f1, f0);
              f1  -= f0; break;
            case OP_FMUL:  DEBUGF("fmul(%f,%f)", f1, f0);
              f1  *= f0; break;
            case OP_FDIV:  DEBUGF("fdiv(%f,%f)", f1, f0);
              if(!f0) error(ERROR_VM_DIVISION_BY_ZERO);
              f1  /= f0; break;
            case OP_FREM:  DEBUGF("frem(%f,%f)", f1, f0);
              error(ERROR_VM_UNSUPPORTED_OPCODE);
              //f1  = f1%f0; break;
          }
          stack_push(nvm_float2stack(f1));
          DEBUGF(" = %f\n", stack_peek_float(0));
        }
#endif

      } else if( ((instr <= OP_LNEG) && ((instr-0x61)%4 == 0)) ||								 
								 (instr == OP_LAND) || 
								 (instr == OP_LOR) || 
								 (instr == OP_LXOR)){
				tmpl1 = stack_pop_long();
				tmpl2 = stack_pop_long();
				
				switch(instr) {
          case OP_LADD:  DEBUGF("iadd(%d,%d)", tmpl2, tmpl1);
						tmpl2  += tmpl1; break;
					case OP_LSUB:  DEBUGF("isub(%d,%d)", tmpl2, tmpl1);
						tmpl2  -= tmpl1; break;
					case OP_LMUL:  DEBUGF("imul(%d,%d)", tmpl2, tmpl1);
						tmpl2  *= tmpl1; break;
					case OP_LDIV:  DEBUGF("idiv(%d,%d)", tmpl2, tmpl1);
						if(!tmpl1) error(ERROR_VM_DIVISION_BY_ZERO);
						tmpl2  /= tmpl1; break;
					case OP_LREM:  DEBUGF("irem(%d,%d)", tmpl2, tmpl1);
						tmpl2  %= tmpl1; break;
					
					case OP_LAND:  DEBUGF("iand(%d,%d)", tmpl2, tmpl1);
						tmpl2  &= tmpl1; break;
					case OP_LOR:   DEBUGF("ior(%d,%d)",  tmpl2, tmpl1);
						tmpl2  |= tmpl1; break;
					case OP_LXOR:  DEBUGF("ixor(%d,%d)", tmpl2, tmpl1);
						tmpl2  ^= tmpl1; break;					
				}
				stack_push64(nvm_long2stack(tmpl2));
        DEBUGF(" = %d\n", stack_peek_long(0));
			} else if((instr == OP_LSHR) || (instr == OP_LSHL) || (instr == OP_LUSHR)){
				tmp1 = stack_pop_int();
				tmpl2 = stack_pop_long();
				switch(instr) {
					case OP_LSHL:  DEBUGF("ishl(%d,%d)", tmpl2, tmp1);
						tmpl2 <<= tmp1; break;
					case OP_LSHR:  DEBUGF("ishr(%d,%d)", tmpl2, tmp1);
						tmpl2 >>= tmp1; break;
					case OP_LUSHR: DEBUGF("iushr(%d,%d)", tmpl2, tmp1);
						tmpl2 = ((nvm_uint_t)tmpl2 >> tmp1); break;
				}
				stack_push64(nvm_long2stack(tmpl2));
			}else {
				tmp1 = stack_pop_int();  // fetch operands from stack
				tmp2 = stack_pop_int();
	
				switch(instr) {
          case OP_IADD:  DEBUGF("iadd(%d,%d)", tmp2, tmp1);
						tmp2  += tmp1; break;
					case OP_ISUB:  DEBUGF("isub(%d,%d)", tmp2, tmp1);
						tmp2  -= tmp1; break;
					case OP_IMUL:  DEBUGF("imul(%d,%d)", tmp2, tmp1);
						tmp2  *= tmp1; break;
					case OP_IDIV:  DEBUGF("idiv(%d,%d)", tmp2, tmp1);
						if(!tmp1) error(ERROR_VM_DIVISION_BY_ZERO);
						tmp2  /= tmp1; break;
					case OP_IREM:  DEBUGF("irem(%d,%d)", tmp2, tmp1);
						tmp2  %= tmp1; break;
					case OP_ISHL:  DEBUGF("ishl(%d,%d)", tmp2, tmp1);
						tmp2 <<= tmp1; break;
					case OP_ISHR:  DEBUGF("ishr(%d,%d)", tmp2, tmp1);
						tmp2 >>= tmp1; break;
					case OP_IAND:  DEBUGF("iand(%d,%d)", tmp2, tmp1);
						tmp2  &= tmp1; break;
					case OP_IOR:   DEBUGF("ior(%d,%d)",  tmp2, tmp1);
						tmp2  |= tmp1; break;
					case OP_IXOR:  DEBUGF("ixor(%d,%d)", tmp2, tmp1);
						tmp2  ^= tmp1; break;
					case OP_IUSHR: DEBUGF("iushr(%d,%d)", tmp2, tmp1);
						tmp2 = ((nvm_uint_t)tmp2 >> tmp1); break;
				}
	
	// and finally push result
        stack_push(nvm_int2stack(tmp2));
        DEBUGF(" = %d\n", stack_peek_int(0));
      }
    }

    else if((instr == OP_IRETURN)
#ifdef NVM_USE_FLOAT
          ||(instr == OP_FRETURN)
#endif
          ||(instr == OP_RETURN)) {
      if((instr == OP_IRETURN)
#ifdef NVM_USE_FLOAT
       ||(instr == OP_FRETURN)
#endif
      ) {
				tmp1 = stack_pop();     // save result
				DEBUGF("i");
      }

      DEBUGF("return: ");

      // return from locally called method? other case: return
      // from main() -> end of program
      if(!stack_is_empty()) {
				u08_t old_locals = mhdr.max_locals;
				u08_t old_unsteal = VM_METHOD_CALL_REQUIREMENTS +	mhdr.max_locals + mhdr.max_stack + mhdr.args;
				u16_t old_localsoffset = stack_pop();
	
				// make space for locals on the stack
				DEBUGF("Return from method with %d local(s) and %d "
						 "stack elements - %d args\n", 
						 mhdr.max_locals, mhdr.max_stack, mhdr.args);
	
				mref = stack_pop();
	
				// read header of method to return to
				mhdr_ptr = nvmfile_get_method_hdr(mref);
				// load method header into ram
				nvmfile_read(&mhdr, mhdr_ptr, sizeof(nvm_method_hdr_t));
			
				// restore pc
				pc = (u08_t*)mhdr_ptr + stack_pop();
				pc_inc = 3; // continue _behind_ calling invoke instruction
	
				// and remove locals from stack and hope that method left
				// an uncorrupted stack
				stack_add_sp(-old_locals);
				current_task->j_locals = stack_get_sp() - old_localsoffset;
	
				// give memory used by returning method back to heap
				heap_unsteal(sizeof(nvm_stack_t) * old_unsteal);
	
        if(instr == OP_IRETURN){
          stack_push(tmp1);
          DEBUGF("ireturn val: %d\n", stack_peek_int(0));
        }
#ifdef NVM_USE_FLOAT
        else if(instr == OP_FRETURN){
					stack_push(tmp1);
          DEBUGF("freturn val: %f\n", stack_peek_float(0));
				}
#endif
				instr = OP_NOP;  // make vm continue
      }
    }

    // discard both top stack items
    else if(instr == OP_POP2) {
      DEBUGF("ipop\n");
      stack_pop(); stack_pop();
    }
    
    // discard top stack item
    else if(instr == OP_POP) {
      DEBUGF("pop\n");
      stack_pop();
    }
    
    // duplicate top stack item
    else if(instr == OP_DUP) {
      stack_push(stack_peek(0));
      DEBUGF("dup ("DBG16")\n", stack_peek(0) & 0xffff);
    }

    // duplicate top two stack items  (a,b -> a,b,a,b)
    else if(instr == OP_DUP2) {
      stack_push(stack_peek(1));
      stack_push(stack_peek(1));
      DEBUGF("dup2 ("DBG16","DBG16")\n", 
	     stack_peek(0) & 0xffff, stack_peek(1) & 0xffff);
    }

#ifdef NVM_USE_EXTSTACKOPS
    
    // duplicate top stack item and put it under the second
    else if(instr == OP_DUP_X1) {
      nvm_stack_t w1 = stack_pop();
      nvm_stack_t w2 = stack_pop();
      stack_push(w1);
      stack_push(w2);
      stack_push(w1);
      DEBUGF("dup_x1 ("DBG16")\n", stack_peek(0) & 0xffff);
    }

    // duplicate top stack item
    else if(instr == OP_DUP_X2) {
      nvm_stack_t w1 = stack_pop();
      nvm_stack_t w2 = stack_pop();
      nvm_stack_t w3 = stack_pop();
      stack_push(w1);
      stack_push(w2);
      stack_push(w3);
      stack_push(w1);
      DEBUGF("dup ("DBG16")\n", stack_peek(0) & 0xffff);
    }

    // duplicate top two stack items  (a,b -> a,b,a,b)
    else if(instr == OP_DUP2_X1) {
      nvm_stack_t w1 = stack_pop();
      nvm_stack_t w2 = stack_pop();
      nvm_stack_t w3 = stack_pop();
      stack_push(w1);
      stack_push(w2);
      stack_push(w3);
      stack_push(w1);
      stack_push(w2);
      DEBUGF("dup2 ("DBG16","DBG16")\n",
             stack_peek(0) & 0xffff, stack_peek(1) & 0xffff);
    }

    // duplicate top two stack items  (a,b -> a,b,a,b)
    else if(instr == OP_DUP2_X2) {
      nvm_stack_t w1 = stack_pop();
      nvm_stack_t w2 = stack_pop();
      nvm_stack_t w3 = stack_pop();
      nvm_stack_t w4 = stack_pop();
      stack_push(w1);
      stack_push(w2);
      stack_push(w3);
      stack_push(w4);
      stack_push(w1);
      stack_push(w2);
      DEBUGF("dup2 ("DBG16","DBG16")\n",
             stack_peek(0) & 0xffff, stack_peek(1) & 0xffff);
    }
    
    // swap top two stack items  (a,b -> b,a)
    else if(instr == OP_SWAP) {
      nvm_stack_t w1 = stack_pop();
      nvm_stack_t w2 = stack_pop();
      stack_push(w1);
      stack_push(w2);
      DEBUGF("swap ("DBG16","DBG16")\n", stack_peek(0), stack_peek(1));
    }
    
#endif
    
    
#ifdef NVM_USE_TABLESWITCH
    else if(instr == OP_TABLESWITCH) {
      DEBUGF("TABLESWITCH\n");
      // padding was eliminated by generator
      tmp1 = ((nvmfile_read08(pc+7)<<8) |
	      nvmfile_read08(pc+8));        // get low value
      tmp2 = ((nvmfile_read08(pc+11)<<8) |
	      nvmfile_read08(pc+12));       // get high value
      arg0.tmp = stack_pop();               // get actual value
      DEBUGF("tableswitch %d-%d (%d)\n", tmp1, tmp2, arg0.w);
      
      // value within range?
      if((arg0.tmp < tmp1)||(arg0.tmp > tmp2))
	// no: use default
				tmp2 = 3;
      else
	// yes: get offset from table
				tmp2 = 3 + 12 + ((arg0.tmp - tmp1)<<2);
      
      // and do the jump
      pc += ((nvmfile_read08(pc+tmp2+0)<<8) | nvmfile_read08(pc+tmp2+1));
      pc_inc = 0;
    }
#endif
    
#ifdef NVM_USE_LOOKUPSWITCH
    else if(instr == OP_LOOKUPSWITCH) {			
			u08_t size;
      DEBUGF("LOOKUPSWITCH\n");
      // padding was eliminated by generator
     
      arg0.tmp = 1 + 4;
      size = nvmfile_read08(pc+arg0.tmp+3); // get table size (max for nvm is 30 cases!)
      DEBUGF("  size: %d\n", size);
      arg0.tmp += 4;
      
      tmp1 = stack_pop_int();                        // get actual value
      DEBUGF("  val=: %d\n", tmp1);
      
      while(size)
      {
        if (
#ifdef NVM_USE_32BIT_WORD
             nvmfile_read08(pc+arg0.tmp+0)==(u08_t)(tmp1>>24) &&
             nvmfile_read08(pc+arg0.tmp+1)==(u08_t)(tmp1>>16) &&
#endif
             nvmfile_read08(pc+arg0.tmp+2)==(u08_t)(tmp1>>8) &&
             nvmfile_read08(pc+arg0.tmp+3)==(u08_t)(tmp1>>0)
           )
        {
          DEBUGF("  value found, index is %d\n", (int)(arg0.tmp-pc_inc-8)/8);
          arg0.tmp+=4;
          break;
        }
        arg0.tmp+=8;
        size--;
      }
      
      if (size==0)
      {
        DEBUGF("  not found, using default!\n");
        arg0.tmp = 1;
      }
      pc += ((nvmfile_read08(pc+arg0.tmp+2)<<8) |
             nvmfile_read08(pc+arg0.tmp+3));
      pc_inc = 0;
    }
#endif

    // get static field from class
    else if(instr == OP_GETSTATIC) {
      pc_inc = 3;   // prefetched data used
      DEBUGF("getstatic #"DBG16"\n", arg0.w);
      stack_push(stack_get_static(arg0.w));
    }
    
    else if(instr == OP_PUTSTATIC) {
      pc_inc = 3;
      stack_set_static(arg0.w, stack_pop());
      DEBUGF("putstatic #"DBG16" -> "DBG16"\n", 
	     arg0.w, stack_get_static(arg0.w));
    }
    
    // push item from constant pool
    else if(instr == OP_LDC) {
      pc_inc = 2;
      DEBUGF("ldc #"DBG16"\n", arg0.z.bh);
#ifdef NVM_USE_32BIT_WORD
      stack_push(nvmfile_get_constant(arg0.z.bh));
#else
      stack_push(NVM_TYPE_CONST | (arg0.z.bh-nvmfile_constant_count));
#endif
    }
		else if(instr == OP_LDC_W) {
			pc_inc = 3;
			DEBUGF("ldc2_w #"DBG32"\n", (arg0.z.bh << 8) | arg0.z.bl);
			stack_push(nvmfile_get_constant((arg0.z.bh << 8) | arg0.z.bl));
		}
		else if(instr == OP_LDC2_W) {
			pc_inc = 3;
			DEBUGF("ldc2_w #"DBG32"\n", (arg0.z.bh << 8) | arg0.z.bl);
			stack_push64(nvmfile_get_constant64((arg0.z.bh << 8) | arg0.z.bl));
		}
    
    else if((instr >= OP_INVOKEVIRTUAL)&&(instr <= OP_INVOKESTATIC)) {
      DEBUGF("invoke");

#ifdef DEBUG
      if(instr == OP_INVOKEVIRTUAL) { DEBUGF("virtual"); }
      if(instr == OP_INVOKESPECIAL) { DEBUGF("special"); }
      if(instr == OP_INVOKESTATIC)  { DEBUGF("static"); }
#endif

      DEBUGF(" #"DBG16"\n", 0xffff & arg0.w);
      
      // invoke a method. check if it's local (within the nvm file)
      // or native (implemented by the runtime environment)
      if(arg0.z.bh < NATIVE_CLASS_BASE) {
				DEBUGF("local method call from method %d to %d\n", mref, arg0.w);

				// save current pc (relative to method start)
				tmp1 = (u08_t*)pc-(u08_t*)mhdr_ptr;
	
				// get pointer to new method
				mhdr_ptr = nvmfile_get_method_hdr(arg0.w);
	
				// load new method header into ram
				nvmfile_read(&mhdr, mhdr_ptr, sizeof(nvm_method_hdr_t));
	
#ifdef NVM_USE_INHERITANCE
				// check class on stack. it may be not the one we expect.
				// this happens due to inheritance
				if(instr == OP_INVOKEVIRTUAL) { 
					nvm_ref_t mref;
					DEBUGF("checking inheritance\n");

					// fetch class reference from stack and use it to address
					// the class instance on the heap. The first entry in this 
					// object is the class id of it
					mref = ((nvm_ref_t*)heap_get_addr(stack_peek(0) & ~NVM_TYPE_MASK))[0];
					DEBUGF("class ref on stack/ref: %d/%d\n", 
							 NATIVE_ID2CLASS(mref), NATIVE_ID2CLASS(mhdr.id));

					if(NATIVE_ID2CLASS(mref) != NATIVE_ID2CLASS(mhdr.id)) {
						DEBUGF("stack/ref class mismatch -> inheritance\n");

						// get matching method in class on stack or its
						// super classes
						arg0.z.bl = nvmfile_get_method_by_class_and_id(
							NATIVE_ID2CLASS(mref), NATIVE_ID2METHOD(mhdr.id));

						// get pointer to new method
						mhdr_ptr = nvmfile_get_method_hdr(arg0.z.bl);
				
						// load new method header into ram
						nvmfile_read(&mhdr, mhdr_ptr, sizeof(nvm_method_hdr_t));
					}
				}
#endif
				
				// arguments are left on the stack by the calling
				// method and expected in the locals by the called
				// method. Thus we make this part of the old stack
				// be the locals part of the method
				DEBUGF("Remove %d args from stack\n", mhdr.args);
				stack_add_sp(-mhdr.args);
				
				tmp2 = stack_get_sp() - current_task->j_locals;
				
				current_task->j_locals = stack_get_sp() + 1;
				
#ifdef DEBUG
				if(instr == OP_INVOKEVIRTUAL) { 
					DEBUGF("virtual call with object reference "DBG16"\n",
							 locals[0]); 
				}
#endif

				// make space for locals on the stack
				DEBUGF("Allocating space for %d local(s) and %d "
						 "stack elements - %d args\n", 
						 mhdr.max_locals, mhdr.max_stack, mhdr.args);
				
				// increase stack space. locals will be put on the stack as 
				// well. method arguments are part of the locals and are 
				// already on the stack
				heap_steal(sizeof(nvm_stack_t) *
						 (VM_METHOD_CALL_REQUIREMENTS +
							mhdr.max_locals + mhdr.max_stack + mhdr.args));
				
				// add space for locals on stack
				stack_add_sp(mhdr.max_locals);
				
				// push everything required to return onto the stack
				stack_push(tmp1);   // pc offset
				stack_push(mref);   // method reference
				stack_push(tmp2);   // locals offset
				
				// set new pc (this is the actual call)
				mref = arg0.w;
				pc = (u08_t*)mhdr_ptr + mhdr.code_index;
				pc_inc = 0;  // don't add further bytes to program counter
      } else { 
				native_invoke(arg0.w);
				pc_inc = 3;   // prefetched data used
      }
    }
    
    else if(instr == OP_GETFIELD) {
      pc_inc = 3;
      DEBUGF("getfield #%d\n", arg0.w);
      stack_push(((nvm_word_t*)heap_get_addr(stack_pop() & ~NVM_TYPE_MASK))
	      [VM_CLASS_CONST_ALLOC+arg0.w]);
    }
    
    else if(instr == OP_PUTFIELD) {
      pc_inc = 3;
      tmp1 = stack_pop();
      
      DEBUGF("putfield #%d\n", arg0.w);
      ((nvm_word_t*)heap_get_addr(stack_pop() & ~NVM_TYPE_MASK))
			[VM_CLASS_CONST_ALLOC+arg0.w] = tmp1;
    }
    
    else if(instr == OP_NEW) {
      pc_inc = 3;
      DEBUGF("new #"DBG16"\n", 0xffff & arg0.w);
      vm_new(arg0.w);
    }
    
#ifdef NVM_USE_ARRAY
    else if(instr == OP_NEWARRAY) {
      pc_inc = 2;
      stack_push(array_new(stack_pop(), arg0.z.bh) | NVM_TYPE_HEAP);
    }
    
    else if(instr == OP_ARRAYLENGTH) {
      stack_push(array_length(stack_pop() & ~NVM_TYPE_MASK));
    }
    
    else if(instr == OP_BASTORE) {
      tmp2 = stack_pop_int();       // value
      tmp1 = stack_pop_int();         // index
      // third parm on stack: array reference
      array_bastore(stack_pop() & ~NVM_TYPE_MASK, tmp1, tmp2);
    }
    
    else if(instr == OP_IASTORE) {
      tmp2 = stack_pop_int();       // value
      tmp1 = stack_pop_int();       // index
      // third parm on stack: array reference
      array_iastore(stack_pop() & ~NVM_TYPE_MASK, tmp1, tmp2);
    }
    
    else if(instr == OP_BALOAD) {
      tmp1 = stack_pop_int();       // index
      // second parm on stack: array reference
      stack_push(array_baload(stack_pop() & ~NVM_TYPE_MASK, tmp1));
    }
    
    else if(instr == OP_IALOAD) {
      tmp1 = stack_pop_int();       // index
      // second parm on stack: array reference
      stack_push(array_iaload(stack_pop() & ~NVM_TYPE_MASK, tmp1));
    }
#endif

#ifdef NVM_USE_OBJ_ARRAY
    else if(instr == OP_ANEWARRAY) {
      // Object array is the same as int array...
      pc_inc = 3;
      stack_push(array_new(stack_pop(), T_INT) | NVM_TYPE_HEAP);
    }
    
    else if(instr == OP_AASTORE) {
      tmp2 = stack_pop_int();       // value
      tmp1 = stack_pop_int();       // index
      // third parm on stack: array reference
      array_iastore(stack_pop(), tmp1, tmp2);
    }
    
    else if(instr == OP_AALOAD) {
      tmp1 = stack_pop_int();       // index
      // second parm on stack: array reference
      stack_push(array_iaload(stack_pop(), tmp1));
    }
#endif

#ifdef NVM_USE_FLOAT
# ifdef NVM_USE_ARRAY
    else if(instr == OP_FALOAD) {
      tmp1 = stack_pop_int();       // index
      // second parm on stack: array reference
      stack_push(array_faload(stack_pop() & ~NVM_TYPE_MASK, tmp1));
    }
    else if(instr == OP_FASTORE) {
      f0 = stack_pop_float();       // value
      tmp1 = stack_pop_int();         // index
      // third parm on stack: array reference
      array_fastore(stack_pop() & ~NVM_TYPE_MASK, tmp1, f0);
    }
# endif

    else if(instr == OP_FCONST_0) {
      stack_push(nvm_float2stack(0.0));
      DEBUGF("fconst_%d\n", stack_peek_float(0));
    }
    else if(instr == OP_FCONST_1) {
      stack_push(nvm_float2stack(1.0));
      DEBUGF("fconst_%d\n", stack_peek_float(0));
    }
    else if(instr == OP_FCONST_2) {
      stack_push(nvm_float2stack(2.0));
      DEBUGF("fconst_%d\n", stack_peek_float(0));
    }
    else if(instr == OP_I2F) {
      tmp1 = stack_pop_int();
      stack_push(nvm_float2stack(tmp1));
      DEBUGF("i2f %f\n", stack_peek_float(0));
    }
    else if(instr == OP_F2I) {
      tmp1 = stack_pop_float();
      stack_push(nvm_int2stack(tmp1));
      DEBUGF("i2f %f\n", stack_peek_int(0));
    }
    
    // move float from stack into locals
    else if(instr == OP_FSTORE) {
      current_task->j_locals[arg0.z.bh] = stack_pop(); pc_inc = 2;
      DEBUGF("fstore %d (%f)\n", arg0.z.bh, nvm_stack2float(locals[arg0.z.bh]));
    } 
    
    // move integer from stack into locals
    else if((instr >= OP_FSTORE_0) && (instr <= OP_FSTORE_3)) {
      current_task->j_locals[instr - OP_FSTORE_0] = stack_pop();
      DEBUGF("fstore_%d (%f)\n", instr - OP_FSTORE_0, nvm_stack2float(locals[instr - OP_FSTORE_0]));
    } 

    // load float from local variable (push local var)
    else if(instr == OP_FLOAD) {
      stack_push(current_task->j_locals[arg0.z.bh]); pc_inc = 2;
      DEBUGF("fload %d (%f, "DBG16")\n", locals[arg0.z.bh], stack_peek_float(0), stack_peek_int(0));
    } 

    // push local onto stack
    else if((instr >= OP_FLOAD_0) && (instr <= OP_FLOAD_3)) {
      stack_push(current_task->j_locals[instr - OP_FLOAD_0]);
      DEBUGF("fload_%d (%f, "DBG16")\n", instr-OP_FLOAD_0, stack_peek_float(0), stack_peek_int(0));
    }
    
    // compare top values on stack
    else if((instr == OP_FCMPL) || (instr == OP_FCMPG)) {
      f1 = stack_pop_float();
      f0 = stack_pop_float();
      tmp1=0;
      if (f0<f1)
        tmp1=-1;
      else if (f0>f1)
        tmp1=1;
      stack_push(nvm_int2stack(tmp1));
      DEBUGF("fcmp%c (%f, %f, %i)\n", (instr==OP_FCMPL)?'l':'g', f0, f1, stack_peek_int(0));
    }
#endif
		else if(instr == OP_ATHROW)
		{
			nvm_ref_t ref;
			u32_t temp_addr;
			u08_t *exc_st,*exc_ed,*exc_hdl;
			u16_t exc_cat,exctable_idx;
			
			ref = stack_peek(0);
reathrow:
			exctable_idx = mhdr.exception_index;
			while(exctable_idx < mhdr.code_index)
			{
				temp_addr = (u32_t)mhdr_ptr + mhdr.code_index;
				temp_addr += (u32_t)(*(u08_t*)((u32_t)mhdr_ptr + exctable_idx + 0));
				temp_addr += (u32_t)((*(u08_t*)((u32_t)mhdr_ptr + exctable_idx + 1)) << 8);
				exc_st = (u08_t*)temp_addr;
				
				temp_addr = (u32_t)mhdr_ptr + mhdr.code_index;
				temp_addr += (u32_t)(*(u08_t*)((u32_t)mhdr_ptr + exctable_idx + 2));
				temp_addr += (u32_t)((*(u08_t*)((u32_t)mhdr_ptr + exctable_idx + 3)) << 8);
				exc_ed = (u08_t*)temp_addr;
				
				temp_addr = (u32_t)mhdr_ptr + mhdr.code_index;
				temp_addr += (u32_t)(*(u08_t*)((u32_t)mhdr_ptr + exctable_idx + 4));
				temp_addr += (u32_t)((*(u08_t*)((u32_t)mhdr_ptr + exctable_idx + 5)) << 8);
				exc_hdl = (u08_t*)temp_addr;
				
				exc_cat = (u32_t)(*(u08_t*)((u32_t)mhdr_ptr + exctable_idx + 6));
				exc_cat += (u32_t)((*(u08_t*)((u32_t)mhdr_ptr + exctable_idx + 7)) << 8);
				
				if((exc_cat == 0) || (ref == exc_cat))
				{
					if((exc_st <= pc) && (pc < exc_ed))
					{
						pc = exc_hdl;
						pc_inc = 0;
						break;
					}
				}
				exctable_idx += 8;
			}
			if(exctable_idx >= mhdr.code_index)
			{
				ref = stack_pop();
				if(!stack_is_empty()) {
					u08_t old_locals = mhdr.max_locals;
					u08_t old_unsteal = VM_METHOD_CALL_REQUIREMENTS +	mhdr.max_locals + mhdr.max_stack + mhdr.args;
					u16_t old_localsoffset = stack_pop();
		
					// make space for locals on the stack
					DEBUGF("Return from method with %d local(s) and %d "
							 "stack elements - %d args\n", 
							 mhdr.max_locals, mhdr.max_stack, mhdr.args);
		
					mref = stack_pop();
		
					// read header of method to return to
					mhdr_ptr = nvmfile_get_method_hdr(mref);
					// load method header into ram
					nvmfile_read(&mhdr, mhdr_ptr, sizeof(nvm_method_hdr_t));
				
					// restore pc					
					pc = (u08_t*)mhdr_ptr + stack_pop();
					pc_inc = 3; // continue _behind_ calling invoke instruction
		
					// and remove locals from stack and hope that method left
					// an uncorrupted stack
					stack_add_sp(-old_locals);
					current_task->j_locals = stack_get_sp() - old_localsoffset;
		
					// give memory used by returning method back to heap
					heap_unsteal(sizeof(nvm_stack_t) * old_unsteal);
		
					stack_push(ref);
					goto reathrow;
/*					if(instr == OP_IRETURN){
						stack_push(tmp1);
						DEBUGF("ireturn val: %d\n", stack_peek_int(0));
					}
	#ifdef NVM_USE_FLOAT
					else if(instr == OP_FRETURN){
						stack_push(tmp1);
						DEBUGF("freturn val: %f\n", stack_peek_float(0));
					}
	#endif*/
					//instr = OP_NOP;  // make vm continue
				}				
			}
		}
		else if(instr == OP_MONITORENTER)
		{
			heap_id_t h = stack_pop() & ~NVM_TYPE_HEAP;
			MutexLock(&mutex_key[h]);
		}
		else if(instr == OP_MONITOREXIT)
		{
			heap_id_t h = stack_pop() & ~NVM_TYPE_HEAP;			
			MutexUnlock(&mutex_key[h]);
		}
    else {
      error(ERROR_VM_UNSUPPORTED_OPCODE);
    }
    
    // reset watchdog here if present

    pc += pc_inc;
  } while((instr != OP_IRETURN)&&(instr != OP_RETURN));

  // and remove locals from stack and hope that method left
  // an uncorrupted stack
  stack_add_sp(-mhdr.max_locals);

#ifdef NVM_USE_STACK_CHECK
  stack_verify_sp();
#endif

  // give memory back to heap
  heap_unsteal(sizeof(nvm_stack_t) * (mhdr.max_locals + mhdr.max_stack + mhdr.args));
	
	return 0;
}

