/* tak_bc.c */

typedef const unsigned char *code_ptr_t;

union stack_slot {
    int val;
    code_ptr_t ra;
};
typedef union stack_slot *stack_ptr_t;

#define DROP(n)			do { sp += n; } while(0)
#define GET_INT(n, x)		do { x = sp[n].val; } while(0)
#define SET_INT(n, x)		do { sp[n].val = x; } while(0)
#define POP_INT(x)		do { x = (sp++)->val; } while(0)
#define PUSH_INT(x)		do { (--sp)->val = x; } while(0)
#define POP_RA(x)		do { x = (sp++)->ra; } while(0)
#define PUSH_RA(x)		do { (--sp)->ra = x; } while(0)

#include <stdio.h>

#if DEBUG
#define DPRINTF(fmt, args...)	printf(fmt, ##args)
#else
#define DPRINTF(fmt, args...)
#endif

#include "tak_bytecode.h"

#define NEXT		continue

static void run(code_ptr_t pc0)
{
    union stack_slot stack[1024];
    union stack_slot *sp;
    code_ptr_t pc;

    sp = &stack[1024];
    pc = pc0;

    for(;;) {
	switch (*pc++) {
	  case OP_GET:
	    {
		unsigned int n;
		int x;
		n = *pc++;
		GET_INT(n, x);
		PUSH_INT(x);
		NEXT;
	    }
	  case OP_SET:
	    {
		unsigned int n;
		int x;
		n = *pc++;
		POP_INT(x);
		SET_INT(n, x);
		NEXT;
	    }
	  case OP_SUB1:
	    {
		int x;
		GET_INT(0, x);
		--x;
		SET_INT(0, x);
		NEXT;
	    }
	  case OP_RET:
	    {
		unsigned int n;
		int rv;
		n = *pc;
		POP_INT(rv);
		POP_RA(pc);
		DROP(n);
		PUSH_INT(rv);
		NEXT;
	    }
	  case OP_CALL:
	    {
		int off;
		PUSH_RA(pc+1);
		off = *(signed char*)pc;
		pc += off;
		NEXT;
	    }
	  case OP_JMP:
	    {
		int off;
		off = *(signed char*)pc;
		pc += off;
		NEXT;
	    }
	  case OP_JGT:
	    {
		int x, y;
		POP_INT(x);
		POP_INT(y);
		if (x > y) {
		    int off;
		    off = *(signed char*)pc;
		    pc += off;
		} else
		    ++pc;
		NEXT;
	    }
	  case OP_PUSH:
	    {
		int x;
		x = *pc++;
		PUSH_INT(x);
		NEXT;
	    }
	  case OP_POP:
	    {
		DROP(1);
		NEXT;
	    }
	  case OP_DEBUG:
	    {
		printf("%d %d %d\n", sp[1].val, sp[2].val, sp[3].val);
		NEXT;
	    }
	  case OP_HALT:
	    {
		DPRINTF("sp == %p, &stack[1024] == %p\n", sp, &stack[1024]);
		return;
	    }
	}
    }
}

static void doit(void)
{
    run(&tak_bc[0]);
}

#if STANDALONE
int main(void)
{
    doit();
    return 0;
}
#else
#include "interpreter.h"

void prepare_code(void)
{
}

void run_code(void)
{
    doit();
}
#endif
