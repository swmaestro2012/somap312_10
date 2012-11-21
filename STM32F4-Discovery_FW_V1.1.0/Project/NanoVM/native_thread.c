#include "types.h"
#include "debug.h"
#include "config.h"
#include "error.h"
#include "stack.h"
#include "native.h"
#include "native_formatter.h"
#include "nvmstring.h"
#include "kernel.h"


void native_thread_init(u08_t mref)
{
	heap_id_t h;
	h = heap_alloc(TRUE, sizeof(nvm_word_t));
	stack_push(NVM_TYPE_HEAP | h);

	// store reference in object, so we can later determine which kind
	// of object this is. this is required for inheritance
	((nvm_ref_t*)heap_get_addr(h))[0] = SubThread_idx++;
	/*heap_id_t h;
	u16_t methodidx;
	h = stack_pop();
	methodidx = ((nvm_ref_t*)heap_get_addr(h))[0];
	
	//Task_init(&MainThread, Maintread_invoke, 0, CONFIG_MAX_PRIORITY - 1,MainThread_stack, sizeof(MainThread_stack));
	Task_init(&SubThread[SubThread_idx],vm_run, (void*)(methodidx&0xFF), CONFIG_MAX_PRIORITY - 1, SubThread_stack[SubThread_idx], sizeof(SubThread_stack[SubThread_idx]));
	//SubThread[5];
	//SubThread_stack[5][512];
	*/
}

void native_thread_invoke(u08_t mref) {	
	heap_id_t method_h,thread_h;
	u16_t method_id,thread_id;		
	if(mref == NATIVE_METHOD_THREAD_RUNNABLE)
	{
//		nvm_method_hdr_t mhdr, *mhdr_ptr;
		method_h = stack_pop();
		thread_h = stack_pop();
		
		method_id = ((nvm_ref_t*)heap_get_addr(method_h))[0];
		thread_id = ((nvm_ref_t*)heap_get_addr(thread_h))[0];
		Task_init(&SubThread[thread_id],vm_run, (void*)(method_id&0xFF), CONFIG_MAX_PRIORITY - 1, SubThread_stack[thread_id], sizeof(SubThread_stack[thread_id]));
				
//		mhdr_ptr = nvmfile_get_method_hdr(method_id&0xFF);	
		// load new method header into ram
//		nvmfile_read(&mhdr, mhdr_ptr, sizeof(nvm_method_hdr_t));
//		heap_steal(sizeof(nvm_stack_t) * (mhdr.max_locals + mhdr.max_stack + mhdr.args));
//		Task_setjsp(mhdr.
		//stack_push(NVM_TYPE_HEAP | thread_h);
	}
	else if(mref == NATIVE_METHOD_THREAD_START) {
		thread_h = stack_pop();		
		thread_id = ((nvm_ref_t*)heap_get_addr(thread_h))[0];
		SubThread[thread_id].j_sp = (unsigned int*)&heap[heap_base];
		SubThread[thread_id].j_stack = (unsigned int*)&heap[heap_base];
		Task_start(&SubThread[thread_id]);
		//Task_yield();
	}
}
