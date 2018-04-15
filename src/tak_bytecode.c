/* tak_bytecode.c
 */
#include "tak_bytecode.h"

#define Lloop		2
#define Ltak0		14
#define Ltak3_debug	24
#define Ltak3_nodebug	25
#define Ltak3_body	35

#if DEBUG
#define Ltak3	Ltak3_debug
#define NITER	1
#else
#define Ltak3	Ltak3_nodebug
#define NITER	255
#endif

const unsigned char tak_bc[] = {
    /*  0 *//* main: */			/* */
    /*  0 */	OP_PUSH, NITER,		/* i */
    /*  2 *//* Lloop: */
    /*  2 */	OP_CALL, Ltak0-3,	/* i rv */
    /*  4 */	OP_POP,			/* i */
    /*  5 */	OP_SUB1,		/* i' */
    /*  6 */	OP_PUSH, 0,		/* i' 0 */
    /*  8 */	OP_GET, 1,		/* i' 0 i' */
    /* 10 */	OP_JGT, Lloop-11,	/* i' */
    /* 12 */	OP_POP,			/* */
    /* 13 */	OP_HALT,		/* */
    /* 14 *//* Ltak0: */		/* */
    /* 14 */	OP_PUSH, 6,		/* ra 6 */
    /* 16 */	OP_PUSH, 12,		/* ra 6 12 */
    /* 18 */	OP_PUSH, 18,		/* ra 6 12 18 */
    /* 20 */	OP_CALL, Ltak3-21,	/* ra rv */
    /* 22 */	OP_RET, 0,		/* */
    /* 24 *//* Ltak3_debug: */		/* z y x ra */
    /* 24 */	OP_DEBUG,		/* z y x ra */
    /* 25 *//* Ltak3_nodebug: */
    /* 25 */	OP_GET, 2,		/* z y x ra y */
    /* 27 */	OP_GET, 2,		/* z y x ra y x */
    /* 29 */	OP_JGT, Ltak3_body-30,	/* z y x ra */
    /* 31 */	OP_GET, 3,		/* z y x ra z */
    /* 33 */	OP_RET, 3,		/* z */
    /* 35 *//* Ltak3_body: */
    /* 35 */	OP_GET, 3,		/* z y x ra z */
    /* 37 */	OP_GET, 3,		/* z y x ra z y */
    /* 39 */	OP_GET, 3,		/* z y x ra z y x */
    /* 41 */	OP_SUB1,		/* z y x ra z y x' */
    /* 42 */	OP_CALL, Ltak3-43,	/* z y x ra a */
    /* 44 */	OP_GET, 2,		/* z y x ra a x */
    /* 46 */	OP_GET, 5,		/* z y x ra a x z */
    /* 48 */	OP_GET, 5,		/* z y x ra a x z y */
    /* 50 */	OP_SUB1,		/* z y x ra a x z y' */
    /* 51 */	OP_CALL, Ltak3-52,	/* z y x ra a b */
    /* 53 */	OP_GET, 4,		/* z y x ra a b y */
    /* 55 */	OP_GET, 4,		/* z y x ra a b y x */
    /* 57 */	OP_GET, 7,		/* z y x ra a b y x z */
    /* 59 */	OP_SUB1,		/* z y x ra a b y x z' */
    /* 60 */	OP_CALL, Ltak3-61,	/* z y x ra a b c */
    /* 62 */	OP_SET, 5,		/* c y x ra a b */
    /* 64 */	OP_SET, 3,		/* c b x ra a */
    /* 66 */	OP_SET, 1,		/* c b a ra */
    /* 68 */	OP_JMP, Ltak3-69,	/* c b a ra */
};

const unsigned int tak_bc_size = sizeof(tak_bc);
