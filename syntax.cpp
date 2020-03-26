#include "syntax.h"
#include "lex.h"
#include "err.h"
#include "inter.h"

string buf;
string callFuncName;    // for calling, not definition
string name;
int retType;            // for checking return value
int kind, type, value;
int offset = -4;
int tmpNo, strNo, lblNo;
bool global = true;
bool hasRet = false;

map<string, Entry> localSymTable;
map<string, Entry> globalSymTable;
map<string, map<string, Entry>> totLocalSymTable;
map<string, vector<Parm>> parmTable;
map<string, string> strTable;
map<int, int> sym2op = {
    {LSS, INTER_OP_BLT},
    {LEQ, INTER_OP_BLE},
    {GRE, INTER_OP_BGT},
    {GEQ, INTER_OP_BGE},
    {EQL, INTER_OP_BEQ},
    {NEQ, INTER_OP_BNE}
};
map<int, int> sym2op_opp = {
    {LSS, INTER_OP_BGE},
    {LEQ, INTER_OP_BGT},
    {GRE, INTER_OP_BLE},
    {GEQ, INTER_OP_BLT},
    {EQL, INTER_OP_BNE},
    {NEQ, INTER_OP_BEQ}
};

void clearBuf()
{
    buf.clear();
}

void catBuf()
{
    buf += symTypeStr[symbol] + " " + token + "\n";
}

bool inLocalSymTable(string name)
{
    return localSymTable.find(name) != localSymTable.end();
}

bool inGlobalSymTable(string name)
{
    return globalSymTable.find(name) != globalSymTable.end();
}

bool inTotLocalSymTable(string name)
{
    return totLocalSymTable.find(name) != totLocalSymTable.end();
}

bool inParmTable(string name)
{
    return parmTable.find(name) != parmTable.end();
}

bool undefined(string name)
{
    return (!inLocalSymTable(name) && !inGlobalSymTable(name));
}

void allocOffset(int kind, int value)
{
    offset -= ((kind == VAR) ? 4 : 4 * value);
}

void resetOffset(string name)
{
    if (inGlobalSymTable(name)) globalSymTable[name].offset = offset;
    offset = -4;
}

void insertSymTable(string name, int kind, int type, int value, int offset)
{
    if (((global || kind == FUNC) && inGlobalSymTable(name)) ||
        (!global && inLocalSymTable(name)))
    {
        error(REDEFINED_NAME);
        return;
    }
    Entry entry = Entry {kind, type, value, offset};
    if (global || kind == FUNC) globalSymTable[name] = entry;
    else
    {
        localSymTable[name] = entry;
        if (kind == VAR || kind == ARRAY) allocOffset(kind, value);
    }
}

void insertParmTable(string name, vector<Parm> vec)
{
    if (inGlobalSymTable(name)) return;
    parmTable[name] = vec;
}

void clearLocalSymTable(string name)
{
    if (!inTotLocalSymTable(name)) totLocalSymTable[name] = localSymTable;
    localSymTable.clear();
}

string genTmp()
{
    string tmp = "$tmp" + to_string(tmpNo++);
    insertSymTable(tmp, VAR, INT, 0, offset);
    return tmp;
}

string genStr()
{
    return ("$str" + to_string(strNo++));
}

string genLbl()
{
    return ("$label" + to_string(lblNo++));
}

string insertStrTable(string str)
{
    string strName = genStr();
    strTable[strName] = str;
    return strName;
}

int integer(string *opd)
{
    string num = "";
    if (symbol == PLUS || symbol == MINU)
    {
        num += token;
        getSym();
    }
    if (symbol == INTCON)
    {
        num += token;
        getSym();
        fout << "<无符号整数>" << endl;
    }
    else
    {
        error();
        num = "0";
    }
    if (opd != NULL) *opd = num;
    fout << "<整数>" << endl;
    return stoi(num);
}

void defineConst()
{
    kind = CONST;
    if (symbol == INTTK)
    {
        type = INT;
        getSym();
        if (symbol == IDENFR)
        {
            name = token;
            getSym();
        }
        else error();
        if (symbol == ASSIGN) getSym();
        else error();
        if (symbol == PLUS || symbol == MINU || symbol == INTCON) value = integer();
        else error(DEFINE_CONST_ERROR);
        insertSymTable(name, kind, type, value, offset);
        while (symbol == COMMA)
        {
            getSym();
            if (symbol == IDENFR)
            {
                name = token;
                getSym();
            }
            else error();
            if (symbol == ASSIGN) getSym();
            else error();
            if (symbol == PLUS || symbol == MINU || symbol == INTCON) value = integer();
            else error(DEFINE_CONST_ERROR);
            insertSymTable(name, kind, type, value, offset);
        }
    }
    else if (symbol == CHARTK)
    {
        type = CHAR;
        getSym();
        if (symbol == IDENFR)
        {
            name = token;
            getSym();
        }
        else error();
        if (symbol == ASSIGN) getSym();
        else error();
        if (symbol == CHARCON)
        {
            value = token[0];
            getSym();
        }
        else error(DEFINE_CONST_ERROR);
        insertSymTable(name, kind, type, value, offset);
        while (symbol == COMMA)
        {
            getSym();
            if (symbol == IDENFR)
            {
                name = token;
                getSym();
            }
            else error();
            if (symbol == ASSIGN) getSym();
            else error();
            if (symbol == CHARCON)
            {
                value = token[0];
                getSym();
            }
            else error(DEFINE_CONST_ERROR);
            insertSymTable(name, kind, type, value, offset);
        }
    }
    else error();
    fout << "<常量定义>" << endl;
}

void declareConst()
{
    if (symbol == CONSTTK) getSym();
    else error();
    defineConst();
    if (symbol == SEMICN) getSym();
    else error(MISSING_SEMICN);
    while (symbol == CONSTTK)
    {
        getSym();
        defineConst();
        if (symbol == SEMICN) getSym();
        else error(MISSING_SEMICN);
    }
    fout << "<常量说明>" << endl;
}

void defineVar()
{
    kind = VAR;
    if (symbol == INTTK || symbol == CHARTK)
    {
        type = (symbol == INTTK) ? INT : CHAR;
        getSym();
    }
    else error();
    if (symbol == IDENFR)
    {
        name = token;
        getSym();
    }
    else error();
    if (symbol == LBRACK)
    {
        kind = ARRAY;
        getSym();
        if (symbol == INTCON)
        {
            value = stoi(token);
            getSym();
            fout << "<无符号整数>" << endl;
        }
        else error();
        if (symbol == RBRACK) getSym();
        else error(MISSING_RBRACK);
    }
    insertSymTable(name, kind, type, value, offset);
    while (symbol == COMMA)
    {
        kind = VAR;
        getSym();
        if (symbol == IDENFR)
        {
            name = token;
            getSym();
        }
        else error();
        if (symbol == LBRACK)
        {
            kind = ARRAY;
            getSym();
            if (symbol == INTCON)
            {
                value = stoi(token);
                getSym();
                fout << "<无符号整数>" << endl;
            }
            else error();
            if (symbol == RBRACK) getSym();
            else error(MISSING_RBRACK);
        }
        insertSymTable(name, kind, type, value, offset);
    }
    fout << "<变量定义>" << endl;
}

void defineVar_3()
{
    if (symbol == LBRACK)
    {
        kind = ARRAY;
        getSym();
        if (symbol == INTCON)
        {
            value = stoi(token);
            getSym();
            fout << "<无符号整数>" << endl;
        }
        else error();
        if (symbol == RBRACK) getSym();
        else error(MISSING_RBRACK);
    }
    insertSymTable(name, kind, type, value, offset);
    while (symbol == COMMA)
    {
        kind = VAR;
        getSym();
        if (symbol == IDENFR)
        {
            name = token;
            getSym();
        }
        else error();
        if (symbol == LBRACK)
        {
            kind = ARRAY;
            getSym();
            if (symbol == INTCON)
            {
                value = stoi(token);
                getSym();
                fout << "<无符号整数>" << endl;
            }
            else error();
            if (symbol == RBRACK) getSym();
            else error(MISSING_RBRACK);
        }
        insertSymTable(name, kind, type, value, offset);
    }
    fout << "<变量定义>" << endl;
}

void declareVar()
{
    defineVar();
    if (symbol == SEMICN) getSym();
    else error(MISSING_SEMICN);
    while (symbol == INTTK || symbol == CHARTK)
    {
        defineVar();
        if (symbol == SEMICN) getSym();
        else error(MISSING_SEMICN);
    }
    fout << "<变量说明>" << endl;
}

void declareVar_3()
{
    defineVar_3();
    if (symbol == SEMICN) getSym();
    else error(MISSING_SEMICN);
    while (symbol == INTTK || symbol == CHARTK)
    {
        type = retType = (symbol == INTTK) ? INT : CHAR;
        catBuf();
        getSym(false);
        if (symbol == IDENFR)
        {
            catBuf();
            name = token;
            getSym(false);
        }
        else error();
        if (!(symbol == LBRACK || symbol == COMMA || symbol == SEMICN)) break;
        kind = VAR;
        fout << buf;
        clearBuf();
        defineVar_3();
        if (symbol == SEMICN) getSym();
        else error(MISSING_SEMICN);
    }
    fout << "<变量说明>" << endl;
}

void declareHead()
{
    if (symbol == INTTK || symbol == CHARTK)
    {
        type = retType = (symbol == INTTK) ? INT : CHAR;
        getSym();
    }
    else error();
    if (symbol == IDENFR)
    {
        name = token;
        getSym();
    }
    else error();
    fout << "<声明头部>" << endl;
}

void paraTable()
{
    int t;
    string s;
    vector<Parm> vec;
    if (symbol == INTTK || symbol == CHARTK)
    {
        t = (symbol == INTTK) ? INT : CHAR;
        getSym();
        if (symbol == IDENFR)
        {
            s = token;
            getSym();
        }
        else error();
        vec.push_back(Parm {t});
        insertSymTable(s, VAR, t, 0, offset);
        while (symbol == COMMA)
        {
            getSym();
            if (symbol == INTTK || symbol == CHARTK)
            {
                t = (symbol == INTTK) ? INT : CHAR;
                getSym();
            }
            else error();
            if (symbol == IDENFR)
            {
                s = token;
                getSym();
            }
            else error();
            vec.push_back(Parm {t});
            insertSymTable(s, VAR, t, 0, offset);
        }
    }
    insertParmTable(name, vec);
    fout << "<参数表>" << endl;
}

void paraValTable()
{
    string opd1;
    int opNo = INTER_OP_PUSH;
    int i = 0;
    int formalType, actualType;
    vector<Parm> vec;
    if (inParmTable(callFuncName)) vec = parmTable[callFuncName];
    else while (symbol != RPARENT) getSym();
    if (symbol == PLUS || symbol == MINU || symbol == IDENFR ||
        symbol == LPARENT || symbol == INTCON || symbol == CHARCON)
    {
        if (i < vec.size()) formalType = vec[i].type;
        else formalType = -1;
        actualType = expr(&opd1);
        if (formalType != -1 && formalType != actualType) error(PARA_TYPE_NOT_MATCH);
        vec[i].name = opd1;
        i++;
        while (symbol == COMMA)
        {
            getSym();
            if (i < vec.size()) formalType = vec[i].type;
            else formalType = -1;
            actualType = expr(&opd1);
            if (formalType != -1 && formalType != actualType) error(PARA_TYPE_NOT_MATCH);
            vec[i].name = opd1;
            i++;
        }
        for (Parm item: vec)
        {
            opd1 = item.name;
            genInterCode(opNo, opd1);
        }
    }
    if (i != vec.size()) error(PARA_NUM_NOT_MATCH);
    fout << "<值参数表>" << endl;
}

void defineRetFunc()
{
    global = false;
    string opd1;
    int opNo = INTER_OP_LABEL;
    kind = FUNC;
    declareHead();
    opd1 = name;
    genInterCode(opNo, opd1);
    if (symbol == LPARENT) getSym();
    else error();
    paraTable();
    insertSymTable(name, kind, type, value, offset);
    string funcName = name;
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    if (symbol == LBRACE) getSym();
    else error();
    bool isLeafFunc = true;
    callFuncName.clear();
    compStatement();
    if (!callFuncName.empty()) isLeafFunc = false;
    globalSymTable[funcName].value = (int)isLeafFunc;
    if (symbol == RBRACE) getSym();
    else error();
    resetOffset(funcName);
    clearLocalSymTable(funcName);
    fout << "<有返回值函数定义>" << endl;
}

void defineRetFunc_3()
{
    global = false;
    string opd1 = name;
    int opNo = INTER_OP_LABEL;
    genInterCode(opNo, opd1);
    if (symbol == LPARENT) getSym();
    else error();
    paraTable();
    insertSymTable(name, kind, type, value, offset);
    string funcName = name;
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    if (symbol == LBRACE) getSym();
    else error();
    bool isLeafFunc = true;
    callFuncName.clear();
    compStatement();
    if (!callFuncName.empty()) isLeafFunc = false;
    globalSymTable[funcName].value = (int)isLeafFunc;
    if (symbol == RBRACE) getSym();
    else error();
    resetOffset(funcName);
    clearLocalSymTable(funcName);
    fout << "<有返回值函数定义>" << endl;
}

void defineNonRetFunc()
{
    global = false;
    string opd1;
    int opNo = INTER_OP_LABEL;
    kind = FUNC;
    if (symbol == VOIDTK)
    {
        type = retType = VOID;
        getSym();
    }
    else error();
    if (symbol == IDENFR)
    {
        opd1 = name = token;
        getSym();
    }
    else error();
    genInterCode(opNo, opd1);
    if (symbol == LPARENT) getSym();
    else error();
    paraTable();
    insertSymTable(name, kind, type, value, offset);
    string funcName = name;
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    if (symbol == LBRACE) getSym();
    else error();
    bool isLeafFunc = true;
    callFuncName.clear();
    compStatement();
    if (!callFuncName.empty()) isLeafFunc = false;
    globalSymTable[funcName].value = (int)isLeafFunc;
    if (symbol == RBRACE) getSym();
    else error();
    resetOffset(funcName);
    clearLocalSymTable(funcName);
    fout << "<无返回值函数定义>" << endl;
}

void defineNonRetFunc_2()
{
    global = false;
    string opd1;
    int opNo = INTER_OP_LABEL;
    if (symbol == IDENFR)
    {
        opd1 = name = token;
        getSym();
    }
    else error();
    genInterCode(opNo, opd1);
    if (symbol == LPARENT) getSym();
    else error();
    paraTable();
    insertSymTable(name, kind, type, value, offset);
    string funcName = name;
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    if (symbol == LBRACE) getSym();
    else error();
    bool isLeafFunc = true;
    callFuncName.clear();
    compStatement();
    if (!callFuncName.empty()) isLeafFunc = false;
    globalSymTable[funcName].value = (int)isLeafFunc;
    if (symbol == RBRACE) getSym();
    else error();
    resetOffset(funcName);
    clearLocalSymTable(funcName);
    fout << "<无返回值函数定义>" << endl;
}

void callRetFunc(string *opd)
{
    string opd1, opd2, opd3 = "$RET";
    int opNo = INTER_OP_CALL;
    if (symbol == IDENFR)
    {
        opd1 = callFuncName = token;
        getSym();
    }
    else error();
    if (symbol == LPARENT) getSym();
    else error();
    paraValTable();
    genInterCode(opNo, opd1);
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    if (opd != NULL)
    {
        opNo = INTER_OP_ASSIGN;
        opd1 = genTmp();
        genInterCode(opNo, opd1, opd2, opd3);
        *opd = opd1;
    }
    fout << "<有返回值函数调用语句>" << endl;
}

void callRetFunc_2(string *opd)
{
    string opd1 = callFuncName, opd2, opd3 = "$RET";
    int opNo = INTER_OP_CALL;
    if (symbol == LPARENT) getSym();
    else error();
    paraValTable();
    genInterCode(opNo, opd1);
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    if (opd != NULL)
    {
        opNo = INTER_OP_ASSIGN;
        opd1 = genTmp();
        genInterCode(opNo, opd1, opd2, opd3);
        *opd = opd1;
    }
    fout << "<有返回值函数调用语句>" << endl;
}

void callNonRetFunc()
{
    string opd1;
    int opNo = INTER_OP_CALL;
    if (symbol == IDENFR)
    {
        opd1 = callFuncName = token;
        getSym();
    }
    else error();
    if (symbol == LPARENT) getSym();
    else error();
    paraValTable();
    genInterCode(opNo, opd1);
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    fout << "<无返回值函数调用语句>" << endl;
}

void callNonRetFunc_2()
{
    string opd1 = callFuncName;
    int opNo = INTER_OP_CALL;
    if (symbol == LPARENT) getSym();
    else error();
    paraValTable();
    genInterCode(opNo, opd1);
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    fout << "<无返回值函数调用语句>" << endl;
}

int factor(string *opd)
{
    string opd1, opd2, opd3;
    int opNo = INTER_OP_ARRGET;
    int t = INT;
    if (symbol == IDENFR)
    {
        if (undefined(token)) error(UNDEFINED_NAME);
        if (inLocalSymTable(token)) t = localSymTable[token].type;
        else if (inGlobalSymTable(token)) t = globalSymTable[token].type;
        opd2 = token;
        getSym();
        if (symbol == LBRACK)
        {
            getSym();
            int indexType = expr(&opd3);
            if (indexType != INT) error(INDEX_NOT_INT);
            opd1 = genTmp();
            genInterCode(opNo, opd1, opd2, opd3);
            opd2 = opd1;
            if (symbol == RBRACK) getSym();
            else error(MISSING_RBRACK);
        }
        else if (symbol == LPARENT)
        {
            callFuncName = opd2;
            callRetFunc_2(&opd2);
        }
    }
    else if (symbol == LPARENT)
    {
        getSym();
        expr(&opd2);
        if (symbol == RPARENT) getSym();
        else error(MISSING_RPARENT);
    }
    else if (symbol == PLUS || symbol == MINU || symbol == INTCON) integer(&opd2);
    else if (symbol == CHARCON)
    {
        t = CHAR;
        opd2 = to_string(token[0]);
        getSym();
    }
    else error();
    *opd = opd2;
    fout << "<因子>" << endl;
    return t;
}

int term(string *opd)
{
    string opd1, opd2, opd3;
    int opNo;
    int t = factor(&opd2);
    while (symbol == MULT || symbol == DIV)
    {
        t = INT;
        opNo = (symbol == MULT) ? INTER_OP_MUL : INTER_OP_DIV;
        getSym();
        factor(&opd3);
        opd1 = genTmp();
        genInterCode(opNo, opd1, opd2, opd3);
        opd2 = opd1;
    }
    *opd = opd2;
    fout << "<项>" << endl;
    return t;
}

int expr(string *opd)
{
    string opd1, opd2 = "0", opd3;
    int opNo = -1;
    int t = -1;
    if (symbol == PLUS || symbol == MINU)
    {
        t = INT;
        opNo = (symbol == PLUS) ? INTER_OP_ADD : INTER_OP_SUB;
        getSym();
    }
    if (opNo == INTER_OP_ADD) term(&opd2);
    else if (opNo == INTER_OP_SUB)
    {
        term(&opd3);
        opd1 = genTmp();
        genInterCode(opNo, opd1, opd2, opd3);
        opd2 = opd1;
    }
    else t = term(&opd2);
    while (symbol == PLUS || symbol == MINU)
    {
        t = INT;
        opNo = (symbol == PLUS) ? INTER_OP_ADD : INTER_OP_SUB;
        getSym();
        term(&opd3);
        opd1 = genTmp();
        genInterCode(opNo, opd1, opd2, opd3);
        opd2 = opd1;
    }
    *opd = opd2;
    fout << "<表达式>" << endl;
    return t;
}

void readStatement()
{
    string opd1;
    int opNo;
    int t = -1;
    if (symbol == SCANFTK) getSym();
    else error();
    if (symbol == LPARENT) getSym();
    else error();
    if (symbol == IDENFR)
    {
        if (undefined(token)) error(UNDEFINED_NAME);
        if (inLocalSymTable(token)) t = localSymTable[token].type;
        else if (inGlobalSymTable(token)) t = globalSymTable[token].type;
        opNo = (t == INT) ? INTER_OP_READI : INTER_OP_READC;
        opd1 = token;
        genInterCode(opNo, opd1);
        getSym();
    }
    else error();
    while (symbol == COMMA)
    {
        getSym();
        if (symbol == IDENFR)
        {
            if (undefined(token)) error(UNDEFINED_NAME);
            if (inLocalSymTable(token)) t = localSymTable[token].type;
            else if (inGlobalSymTable(token)) t = globalSymTable[token].type;
            opNo = (t == INT) ? INTER_OP_READI : INTER_OP_READC;
            opd1 = token;
            genInterCode(opNo, opd1);
            getSym();
        }
        else error();
    }
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    fout << "<读语句>" << endl;
}

void writeStatement()
{
    string opd1;
    int opNo;
    int t;
    if (symbol == PRINTFTK) getSym();
    else error();
    if (symbol == LPARENT) getSym();
    else error();
    if (symbol == STRCON)
    {
        opNo = INTER_OP_PRINTS;
        opd1 = insertStrTable(token);
        genInterCode(opNo, opd1);
        getSym();
        fout << "<字符串>" << endl;
        if (symbol == COMMA)
        {
            getSym();
            t = expr(&opd1);
            opNo = (t == INT) ? INTER_OP_PRINTI : INTER_OP_PRINTC;
            genInterCode(opNo, opd1);
        }
    }
    else if (symbol == PLUS || symbol == MINU || symbol == IDENFR ||
             symbol == LPARENT || symbol == INTCON || symbol == CHARCON)
    {
        t = expr(&opd1);
        opNo = (t == INT) ? INTER_OP_PRINTI : INTER_OP_PRINTC;
        genInterCode(opNo, opd1);
    }
    else error();
    opNo = INTER_OP_PRINTC;
    opd1 = "10";
    genInterCode(opNo, opd1);
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    fout << "<写语句>" << endl;
}

void retStatement()
{
    string opd1;
    int opNo = INTER_OP_RET;
    int t = -1;
    if (symbol == RETURNTK) getSym();
    else error();
    if (symbol == LPARENT)
    {
        getSym();
        t = expr(&opd1);
        genInterCode(opNo, opd1);
        if (symbol == RPARENT) getSym();
        else error(MISSING_RPARENT);
    }
    if (t == -1)
    {
        opd1 = "";
        genInterCode(opNo, opd1);
    }
    if (retType == VOID && t != -1) error(NON_RET_FUNC_ERR);
    if (retType != VOID && t != retType) error(RET_FUNC_ERR);
    fout << "<返回语句>" << endl;
}

void assignStatement()
{
    string opd1 = token, opd2, opd3;
    int opNo = INTER_OP_ASSIGN;
    if (symbol == IDENFR)
    {
        if (undefined(token)) error(UNDEFINED_NAME);
        if (inLocalSymTable(token))
        {
            if (localSymTable[token].kind == CONST) error(MODIFY_CONST);
        }
        else if (inGlobalSymTable(token))
        {
            if (globalSymTable[token].kind == CONST) error(MODIFY_CONST);
        }
        getSym();
    }
    else error();
    if (symbol == LBRACK)
    {
        opNo = INTER_OP_ARRPUT;
        getSym();
        int indexType = expr(&opd2);
        if (indexType != INT) error(INDEX_NOT_INT);
        if (symbol == RBRACK) getSym();
        else error(MISSING_RBRACK);
    }
    if (symbol == ASSIGN) getSym();
    else error();
    expr(&opd3);
    genInterCode(opNo, opd1, opd2, opd3);
    fout << "<赋值语句>" << endl;
}

void assignStatement_2(string token)
{
    string opd1 = token, opd2, opd3;
    int opNo = INTER_OP_ASSIGN;
    if (inLocalSymTable(token))
    {
        if (localSymTable[token].kind == CONST) error(MODIFY_CONST);
    }
    else if (inGlobalSymTable(token))
    {
        if (globalSymTable[token].kind == CONST) error(MODIFY_CONST);
    }
    if (symbol == LBRACK)
    {
        opNo = INTER_OP_ARRPUT;
        getSym();
        int indexType = expr(&opd2);
        if (indexType != INT) error(INDEX_NOT_INT);
        if (symbol == RBRACK) getSym();
        else error(MISSING_RBRACK);
    }
    if (symbol == ASSIGN) getSym();
    else error();
    expr(&opd3);
    genInterCode(opNo, opd1, opd2, opd3);
    fout << "<赋值语句>" << endl;
}

void condition(string opd, bool opposite)
{
    string opd1, opd2 = "0", opd3 = opd;
    int opNo = -1;
    int t = expr(&opd1);
    if (t == CHAR) error(ILLEGAL_CONDITION);
    if (symbol == LSS || symbol == LEQ || symbol == GRE ||
        symbol == GEQ || symbol == EQL || symbol == NEQ)
    {
        opNo = opposite ? sym2op_opp[symbol] : sym2op[symbol];
        getSym();
        t = expr(&opd2);
        if (t == CHAR) error(ILLEGAL_CONDITION);
    }
    if (opNo == -1) opNo = opposite ? INTER_OP_BEQ : INTER_OP_BNE;
    genInterCode(opNo, opd1, opd2, opd3);
    fout << "<条件>" << endl;
}

void condStatement()
{
    string opd1 = genLbl(), opd2;
    int opNo = -1;
    if (symbol == IFTK) getSym();
    else error();
    if (symbol == LPARENT) getSym();
    else error();
    condition(opd1, true);
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    statement();
    if (symbol == ELSETK)
    {
        opd2 = genLbl();
        opNo = INTER_OP_JUMP;
        genInterCode(opNo, opd2);
        opNo = INTER_OP_LABEL;
        genInterCode(opNo, opd1);
        getSym();
        statement();
        genInterCode(opNo, opd2);
    }
    if (opNo == -1)
    {
        opNo = INTER_OP_LABEL;
        genInterCode(opNo, opd1);
    }
    fout << "<条件语句>" << endl;
}

void step(string *opd)
{
    if (symbol == INTCON)
    {
        *opd = token;
        getSym();
        fout << "<无符号整数>" << endl;
    }
    else error();
    fout << "<步长>" << endl;
}

void loopStatement()
{
    string opd1, opd2, opd3, opd4, opd5;
    int opNo;
    if (symbol == WHILETK)
    {
        opd1 = genLbl();
        opNo = INTER_OP_LABEL;
        genInterCode(opNo, opd1);
        getSym();
        if (symbol == LPARENT) getSym();
        else error();
        opd2 = genLbl();
        condition(opd2, true);
        if (symbol == RPARENT) getSym();
        else error(MISSING_RPARENT);
        statement();
        opNo = INTER_OP_JUMP;
        genInterCode(opNo, opd1);
        opNo = INTER_OP_LABEL;
        genInterCode(opNo, opd2);
    }
    else if (symbol == DOTK)
    {
        opd1 = genLbl();
        opNo = INTER_OP_LABEL;
        genInterCode(opNo, opd1);
        getSym();
        statement();
        if (symbol == WHILETK) getSym();
        else error(MISSING_WHILE);
        if (symbol == LPARENT) getSym();
        else error();
        condition(opd1, false);
        if (symbol == RPARENT) getSym();
        else error(MISSING_RPARENT);
    }
    else if (symbol == FORTK)
    {
        getSym();
        if (symbol == LPARENT) getSym();
        else error();
        if (symbol == IDENFR)
        {
            if (undefined(token)) error(UNDEFINED_NAME);
            opd1 = token;
            getSym();
        }
        else error();
        if (symbol == ASSIGN) getSym();
        else error();
        expr(&opd3);
        opNo = INTER_OP_ASSIGN;
        genInterCode(opNo, opd1, opd2, opd3);
        if (symbol == SEMICN) getSym();
        else error(MISSING_SEMICN);
        opd1 = genLbl();
        opNo = INTER_OP_LABEL;
        genInterCode(opNo, opd1);
        opd2 = genLbl();
        condition(opd2, true);
        if (symbol == SEMICN) getSym();
        else error(MISSING_SEMICN);
        if (symbol == IDENFR)
        {
            if (undefined(token)) error(UNDEFINED_NAME);
            opd3 = token;
            getSym();
        }
        else error();
        if (symbol == ASSIGN) getSym();
        else error();
        if (symbol == IDENFR)
        {
            if (undefined(token)) error(UNDEFINED_NAME);
            opd4 = token;
            getSym();
        }
        else error();
        if (symbol == PLUS || symbol == MINU)
        {
            opNo = (symbol == PLUS) ? INTER_OP_ADD : INTER_OP_SUB;
            getSym();
        }
        else error();
        step(&opd5);
        if (symbol == RPARENT) getSym();
        else error(MISSING_RPARENT);
        statement();
        genInterCode(opNo, opd3, opd4, opd5);
        opNo = INTER_OP_JUMP;
        genInterCode(opNo, opd1);
        opNo = INTER_OP_LABEL;
        genInterCode(opNo, opd2);
    }
    else error();
    fout << "<循环语句>" << endl;
}

void statement()
{
    if (symbol == WHILETK || symbol == DOTK || symbol == FORTK) loopStatement();
    else if (symbol == IFTK) condStatement();
    else if (symbol == SCANFTK)
    {
        readStatement();
        if (symbol == SEMICN) getSym();
        else error(MISSING_SEMICN);
    }
    else if (symbol == PRINTFTK)
    {
        writeStatement();
        if (symbol == SEMICN) getSym();
        else error(MISSING_SEMICN);
    }
    else if (symbol == RETURNTK)
    {
        hasRet = true;
        retStatement();
        if (symbol == SEMICN) getSym();
        else error(MISSING_SEMICN);
    }
    else if (symbol == LBRACE)
    {
        getSym();
        statementList();
        if (symbol == RBRACE) getSym();
        else error();
    }
    else if (symbol == IDENFR)
    {
        if (undefined(token)) error(UNDEFINED_NAME);
        string tmp = token;
        getSym();
        if (symbol == ASSIGN || symbol == LBRACK) assignStatement_2(tmp);
        else if (symbol == LPARENT)
        {
            callFuncName = tmp;
            if (inGlobalSymTable(callFuncName) && globalSymTable[callFuncName].type == VOID)
                callNonRetFunc_2();
            else callRetFunc_2();
        }
        else error();
        if (symbol == SEMICN) getSym();
        else error(MISSING_SEMICN);
    }
    else if (symbol == SEMICN) getSym();
    else error();
    fout << "<语句>" << endl;
}

void compStatement()
{
    string opd1 = "";
    int opNo = INTER_OP_RET;
    if (symbol == CONSTTK) declareConst();
    if (symbol == INTTK || symbol == CHARTK) declareVar();
    statementList();
    if (retType != VOID && !hasRet) error(RET_FUNC_ERR);
    if (retType == VOID) genInterCode(opNo, opd1);
    hasRet = false;
    fout << "<复合语句>" << endl;
}

void statementList()
{
    while (symbol == WHILETK || symbol == DOTK || symbol == FORTK ||
           symbol == IFTK || symbol == SCANFTK || symbol == PRINTFTK ||
           symbol == RETURNTK || symbol == LBRACE || symbol == IDENFR ||
           symbol == SEMICN) statement();
    fout << "<语句列>" << endl;
}

void mainFunc()
{
    global = false;
    string opd1;
    int opNo = INTER_OP_LABEL;
    kind = FUNC;
    value = 0;
    if (symbol == VOIDTK)
    {
        type = retType = VOID;
        getSym();
    }
    else error();
    if (symbol == MAINTK)
    {
        opd1 = name = token;
        getSym();
    }
    else error();
    genInterCode(opNo, opd1);
    insertSymTable(name, kind, type, 0, offset);
    string funcName = name;
    if (symbol == LPARENT) getSym();
    else error();
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    if (symbol == LBRACE) getSym();
    else error();
    compStatement();
    if (symbol == RBRACE) getSym();
    else error();
    resetOffset(funcName);
    clearLocalSymTable(funcName);
    fout << "<主函数>" << endl;
}

void mainFunc_2()
{
    global = false;
    string opd1;
    int opNo = INTER_OP_LABEL;
    kind = FUNC;
    type = retType = VOID;
    value = 0;
    if (symbol == MAINTK)
    {
        opd1 = name = token;
        getSym();
    }
    else error();
    genInterCode(opNo, opd1);
    insertSymTable(name, kind, type, 0, offset);
    string funcName = name;
    if (symbol == LPARENT) getSym();
    else error();
    if (symbol == RPARENT) getSym();
    else error(MISSING_RPARENT);
    if (symbol == LBRACE) getSym();
    else error();
    compStatement();
    if (symbol == RBRACE) getSym();
    else error();
    resetOffset(funcName);
    clearLocalSymTable(funcName);
    fout << "<主函数>" << endl;
}

void program()
{
    bool hasDeclaredVar = false;
    if (symbol == CONSTTK) declareConst();
    if (symbol == INTTK || symbol == CHARTK || symbol == VOIDTK)
    {
        while (symbol == INTTK || symbol == CHARTK || symbol == VOIDTK || symbol == LPARENT)
        {
            if (symbol == INTTK || symbol == CHARTK)
            {
                type = retType = (symbol == INTTK) ? INT : CHAR;
                getSym();
                if (symbol == IDENFR)
                {
                    name = token;
                    getSym();
                    if (symbol == LBRACK || symbol == COMMA || symbol == SEMICN)
                    {
                        kind = VAR;
                        if (hasDeclaredVar) error();
                        declareVar_3();
                        hasDeclaredVar = true;
                    }
                    else if (symbol == LPARENT)
                    {
                        kind = FUNC;
                        fout << "<声明头部>" << endl;
                        defineRetFunc_3();
                    }
                    else error();
                }
                else error();
            }
            else if (symbol == VOIDTK)
            {
                kind = FUNC;
                type = retType = VOID;
                getSym();
                if (symbol == IDENFR) defineNonRetFunc_2();
            }
            else
            {
                kind = FUNC;
                fout << buf;
                clearBuf();
                fout << "<声明头部>" << endl;
                defineRetFunc_3();
            }
        }
    }
    else error();
    if (symbol == MAINTK) mainFunc_2();
    else error();
    fout << "<程序>" << endl;
}
