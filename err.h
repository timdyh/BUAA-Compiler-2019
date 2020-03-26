#ifndef err_h
#define err_h

#include <iostream>
#include <fstream>

using namespace std;

enum errType {
    ILLEGAL_CHAR, MISSING_SINGLE_QUOTE, MISSING_DOUBLE_QUOTE,       // a
    REDEFINED_NAME,                                                 // b
    UNDEFINED_NAME,                                                 // c
    PARA_NUM_NOT_MATCH,                                             // d
    PARA_TYPE_NOT_MATCH,                                            // e
    ILLEGAL_CONDITION,                                              // f
    NON_RET_FUNC_ERR,                                               // g
    RET_FUNC_ERR,                                                   // h
    INDEX_NOT_INT,                                                  // i
    MODIFY_CONST,                                                   // j
    MISSING_SEMICN,                                                 // k
    MISSING_RPARENT,                                                // l
    MISSING_RBRACK,                                                 // m
    MISSING_WHILE,                                                  // n
    DEFINE_CONST_ERROR,                                             // o
    OTHER_ERR                                                       // x
};

extern ofstream ferr;

void error(int type = OTHER_ERR);

#endif /* err_h */
