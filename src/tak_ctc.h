/* tak_ctc.h
 * Skeletal call-threaded code for TAK, to be
 * included by the particular CTC implementation.
 */
static void __attribute__((used)) code(void)
{
BEGIN_CODE()
OP_GLOBAL("code0")
    OP_ENTRY()
    OP_JMP(lab56)
OP_LABEL(lab0)
    /* tak3: */		/* z y x ra */
    OP_DEBUG()		/* z y x ra */
OP_LABEL(lab1)
    OP_GET(2)		/* z y x ra y */
    OP_GET(2)		/* z y x ra y x */
    OP_JGT(lab11)	/* z y x ra */
    OP_GET(3)		/* z y x ra z */
    OP_RET(3)		/* z */
OP_LABEL(lab11)
    OP_GET(3)		/* z y x ra z */
    OP_GET(3)		/* z y x ra z y */
    OP_GET(3)		/* z y x ra z y x */
    OP_SUB1()		/* z y x ra z y x' */
    OP_CALL(Ltak3,lab20)/* z y x ra a */
OP_LABEL(lab20)
    OP_GET(2)		/* z y x ra a x */
    OP_GET(5)		/* z y x ra a x z */
    OP_GET(5)		/* z y x ra a x z y */
    OP_SUB1()		/* z y x ra a x z y' */
    OP_CALL(Ltak3,lab29)/* z y x ra a b */
OP_LABEL(lab29)
    OP_GET(4)		/* z y x ra a b y */
    OP_GET(4)		/* z y x ra a b y x */
    OP_GET(7)		/* z y x ra a b y x z */
    OP_SUB1()		/* z y x ra a b y x z' */
    OP_CALL(Ltak3,lab38)/* z y x ra a b c */
OP_LABEL(lab38)
    OP_SET(5)		/* c y x ra a b */
    OP_SET(3)		/* c b x ra a */
    OP_SET(1)		/* c b a ra */
    OP_JMP(Ltak3)	/* c b a ra */
OP_LABEL(lab46)
    /* tak0: */		/* */
    OP_PUSH(6)		/* ra 6 */
    OP_PUSH(12)		/* ra 6 12 */
    OP_PUSH(18)		/* ra 6 12 18 */
    OP_CALL(Ltak3,lab54)/* ra rv */
OP_LABEL(lab54)
    OP_RET(0)		/* */
OP_LABEL(lab56)
    /* loop: */		/* */
    OP_PUSH(NITER)	/* i */
OP_LABEL(lab58)
    OP_CALL(lab46,lab60)/* i rv */
OP_LABEL(lab60)
    OP_POP()		/* i */
    OP_SUB1()		/* i' */
    OP_PUSH(0)		/* i' 0 */
    OP_GET(1)		/* i' 0 i' */
    OP_JGT(lab58)	/* i' */
    OP_POP()		/* */
    OP_HALT()		/* */
END_CODE()
}
