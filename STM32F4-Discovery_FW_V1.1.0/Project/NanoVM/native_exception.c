#include "types.h"
#include "debug.h"
#include "config.h"
#include "error.h"
#include "stack.h"
#include "native.h"
#include "native_formatter.h"
#include "nvmstring.h"

#define NATIVE_METHOD_EXCEPTION_INIT	0

void native_exception_invoke(u08_t mref) {
	if(mref == NATIVE_METHOD_EXCEPTION_INIT) {
		stack_pop(); //remove ref
		stack_push(NATIVE_CLASS_EXCEPTION<<8);
	}
}
