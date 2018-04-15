/* CTC definitions for simulation in C:
 * - separate interpreter functions
 * - naive treatment of control flow
 */
#if ARCHC
#define BEGIN_CODE()	/*empty*/
#define END_CODE()	/*empty*/
#define OP_LABEL(l)	l:
#define OP_GLOBAL(l)	/*empty*/
#define OP_ENTRY()	/*empty*/ 
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
#define OP_HALT()	return;
#endif
