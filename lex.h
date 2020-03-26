#ifndef lex_h
#define lex_h

#include <iostream>
#include <fstream>
#include <string>
#include <map>

using namespace std;

enum symType {
    IDENFR,
    INTCON, CHARCON, STRCON,
    CONSTTK, INTTK, CHARTK,
    VOIDTK, MAINTK, RETURNTK,
    IFTK, ELSETK,
    DOTK, WHILETK, FORTK,
    SCANFTK, PRINTFTK,
    PLUS, MINU, MULT, DIV,
    LSS, LEQ, GRE, GEQ, EQL, NEQ,
    ASSIGN,
    SEMICN, COMMA,
    LPARENT, RPARENT,
    LBRACK, RBRACK,
    LBRACE, RBRACE,
    UNK
};

extern const string symTypeStr[];
extern ifstream fin;
extern ofstream fout;
extern string token, preToken;
extern int symbol, preSymbol;
extern int line, preLine;

void retract();
void getSym(bool print = true);

#endif /* lex_h */
