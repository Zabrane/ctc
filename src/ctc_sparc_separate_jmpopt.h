/* CTC definitions for SPARC:
 * - separate interpreter functions
 * - improved treatment of control flow
 */
#if defined(__sparc__) && !ARCHC
#define BEGIN_CODE()	asm(
#define END_CODE()	: : : "%o0", "%o1");
#define STRING(x)	#x
#define LABEL(l)	"Lcode_" STRING(l)
#define OP_LABEL(l)	"\n" LABEL(l) ":\n\t"
#define OP_GLOBAL(l)	/*empty*/
#define OP_ENTRY()	/*empty*/ 
#define OP_GET(n)	"call\top_get\n\tmov\t" STRING(n) ", %%o0\n\t"
#define OP_SET(n)	"call\top_set\n\tmov\t" STRING(n) ", %%o0\n\t"
#define OP_SUB1()	"call\top_sub1\n\tnop\n\t"
#define OP_RET(n)	"call\top_ret\n\tmov\t" STRING(n) ", %%o0\n\tjmp\t%%o0\n\tnop\n\t"
/* ba;nop is much faster than ba,a */
#define OP_CALL(l1,l2)	"sethi\t%%hi(" LABEL(l2) "), %%o0\n\tcall\top_push_ra\n\tor\t%%o0, %%lo(" LABEL(l2) "), %%o0\n\tba\t" LABEL(l1) "\n\tnop\n\t"
#define OP_JMP(lab)	"ba\t" LABEL(lab) "\n\tnop\n\t"
#define OP_JGT(lab)	"call\top_jgt\n\tnop\n\tcmp\t%%o0, 0\n\tbg\t" LABEL(lab) "\n\tnop\n\t"
#define OP_PUSH(n)	"call\top_push\n\tmov\t" STRING(n) ", %%o0\n\t"
#define OP_POP()	"call\top_pop\n\tnop\n\t"
#define OP_DEBUG()	"call\top_debug\n\tnop\n\t"
#define OP_HALT()	/* "call\top_halt\n\tnop\n\t" */
#endif
