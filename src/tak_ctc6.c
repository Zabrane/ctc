/* tak_ctc6.c
 * tak_ctc5 + optional SAVE/RESTORE RA used only when needed
 */

typedef const void **code_ptr_t;

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

enum opcode {
    OP_GET,
    OP_SET,
    OP_SUB1,
    OP_RET,
    OP_PUSH_RA,
    OP_JGT,
    OP_PUSH,
    OP_POP,
    OP_DEBUG,
    OP_HALT,
};

#if defined(__i386__)
/* things break if GET_ESP() is an inline function */
#define GET_ESP()	({ register char *_esp asm("%esp"); _esp; })
#define GET1(TY)	({ register TY _a1 asm("%eax"); _a1; })
#define GET2(TY)	({ register TY _a2 asm("%edx"); _a2; })
#define __opcode	__attribute__((regparm(1)))
/* The implicit SAVE_RA will be optimised away if there's no explicit RESTORE_RA. */
#define SAVE_RA		unsigned long _ra = *(unsigned long*)GET_ESP()
#define RESTORE_RA	do { *(unsigned long*)GET_ESP() = _ra; } while(0)
#define NEXT0		asm("ret" : :) /* dummy :: prevents merging and jump-to-ret */
#define RESTORE_NEXT0	do { RESTORE_RA; NEXT0; } while(0)
#define NEXT1(X)	do { register long _a1 asm("%eax"); _a1 = (long)(X); asm("ret" : : "r"(_a1)); } while(0)
#endif

#if defined(__x86_64__)
/* things break if GET_RSP() is an inline function */
#define GET_RSP()	({ register char *_rsp asm("%rsp"); _rsp; })
#define GET1(TY)	({ register TY _a1 asm("%rdi"); _a1; })
#define GET2(TY)	({ register TY _a2 asm("%rsi"); _a2; })
#define __opcode	/*empty*/
#define SAVE_RA		unsigned long _ra = *(unsigned long*)GET_RSP()
#define RESTORE_RA	do { *(unsigned long*)GET_RSP() = _ra; } while(0)
#define NEXT0		asm("ret" : :) /* dummy :: prevents merging and jump-to-ret */
#define RESTORE_NEXT0	do { RESTORE_RA; NEXT0; } while(0)
#define NEXT1(X)	do { register long _rv asm("%rax"); _rv = (long)(X); asm("ret" : : "r"(_rv)); } while(0)
#endif

#if defined(__powerpc__)
#define GET1(TY)	({ register TY _a1 asm("3"); _a1; })
#define GET2(TY)	({ register TY _a2 asm("4"); _a2; })
#define __opcode	/*empty*/
static inline unsigned long get_lr(void)
{
    unsigned long lr;
    /* without this volatile gcc can schedule the mflr past
       code (recursive calls) that clobber lr */
    /* the volatile prevents gcc from removing the mflr when
       there is no RESTORE_RA, so SAVE_RA removal must be
       programmed explicitly */
    asm volatile("mflr %0" : "=r"(lr));
    return lr;
}
#define SAVE_RA		unsigned long _ra = get_lr()
#define RESTORE_RA	asm("mtlr %0" : : "r"(_ra) : "lr")
#define NEXT0		asm("blr")
#define RESTORE_NEXT0	do { RESTORE_RA; NEXT0; } while(0)
#define NEXT1(X)	do { register long _rv asm("3") = (long)(X); asm("blr" : : "r"(_rv)); } while(0)
#endif

#if defined(__sparc__)
#define GET1(TY)	({ register TY _o0 asm("%o0"); _o0; })
#define GET2(TY)	({ register TY _o1 asm("%o1"); _o1; })
#define __opcode	/*empty*/
#define get_o7()	({ register unsigned long _o7 asm("%o7"); _o7; })
#define SAVE_RA		unsigned long _ra = get_o7()
#define NEXT0		asm("retl; nop" : :)
#define RESTORE_NEXT0	do { register unsigned long _o7 asm("%o7") = _ra; asm("retl; nop" : : "r"(_o7)); } while(0)
#define NEXT1(X)	do { register unsigned long _rv asm("%o0") = (unsigned long)(X); asm("retl; nop" : : "r"(_rv)); } while(0)
#endif

#if defined(__arm__)
#define GET1(TY)	({ register TY _a1 asm("%r0"); _a1; })
#define GET2(TY)	({ register TY _a2 asm("%r1"); _a2; })
#define __opcode	/*empty*/
#define get_r14()	({ register unsigned long _r14 asm("%r14"); _r14; })
#define SAVE_RA		unsigned long _ra = get_r14()
#define NEXT0		asm("mov pc, lr" : :)
#define RESTORE_NEXT0	do { register unsigned long _r14 asm("%r14") = _ra; asm("mov pc, lr" : : "r"(_r14)); } while(0)
#define NEXT1(X)	do { register unsigned long _r0 asm("%r0") = (unsigned long)(X); asm("mov pc, lr" : : "r"(_r0)); } while(0)
#endif

#define ENTRY(NAME)	\
	NAME:		\
	__asm__("\n" #NAME ":");

#define NOSAVE_RA	/*empty*/

#define BEGIN_ENTRY0(NAME,PROLOGUE)	\
	ENTRY(NAME) { PROLOGUE;
#define BEGIN_ENTRY1(NAME,TY,X,PROLOGUE)	\
	ENTRY(NAME) { PROLOGUE; TY X = GET1(TY);
#define BEGIN_ENTRY2(NAME,TY1,X1,TY2,X2,PROLOGUE)	\
	ENTRY(NAME) { PROLOGUE; TY1 X1 = GET1(TY1); TY2 X2 = GET2(TY2);
#define END_ENTRY	\
	}

static void run(code_ptr_t pc0)
{
    union stack_slot stack[1024];
    union stack_slot *sp;
    static const void ** const jtab[11] __asm__("_jtab") __attribute__((unused)) = {
	[OP_GET] &&op_get,
	[OP_SET] &&op_set,
	[OP_SUB1] &&op_sub1,
	[OP_RET] &&op_ret,
	[OP_PUSH_RA] &&op_push_ra,
	[OP_JGT] &&op_jgt,
	[OP_PUSH] &&op_push,
	[OP_POP] &&op_pop,
	[OP_DEBUG] &&op_debug,
	[OP_HALT] &&op_halt,
    };

    sp = &stack[1024];
    DPRINTF("sp == %p, &stack[1024] == %p, ra == %p\n", sp, &stack[1024], __builtin_return_address(0));

    /* XXX: run() must not be a leaf function, otherwise the
       call-threaded code's calls into it will clobber its LR */

    for(;;) {
	goto *pc0;
	BEGIN_ENTRY1(op_get,unsigned int,n,NOSAVE_RA);
	{
	    int x;
	    GET_INT(n, x);
	    PUSH_INT(x);
	    NEXT0;
	}
	END_ENTRY;
	/* Without the continue statements, gcc-3.4.1 on PowerPC uses
	   different registers for 'sp' on entry and exit from op_get,
	   which breaks the call-threaded code. The continue statements
	   de-linearise the control flow graph, which enforces consistent
	   register usage in the opcode blocks. */
	continue;
	BEGIN_ENTRY1(op_set,unsigned int,n,NOSAVE_RA);
	{
	    int x;
	    POP_INT(x);
	    SET_INT(n, x);
	    NEXT0;
	}
	END_ENTRY;
	continue;
	BEGIN_ENTRY0(op_sub1,NOSAVE_RA);
	{
	    int x;
	    GET_INT(0, x);
	    --x;
	    SET_INT(0, x);
	    NEXT0;
	}
	END_ENTRY;
	continue;
	BEGIN_ENTRY1(op_ret,unsigned int,n,NOSAVE_RA);
	{
	    int rv;
	    code_ptr_t npc;
	    POP_INT(rv);
	    POP_RA(npc);
	    DROP(n);
	    PUSH_INT(rv);
	    NEXT1(npc);
	}
	END_ENTRY;
	continue;
	BEGIN_ENTRY1(op_push_ra,code_ptr_t,ra,NOSAVE_RA);
	{
	    PUSH_RA(ra);
	    NEXT0;
	}
	END_ENTRY;
	continue;
	BEGIN_ENTRY0(op_jgt,NOSAVE_RA);
	{
	    int x, y;
	    POP_INT(x);
	    POP_INT(y);
	    NEXT1((x - y));
	}
	END_ENTRY;
	continue;
	BEGIN_ENTRY1(op_push,int,x,NOSAVE_RA);
	{
	    PUSH_INT(x);
	    NEXT0;
	}
	END_ENTRY;
	continue;
	BEGIN_ENTRY0(op_pop,NOSAVE_RA);
	{
	    DROP(1);
	    NEXT0;
	}
	END_ENTRY;
	continue;
	BEGIN_ENTRY0(op_debug,SAVE_RA);
	{
	    printf("%d %d %d @ sp %p\n", sp[1].val, sp[2].val, sp[3].val, sp);
	    RESTORE_NEXT0;
	}
	END_ENTRY;
	continue;
	BEGIN_ENTRY0(op_halt,NOSAVE_RA);
	{
	    DPRINTF("sp == %p, &stack[1024] == %p, ra == %p\n", sp, &stack[1024], __builtin_return_address(0));
	    return;
	}
	END_ENTRY;
    }
}

extern __opcode void op_get(unsigned int);
extern __opcode void op_set(unsigned int);
extern __opcode void op_sub1(void);
extern __opcode code_ptr_t op_ret(unsigned int);
extern __opcode void op_push_ra(code_ptr_t);
extern __opcode int op_jgt(void);
extern __opcode void op_push(int);
extern __opcode void op_pop(void);
extern __opcode void op_debug(void);
extern __opcode void op_halt(void);

#include "ctc_c_shared_jmpopt.h"
#include "ctc_x86_shared_argopt.h"
#include "ctc_amd64_shared_jmpopt.h"
#include "ctc_ppc_shared_jmpopt.h"
#include "ctc_sparc_shared_jmpopt.h"
#include "ctc_arm_shared_jmpopt.h"

#if DEBUG
#define Ltak3	lab0
#define NITER	1
#else
#define Ltak3	lab1
#define NITER	255
#endif

#include "tak_ctc.h" /* tak's call-threaded code skeleton */

static void doit(void)
{
    extern const void * code0 asm("code0");
    run(&code0);
}

#if STANDALONE
static void foo(void *ra)
{
    DPRINTF("main()'s ra == %p\n", ra);
}

int main(void)
{
    foo(__builtin_return_address(0));
    doit();
    foo(__builtin_return_address(0));
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
