/* CTC definitions for PowerPC:
 * - fake interpreter functions sharing context in a single C function
 * - naive treatment of control flow
 */
#if defined(__powerpc__) && !ARCHC
#define BEGIN_CODE()	asm(
#define END_CODE()	: : : "lr", "r0", "r3", "r4");
#define STRING(x)	#x
#define LABEL(l)	"Lcode_" STRING(l)
#define OP_LABEL(l)	"\n" LABEL(l) ":\n\t"
#define OP_GLOBAL(l)	"\n" l ":\n\t"
#define OP_ENTRY()	/*empty*/
#define OP_GET(n)	"li 3," STRING(n) "\n\tbl op_get\n\t"
#define OP_SET(n)	"li 3," STRING(n) "\n\tbl op_set\n\t"
#define OP_SUB1()	"bl op_sub1\n\t"
#define OP_RET(n)	"li 3," STRING(n) "\n\tbl op_ret\n\tmtctr 3\n\tbctr\n\t"
#define OP_CALL(l1,l2)	"lis 3," LABEL(l1) "@ha\n\tlis 4," LABEL(l2) "@ha\n\tla 3," LABEL(l1) "@l(3)\n\tla 4," LABEL(l2) "@l(4)\n\tbl op_call\n\tmtctr 3\n\tbctr\n\t"
#define OP_JMP(lab)	"lis 3," LABEL(lab) "@ha\n\tla 3," LABEL(lab) "@l(3)\n\tbl op_jmp\n\tmtctr 3\n\tbctr\n\t"
#if 1 /* "beq 1f; bctr; 1:" is faster than "bnectr" */
#define OP_JGT(lab)	"lis 3," LABEL(lab) "@ha\n\tla 3," LABEL(lab) "@l(3)\n\tbl op_jgt\n\tmr. 0,3\n\tmtctr 0\n\tbeq 1f\n\tbctr\n1:\n\t"
#else
#define OP_JGT(lab)	"lis 3," LABEL(lab) "@ha\n\tla 3," LABEL(lab) "@l(3)\n\tbl op_jgt\n\tmr. 0,3\n\tmtctr 0\n\tbnectr\n\t"
#endif
#define OP_PUSH(n)	"li 3," STRING(n) "\n\tbl op_push\n\t"
#define OP_POP()	"bl op_pop\n\t"
#define OP_DEBUG()	"bl op_debug\n\t"
#define OP_HALT()	"bl op_halt\n\t"
#endif
