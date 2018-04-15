/* CTC definitions for simulation in C:
 * - fake interpreter functions sharing context in a single C function
 * - naive treatment of control flow
 */
/* gcc-3.4.1 on PowerPC and SPARC caches code()'s &&label computations
 * in callee-save registers, which conflicts with run()'s use of the
 * same callee-save registers for VM state. There are two workarounds:
 * 1. Disable the C version of code() on PowerPC and SPARC.
 * 2. Use global registers for run()'s VM state.
 * We use the first workaround.
 */
#if ARCHC && !defined(__powerpc__) && !defined(__sparc__)
#define BEGIN_CODE()	/*empty*/
#define END_CODE()	OP_HALT() /* prevent last explicit OP_ from becoming a tailcall */
#define OP_LABEL(l)	l:
#define OP_GLOBAL(l)	asm("\n" l ":\n\t");
#if defined(__i386__)
#define OP_ENTRY()	asm("addl\t$4, %esp"); /* to offset the 'call's done here */
#else
#define OP_ENTRY()	/* no stack pointer adjustment needed */
#endif 
#define OP_GET(n)	op_get(n);
#define OP_SET(n)	op_set(n);
#define OP_SUB1()	op_sub1();
#define OP_RET(n)	goto *op_ret(n);
#define OP_CALL(l1,l2)	goto *op_call(&&l1,&&l2);
#define OP_JMP(lab)	goto *op_jmp(&&lab);
#define OP_JGT(lab)	do{const void **npc = op_jgt(&&lab);if(npc)goto*npc;}while(0);
#define OP_PUSH(n)	op_push(n);
#define OP_POP()	op_pop();
#define OP_DEBUG()	op_debug();
#define OP_HALT()	op_halt();
#endif
