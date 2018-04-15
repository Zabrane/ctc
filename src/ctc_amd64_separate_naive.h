/* CTC definitions for AMD64:
 * - separate interpreter functions
 * - naive treatment of control flow
 */
#if defined(__x86_64__) && !ARCHC
#define BEGIN_CODE()	asm(
#define END_CODE()	: : : "%rax", "%rdi", "%rsi");
#define STRING(x)	#x
#define LABEL(l)	"Lcode_" STRING(l)
#define OP_LABEL(l)	"\n" LABEL(l) ":\n\t"
#define OP_GLOBAL(l)	/*empty*/
#define OP_ENTRY()	/*empty*/ 
#define OP_GET(n)	"movl\t$" STRING(n) ", %%edi\n\tcall\top_get\n\t"
#define OP_SET(n)	"movl\t$" STRING(n) ", %%edi\n\tcall\top_set\n\t"
#define OP_SUB1()	"call\top_sub1\n\t"
#define OP_RET(n)	"movl\t$" STRING(n) ", %%edi\n\tcall\top_ret\n\tjmp\t*%%rax\n\t"
#define OP_CALL(l1,l2)	"movq\t$" LABEL(l2) ", %%rsi\n\tmovq\t$" LABEL(l1) ", %%rdi\n\tcall\top_call\n\tjmp\t*%%rax\n\t"
#define OP_JMP(lab)	"movq\t$" LABEL(lab) ", %%rdi\n\tcall\top_jmp\n\tjmp\t*%%rax\n\t"
#define OP_JGT(lab)	"movq\t$" LABEL(lab) ", %%rdi\n\tcall\top_jgt\n\ttestq\t%%rax, %%rax\n\tjz\t1f\n\tjmp\t*%%rax\n1:\n\t"
#define OP_PUSH(n)	"movl\t$" STRING(n) ", %%edi\n\tcall\top_push\n\t"
#define OP_POP()	"call\top_pop\n\t"
#define OP_DEBUG()	"call\top_debug\n\t"
#define OP_HALT()	/* "call\top_halt\n\t" */
#endif
