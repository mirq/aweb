/* Automatically generated file! Do not edit! */

#include "clib/awebjs_protos.h"

#include <emul/emulregs.h>

#include <stdarg.h>

struct Jobject * Addjfunction(struct Jcontext * jc, struct Jobject * jo, UBYTE * name,
	void (*code)(struct Jcontext *), ...)
{
	va_list va;
	va_start(va, code);

	return AddjfunctionA(jc, jo, name, code, va->overflow_arg_area);
}
