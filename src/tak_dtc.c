/* tak_dtc.c */

union code_slot {
    const void **label;
    int val;
    const union code_slot *npc;
};
typedef const union code_slot *code_ptr_t;

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
#include <stdlib.h>

#if DEBUG
#define DPRINTF(fmt, args...)	printf(fmt, ##args)
#else
#define DPRINTF(fmt, args...)
#endif

#include "tak_bytecode.h"

#define NEXT		goto *((pc++)->label)
#define DISPATCH	NEXT

static void run(code_ptr_t pc0)
{
    union stack_slot stack[1024];
    union stack_slot *sp;
    code_ptr_t pc;
    static const void ** const jtab[11] __asm__("_jtab") __attribute__((unused)) = {
	[OP_GET] &&lab_op_get,
	[OP_SET] &&lab_op_set,
	[OP_SUB1] &&lab_op_sub1,
	[OP_RET] &&lab_op_ret,
	[OP_CALL] &&lab_op_call,
	[OP_JMP] &&lab_op_jmp,
	[OP_JGT] &&lab_op_jgt,
	[OP_PUSH] &&lab_op_push,
	[OP_POP] &&lab_op_pop,
	[OP_DEBUG] &&lab_op_debug,
	[OP_HALT] &&lab_op_halt,
    };

    sp = &stack[1024];
    pc = pc0;

    for(;;) {
	DISPATCH;
    lab_op_get:
	{
	    unsigned int n;
	    int x;
	    n = (pc++)->val;
	    GET_INT(n, x);
	    PUSH_INT(x);
	    NEXT;
	}
    lab_op_set:
	{
	    unsigned int n;
	    int x;
	    n = (pc++)->val;
	    POP_INT(x);
	    SET_INT(n, x);
	    NEXT;
	}
    lab_op_sub1:
	{
	    int x;
	    GET_INT(0, x);
	    --x;
	    SET_INT(0, x);
	    NEXT;
	}
    lab_op_ret:
	{
	    unsigned int n;
	    int rv;
	    n = pc->val;
	    POP_INT(rv);
	    POP_RA(pc);
	    DROP(n);
	    PUSH_INT(rv);
	    NEXT;
	}
    lab_op_call:
	{
	    PUSH_RA(pc+1);
	    pc = pc->npc;
	    NEXT;
	}
    lab_op_jmp:
	{
	    pc = pc->npc;
	    NEXT;
	}
    lab_op_jgt:
	{
	    int x, y;
	    POP_INT(x);
	    POP_INT(y);
	    if (x > y) {
		pc = pc->npc;
	    } else
		++pc;
	    NEXT;
	}
    lab_op_push:
	{
	    int x;
	    x = (pc++)->val;
	    PUSH_INT(x);
	    NEXT;
	}
    lab_op_pop:
	{
	    DROP(1);
	    NEXT;
	}
    lab_op_debug:
	{
	    printf("%d %d %d\n", sp[1].val, sp[2].val, sp[3].val);
	    NEXT;
	}
    lab_op_halt:
	{
	    DPRINTF("sp == %p, &stack[1024] == %p\n", sp, &stack[1024]);
	    return;
	}
    }
}

static code_ptr_t compile(const unsigned char *bc, unsigned int bc_size)
{
    int pc;
    int off, npc;
    union code_slot *dtc;
    extern const void ** const _jtab[];

    dtc = malloc(bc_size * sizeof(dtc[0]));

    pc = 0;
    while (pc < bc_size) {
	dtc[pc].label = _jtab[bc[pc]];
	switch (bc[pc]) {
	  case OP_GET: case OP_SET: case OP_RET: case OP_PUSH:
	    /* Copying one immediate operand. */
	    dtc[pc+1].val = bc[pc+1];
	    pc += 2;
	    break;
	  case OP_CALL: case OP_JMP: case OP_JGT:
	    /* Converting from signed offset to direct pointer. */
	    off = *(signed char*)&bc[pc+1];
	    npc = pc + 1 + off;
	    if (npc < 0 || npc >= tak_bc_size)
		abort();
	    dtc[pc+1].npc = &dtc[npc];
	    pc += 2;
	    break;
	  case OP_SUB1: case OP_HALT: case OP_DEBUG: case OP_POP:
	    pc += 1;
	    break;
	  default:
	    abort();
	}
    }
    return &dtc[0];
}

#if STANDALONE
int main(void)
{
    code_ptr_t dtc = compile(tak_bc, tak_bc_size);
    run(dtc);
    return 0;
}
#else
#include "interpreter.h"

static code_ptr_t dtc;

void prepare_code(void)
{
    dtc = compile(tak_bc, tak_bc_size);
}

void run_code(void)
{
    run(dtc);
}
#endif
