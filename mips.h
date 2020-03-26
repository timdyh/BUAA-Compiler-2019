#ifndef mips_h
#define mips_h

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

enum mipsOpType {
    MIPS_OP_DATA, MIPS_OP_TEXT,                                                     // .data, .text
    MIPS_OP_SPACE, MIPS_OP_ASCIIZ,                                                  // .space, .asciiz
    MIPS_OP_LABEL,                                                                  // :
    MIPS_OP_SYSCALL,                                                                // syscall
    MIPS_OP_ADDU, MIPS_OP_SUBU, MIPS_OP_ADDIU, MIPS_OP_SUBIU,                       // addu, subu, addiu, subiu
    MIPS_OP_MULT, MIPS_OP_MUL, MIPS_OP_DIV,                                         // mult, mul, div
    MIPS_OP_SLL, MIPS_OP_SRL,                                                       // sll, srl
    MIPS_OP_MOVE, MIPS_OP_LUI,                                                      // move, lui
    MIPS_OP_LI, MIPS_OP_LA,                                                         // li, la
    MIPS_OP_LW, MIPS_OP_SW,                                                         // lw, sw
    MIPS_OP_MFHI, MIPS_OP_MFLO,                                                     // mfhi, mflo
    MIPS_OP_J, MIPS_OP_JR, MIPS_OP_JAL,                                             // j, jr, jal
    MIPS_OP_BEQ, MIPS_OP_BNE, MIPS_OP_BLT, MIPS_OP_BLE, MIPS_OP_BGT, MIPS_OP_BGE    // beq, bne, blt, ble, bgt, bge
};

enum regs {
    ZERO,
    V0, V1,
    A0, A1, A2, A3,
    T0, T1, T2, T3, T4, T5, T6, T7, T8, T9,
    S0, S1, S2, S3, S4, S5, S6, S7,
    GP, SP, FP,
    RA
};

extern ofstream fmips;

void translate();

#endif /* mips_h */
