/* CTC definitions for ARM:
 * - separate interpreter functions
 * - improved treatment of control flow
 */
#if defined(__arm__) && !ARCHC
#define BEGIN_CODE()	asm(
#define END_CODE()	: : : "%r0", "%r1", "%r12", "%r14");
#define STRING(x)	#x
#define LABEL(l)	"Lcode_" STRING(l)
#define OP_LABEL(l)	"\n" LABEL(l) ":\n\t"
#define OP_GLOBAL(l)	/*empty*/
#define OP_ENTRY()	/*empty*/ 
#define OP_GET(n)	"mov\tr0, #" STRING(n) "\n\tbl\top_get\n\t"
#define OP_SET(n)	"mov\tr0, #" STRING(n) "\n\tbl\top_set\n\t"
#define OP_SUB1()	"bl\top_sub1\n\t"
#define OP_RET(n)	"mov\tr0, #" STRING(n) "\n\tbl\top_ret\n\tmov\tpc, r0\n\t"
#define OP_CALL(l1,l2)	"add\tr0, pc, #(" LABEL(l2) "-.)-8\n\tbl\top_push_ra\n\tb\t" LABEL(l1) "\n\t"
#define OP_JMP(lab)	"b\t" LABEL(lab) "\n\t"
#define OP_JGT(lab)	"bl\top_jgt\n\tcmp\tr0, #0\n\tbgt\t" LABEL(lab) "\n\t"
#define OP_PUSH(n)	"mov\tr0, #" STRING(n) "\n\tbl\top_push\n\t"
#define OP_POP()	"bl\top_pop\n\t"
#define OP_DEBUG()	"bl\top_debug\n\t"
#define OP_HALT()	/* "bl\top_halt\n\t" */
#endif
