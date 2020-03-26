#include "inter.h"

ofstream finter;
vector<InterCode> interCodeList;

const string op[] = {
    ":",
    "=",
    "+", "-", "*", "/",
    "[]", "[]=",
    "push", "call", "ret",
    "readi", "readc",
    "printi", "printc", "prints",
    "jump",
    "beq", "bne", "blt", "ble", "bgt", "bge"
};

void genInterCode(int opNo, string opd1, string opd2, string opd3)
{
    switch (opNo) {
        case INTER_OP_LABEL:
            if (opd1[0] != '$') finter << endl;
            interCodeList.push_back(InterCode {opNo, opd1});
            finter << opd1 << op[opNo] << endl;
            break;
        case INTER_OP_ASSIGN:
            interCodeList.push_back(InterCode {opNo, opd1, opd3});
            finter << opd1 << " " << op[opNo] << " " << opd3 << endl;
            break;
        case INTER_OP_ADD:
        case INTER_OP_SUB:
        case INTER_OP_MUL:
        case INTER_OP_DIV:
        case INTER_OP_ARRGET:
        case INTER_OP_ARRPUT:
            interCodeList.push_back(InterCode {opNo, opd1, opd2, opd3});
            finter << opd1 << " = " << opd2 << " " << op[opNo] << " " << opd3 << endl;
            break;
        case INTER_OP_PUSH:
        case INTER_OP_CALL:
        case INTER_OP_RET:
        case INTER_OP_READI:
        case INTER_OP_READC:
        case INTER_OP_PRINTI:
        case INTER_OP_PRINTC:
        case INTER_OP_PRINTS:
        case INTER_OP_JUMP:
            interCodeList.push_back(InterCode {opNo, opd1});
            finter << op[opNo] << " " << opd1 << endl;
            break;
        default:
            interCodeList.push_back(InterCode {opNo, opd1, opd2, opd3});
            finter << op[opNo] << " " << opd1 << " " << opd2 << " " << opd3 << endl;
            break;
    }
}
