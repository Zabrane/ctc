/* tak_ctc2.c
 * tak_ctc1 + instruction operands in registers on x86
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

#if defined(__i386__)
register stack_ptr_t sp __asm__("%ebp");
#define __opcode	__attribute__((regparm(1)))
#elif defined(__x86_64__)
register stack_ptr_t sp __asm__("%r15");
#define __opcode	/*empty*/
#elif defined(__powerpc__)
register stack_ptr_t sp __asm__("%r31");
#define __opcode	/*empty*/
#elif defined(__sparc__)
register stack_ptr_t sp __asm__("%g5");
#define __opcode	/*empty*/
#elif defined(__arm__)
register stack_ptr_t sp __asm__("%r11");
#define __opcode	/*empty*/
#endif

#include <stdio.h>	/* must come after the global register decl above */

#if DEBUG
#define DPRINTF(fmt, args...)	printf(fmt, ##args)
#else
#define DPRINTF(fmt, args...)
#endif

__opcode void op_get(unsigned int n)
{
    int x;
    GET_INT(n, x);
    PUSH_INT(x);
}

__opcode void op_set(unsigned int n)
{
    int x;
    POP_INT(x);
    SET_INT(n, x);
}

__opcode void op_sub1(void)
{
    int x;
    GET_INT(0, x);
    --x;
    SET_INT(0, x);
}

__opcode code_ptr_t op_ret(unsigned int n)
{
    int rv;
    code_ptr_t npc;
    POP_INT(rv);
    POP_RA(npc);
    DROP(n);
    PUSH_INT(rv);
    return npc;
}

__opcode void op_push_ra(code_ptr_t ra)
{
    PUSH_RA(ra);
}

__opcode int op_jgt(void)
{
    int x, y;
    POP_INT(x);
    POP_INT(y);
    return x - y;
}

__opcode void op_push(int x)
{
    PUSH_INT(x);
}

__opcode void op_pop(void)
{
    DROP(1);
}

__opcode void op_debug(void)
{
    printf("%d %d %d\n", sp[1].val, sp[2].val, sp[3].val);
}

#include "ctc_c_separate_jmpopt.h"
#include "ctc_x86_separate_argopt.h"
#include "ctc_amd64_separate_jmpopt.h"
#include "ctc_ppc_separate_jmpopt.h"
#include "ctc_sparc_separate_jmpopt.h"
#include "ctc_arm_separate_jmpopt.h"

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
    /* Unless -mpreferred-stack-boundary=2 is used, gcc-3.x
     * will define and use %ebp in main() in order to align
     * the stack, even when -fomit-frame-pointer -ffixed-ebp
     * are used.
     * Explicitly save and restore %ebp here to avoid breakage.
     */
    union stack_slot stack[1024];
    stack_ptr_t old_sp = sp;
    DPRINTF("&stack[1024] == %p\n", &stack[1024]);
    sp = &stack[1024];
    code();
    DPRINTF("sp == %p\n", sp);
    sp = old_sp; /* skipping this crashes main()'s return */
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
