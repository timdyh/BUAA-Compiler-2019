#include "err.h"
#include "lex.h"

ofstream ferr;
const char errTypeStr[] = "aaabcdefghijklmnox";

void error(int type)
{
    switch (type) {
        case ILLEGAL_CHAR:
            ferr << line << " " << errTypeStr[type] << endl;
            break;
        case MISSING_SINGLE_QUOTE:
        case MISSING_DOUBLE_QUOTE:
            retract();
            ferr << line << " " << errTypeStr[type] << endl;
            break;
        case REDEFINED_NAME:
        case UNDEFINED_NAME:
        case PARA_NUM_NOT_MATCH:
        case PARA_TYPE_NOT_MATCH:
        case ILLEGAL_CONDITION:
        case NON_RET_FUNC_ERR:
        case RET_FUNC_ERR:
        case INDEX_NOT_INT:
            ferr << line << " " << errTypeStr[type] << endl;
            break;
        case MODIFY_CONST:
        case MISSING_SEMICN:
        case MISSING_RPARENT:
        case MISSING_RBRACK:
        case MISSING_WHILE:
            ferr << preLine << " " << errTypeStr[type] << endl;
            break;
        case DEFINE_CONST_ERROR:
            ferr << line << " " << errTypeStr[type] << endl;
            getSym();
            break;
        default:
            ferr << line << " " << errTypeStr[type] << endl;
            break;
    }
}
