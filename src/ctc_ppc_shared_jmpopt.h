/* CTC definitions for PowerPC:
 * - fake interpreter functions sharing context in a single C function
 * - improved treatment of control flow
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
#define OP_CALL(l1,l2)	"lis 3," LABEL(l2) "@ha\n\tla 3," LABEL(l2) "@l(3)\n\tbl op_push_ra\n\tb " LABEL(l1) "\n\t"
#define OP_JMP(lab)	"b " LABEL(lab) "\n\t"
#define OP_JGT(lab)	"bl op_jgt\n\tcmpwi 7,3,0\n\tbgt 7," LABEL(lab) "\n\t"
#define OP_PUSH(n)	"li 3," STRING(n) "\n\tbl op_push\n\t"
#define OP_POP()	"bl op_pop\n\t"
#define OP_DEBUG()	"bl op_debug\n\t"
#define OP_HALT()	"bl op_halt\n\t"
#endif
