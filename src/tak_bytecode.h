/* tak_bytecode.h
 */

enum tak_opcode {
    OP_GET,
    OP_SET,
    OP_SUB1,
    OP_RET,
    OP_CALL,
    OP_JMP,
    OP_JGT,
    OP_PUSH,
    OP_POP,
    OP_DEBUG,
    OP_HALT,
};

extern const unsigned char tak_bc[];
extern const unsigned int tak_bc_size;
