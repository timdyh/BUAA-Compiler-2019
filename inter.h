#ifndef inter_h
#define inter_h

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

enum interOpType {
    INTER_OP_LABEL,                                                                     // :
    INTER_OP_ASSIGN,                                                                    // =
    INTER_OP_ADD, INTER_OP_SUB, INTER_OP_MUL, INTER_OP_DIV,                             // +, -, *, /
    INTER_OP_ARRGET, INTER_OP_ARRPUT,                                                   // [], []=
    INTER_OP_PUSH, INTER_OP_CALL, INTER_OP_RET,                                         // push, call, ret
    INTER_OP_READI, INTER_OP_READC,                                                     // readi, readc
    INTER_OP_PRINTI, INTER_OP_PRINTC, INTER_OP_PRINTS,                                  // printi, printc, prints
    INTER_OP_JUMP,                                                                      // jump
    INTER_OP_BEQ, INTER_OP_BNE, INTER_OP_BLT, INTER_OP_BLE, INTER_OP_BGT, INTER_OP_BGE  // beq, bne, blt, ble, bgt, bge
};

struct InterCode {
    int opNo;
    string opd1, opd2, opd3;
};

extern ofstream finter;
extern vector<InterCode> interCodeList;

void genInterCode(int opNo, string opd1 = "", string opd2 = "", string opd3 = "");

#endif /* inter_h */
