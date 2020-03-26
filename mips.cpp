#include "mips.h"
#include "inter.h"
#include "syntax.h"

ofstream fmips;

const string op[] = {
    ".data", ".text",
    ".space", ".asciiz",
    ":",
    "syscall",
    "addu", "subu", "addiu", "subiu",
    "mult", "mul", "div",
    "sll", "srl",
    "move", "lui",
    "li", "la",
    "lw", "sw",
    "mfhi", "mflo",
    "j", "jr", "jal",
    "beq", "bne", "blt", "ble", "bgt", "bge"
};

const string reg[] = {
    "$zero",
    "$v0", "$v1",
    "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$gp", "$sp", "$fp",
    "$ra"
};

map<int, int> inter2mips = {
    {INTER_OP_BEQ, MIPS_OP_BEQ},
    {INTER_OP_BNE, MIPS_OP_BNE},
    {INTER_OP_BLT, MIPS_OP_BLT},
    {INTER_OP_BLE, MIPS_OP_BLE},
    {INTER_OP_BGT, MIPS_OP_BGT},
    {INTER_OP_BGE, MIPS_OP_BGE}
};

bool isNumber(string name)
{
    return (isdigit(name[0]) || name[0] == '-' || name[0] == '+');
}

string replaceConst(string name)
{
    if (inLocalSymTable(name))
    {
        if (localSymTable[name].kind == CONST)
            return to_string(localSymTable[name].value);
        return name;
    }
    if (inGlobalSymTable(name) && globalSymTable[name].kind == CONST)
        return to_string(globalSymTable[name].value);
    return name;
}

int getOffset(string name)
{
    if (inLocalSymTable(name)) return localSymTable[name].offset;
    if (inGlobalSymTable(name)) return globalSymTable[name].offset;
    return -1;      // -1表示是立即数
}

void genMipsCode(int opNo, string opd1 = "", string opd2 = "", string opd3 = "")
{
    switch (opNo) {
        case MIPS_OP_DATA:
        case MIPS_OP_TEXT:
            fmips << op[opNo] << endl;
            break;
        case MIPS_OP_SPACE:
            fmips << opd1 << ": " << op[opNo] << " " << opd2 << endl;
            break;
        case MIPS_OP_ASCIIZ:
            fmips << opd1 << ": " << op[opNo] << " \"" << opd2 << "\"" << endl;
            break;
        case MIPS_OP_LABEL:
            fmips << opd1 << op[opNo] << endl;
            break;
        case MIPS_OP_SYSCALL:
            fmips << op[opNo] << endl;
            break;
        case MIPS_OP_MULT:
        case MIPS_OP_DIV:
        case MIPS_OP_MOVE:
        case MIPS_OP_LUI:
        case MIPS_OP_LI:
        case MIPS_OP_LA:
            fmips << op[opNo] << " " << opd1 << ", " << opd2 << endl;
            break;
        case MIPS_OP_LW:
        case MIPS_OP_SW:
            if (opd3.empty())
                fmips << op[opNo] << " " << opd1 << ", " << opd2 << endl;
            else if (isNumber(opd2) || !isNumber(opd3))
                fmips << op[opNo] << " " << opd1 << ", " << opd2 << "(" << opd3 << ")" << endl;
            else
                fmips << op[opNo] << " " << opd1 << ", " << opd2 << "+" << opd3 << endl;
            break;
        case MIPS_OP_MFHI:
        case MIPS_OP_MFLO:
        case MIPS_OP_J:
        case MIPS_OP_JR:
        case MIPS_OP_JAL:
            fmips << op[opNo] << " " << opd1 << endl;
            break;
        default:
            fmips << op[opNo] << " " << opd1 << ", " << opd2 << ", " << opd3 << endl;
            break;
    }
}

void genMipsCode_load(string name, string dstReg)
{
    int offset = getOffset(name);
    if (offset == -1) genMipsCode(MIPS_OP_LI, dstReg, name);
    else genMipsCode(MIPS_OP_LW, dstReg, to_string(offset), (offset < 0) ? reg[SP] : reg[GP]);
}

void genMipsCode_save(string name, string srcReg)
{
    int offset = getOffset(name);
    genMipsCode(MIPS_OP_SW, srcReg, to_string(offset), (offset < 0) ? reg[SP] : reg[GP]);
}

void genMipsCode_arr(int op, string arrName, string index, string srcdstReg)
{
    int offset = getOffset(arrName);        // 数组本身的偏移地址
    if (isNumber(index))
    {
        int arrOffset = stoi(index) * 4;    // 某个元素相对数组首地址的偏移
        if (offset < 0) arrOffset = -arrOffset;
        genMipsCode(MIPS_OP_ADDIU, reg[A0], (offset < 0) ? reg[SP] : reg[GP], to_string(offset));   // 数组首地址存入$a0
        genMipsCode(op, srcdstReg, to_string(arrOffset), reg[A0]);                                  // 加载（存储）时加上元素的偏移
    }
    else
    {
        if (offset < 0)
        {
            genMipsCode_load(index, reg[A0]);                           // 将下标加载至$a0
            genMipsCode(MIPS_OP_SLL, reg[A0], reg[A0], "2");            // 下标乘4得到元素的偏移
            genMipsCode(MIPS_OP_SUBU, reg[A0], reg[SP], reg[A0]);       // 基地址加上元素的偏移
            genMipsCode(op, srcdstReg, to_string(offset), reg[A0]);     // 加载（存储）时加上数组本身的偏移
        }
        else
        {
            genMipsCode_load(index, reg[A0]);                           // 将下标加载至$a0
            genMipsCode(MIPS_OP_SLL, reg[A0], reg[A0], "2");            // 下标乘4得到元素的偏移
            genMipsCode(op, srcdstReg, arrName, reg[A0]);               // label($a0)形式的加载（存储）
        }
    }
}

void translate()
{
    /* .data */
    genMipsCode(MIPS_OP_DATA);
    int globalOffset = 0;
    for (map<string, Entry>::iterator it = globalSymTable.begin(); it != globalSymTable.end(); it++)
    {
        string name = it->first;
        int kind = (it->second).kind;
        if (!(kind == VAR || kind == ARRAY)) continue;
        int space = (kind == VAR) ? 4 : (it->second).value * 4;
        genMipsCode(MIPS_OP_SPACE, name, to_string(space));
        (it->second).offset = globalOffset;
        if (kind == VAR) globalOffset += 4;
        else globalOffset += (it->second).value * 4;
    }
    for (map<string, string>::iterator it = strTable.begin(); it != strTable.end(); it++)
    {
        string name = it->first;
        string str = it->second;
        genMipsCode(MIPS_OP_ASCIIZ, name, str);
    }
    
    /* .text */
    fmips << endl;
    genMipsCode(MIPS_OP_TEXT);
    genMipsCode(MIPS_OP_LUI, reg[GP], "0x1001");
    genMipsCode(MIPS_OP_JAL, "main");
    genMipsCode(MIPS_OP_LI, reg[V0], "10");
    genMipsCode(MIPS_OP_SYSCALL);
    
    string funcName;
    int offset = -4;
    for (int i = 0; i < interCodeList.size(); i++)
    {
        InterCode code = interCodeList[i];
        if (i < interCodeList.size() - 1)
        {
            InterCode next = interCodeList[i+1];
            if (code.opd1[0] == '$' && next.opNo == INTER_OP_ASSIGN && code.opd1 == next.opd2)
            {
                code.opd1 = next.opd1;
                i++;
            }
        }
        
        int opNo = code.opNo;
        string opd1 = replaceConst(code.opd1);
        string opd2 = replaceConst(code.opd2);
        string opd3 = replaceConst(code.opd3);
        
        switch (opNo) {
            case INTER_OP_LABEL:
                if (opd1[0] != '$') fmips << endl;
                genMipsCode(MIPS_OP_LABEL, opd1);
                if (opd1[0] != '$')
                {
                    funcName = opd1;
                    localSymTable = totLocalSymTable[funcName];
                    genMipsCode(MIPS_OP_SW, reg[RA], "0", reg[SP]);
                    genMipsCode(MIPS_OP_ADDIU, reg[FP], reg[SP], to_string(getOffset(funcName)));
                }
                break;
            case INTER_OP_ASSIGN:
                if (opd2 == "$RET") genMipsCode_save(opd1, reg[V0]);
                else
                {
                    genMipsCode_load(opd2, reg[T8]);
                    genMipsCode_save(opd1, reg[T8]);
                }
                break;
            case INTER_OP_ADD:
                if (isNumber(opd2) && isNumber(opd3))
                {
                    genMipsCode(MIPS_OP_LI, reg[T8], to_string(stoi(opd2) + stoi(opd3)));
                }
                else if (!isNumber(opd2) && isNumber(opd3))
                {
                    genMipsCode_load(opd2, reg[T8]);
                    genMipsCode(MIPS_OP_ADDIU, reg[T8], reg[T8], opd3);
                }
                else if (isNumber(opd2) && !isNumber(opd3))
                {
                    genMipsCode_load(opd3, reg[T8]);
                    genMipsCode(MIPS_OP_ADDIU, reg[T8], reg[T8], opd2);
                }
                else
                {
                    genMipsCode_load(opd2, reg[T8]);
                    genMipsCode_load(opd3, reg[T9]);
                    genMipsCode(MIPS_OP_ADDU, reg[T8], reg[T8], reg[T9]);
                }
                genMipsCode_save(opd1, reg[T8]);
                break;
            case INTER_OP_SUB:
                if (isNumber(opd2) && isNumber(opd3))
                {
                    genMipsCode(MIPS_OP_LI, reg[T8], to_string(stoi(opd2) - stoi(opd3)));
                }
                else if (!isNumber(opd2) && isNumber(opd3))
                {
                    genMipsCode_load(opd2, reg[T8]);
                    if (opd3[0] == '+') opd3[0] = '-';
                    else if (opd3[0] == '-') opd3 = opd3.substr(1);
                    else opd3 = "-" + opd3;
                    genMipsCode(MIPS_OP_ADDIU, reg[T8], reg[T8], opd3);
                }
                else
                {
                    genMipsCode_load(opd2, reg[T8]);
                    genMipsCode_load(opd3, reg[T9]);
                    genMipsCode(MIPS_OP_SUBU, reg[T8], reg[T8], reg[T9]);
                }
                genMipsCode_save(opd1, reg[T8]);
                break;
            case INTER_OP_MUL:
                if (isNumber(opd2) && isNumber(opd3))
                {
                    genMipsCode(MIPS_OP_LI, reg[T8], to_string(stoi(opd2) * stoi(opd3)));
                }
                else
                {
                    genMipsCode_load(opd2, reg[T8]);
                    genMipsCode_load(opd3, reg[T9]);
                    genMipsCode(MIPS_OP_MUL, reg[T8], reg[T8], reg[T9]);
                }
                genMipsCode_save(opd1, reg[T8]);
                break;
            case INTER_OP_DIV:
                if (isNumber(opd2) && isNumber(opd3))
                {
                    genMipsCode(MIPS_OP_LI, reg[T8], to_string(stoi(opd2) / stoi(opd3)));
                }
                else
                {
                    genMipsCode_load(opd2, reg[T8]);
                    genMipsCode_load(opd3, reg[T9]);
                    genMipsCode(MIPS_OP_DIV, reg[T8], reg[T9]);
                    genMipsCode(MIPS_OP_MFLO, reg[T8]);
                }
                genMipsCode_save(opd1, reg[T8]);
                break;
            case INTER_OP_ARRGET:
                genMipsCode_arr(MIPS_OP_LW, opd2, opd3, reg[T8]);
                genMipsCode_save(opd1, reg[T8]);
                break;
            case INTER_OP_ARRPUT:
                genMipsCode_load(opd3, reg[T8]);
                genMipsCode_arr(MIPS_OP_SW, opd1, opd2, reg[T8]);
                break;
            case INTER_OP_PUSH:
                genMipsCode_load(opd1, reg[T8]);
                genMipsCode(MIPS_OP_SW, reg[T8], to_string(offset), reg[FP]);
                offset -= 4;
                break;
            case INTER_OP_CALL:
                genMipsCode(MIPS_OP_MOVE, reg[SP], reg[FP]);
                genMipsCode(MIPS_OP_JAL, opd1);
                genMipsCode(MIPS_OP_MOVE, reg[FP], reg[SP]);
                genMipsCode(MIPS_OP_ADDIU, reg[SP], reg[SP], to_string(-getOffset(funcName)));
                offset = -4;
                break;
            case INTER_OP_RET:
                if (!opd1.empty()) genMipsCode_load(opd1, reg[V0]);
                if (globalSymTable[funcName].value == 0)
                    genMipsCode(MIPS_OP_LW, reg[RA], "0", reg[SP]);
                genMipsCode(MIPS_OP_JR, reg[RA]);
                break;
            case INTER_OP_READI:
            case INTER_OP_READC:
                genMipsCode(MIPS_OP_LI, reg[V0], (opNo == INTER_OP_READI) ? "5" : "12");
                genMipsCode(MIPS_OP_SYSCALL);
                genMipsCode_save(opd1, reg[V0]);
                break;
            case INTER_OP_PRINTI:
            case INTER_OP_PRINTC:
                genMipsCode(MIPS_OP_LI, reg[V0], (opNo == INTER_OP_PRINTI) ? "1" : "11");
                genMipsCode_load(opd1, reg[A0]);
                genMipsCode(MIPS_OP_SYSCALL);
                break;
            case INTER_OP_PRINTS:
                genMipsCode(MIPS_OP_LI, reg[V0], "4");
                genMipsCode(MIPS_OP_LA, reg[A0], opd1);
                genMipsCode(MIPS_OP_SYSCALL);
                break;
            case INTER_OP_JUMP:
                genMipsCode(MIPS_OP_J, opd1);
                break;
            default:
                genMipsCode_load(opd1, reg[T8]);
                genMipsCode_load(opd2, reg[T9]);
                genMipsCode(inter2mips[opNo], reg[T8], reg[T9], opd3);
                break;
        }
    }
}
