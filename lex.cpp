#include "lex.h"
#include "err.h"

ifstream fin;
ofstream fout;
string token, preToken;
int symbol, preSymbol;
int line = 1, preLine = 1;
char c;

map<string, int> reservedWords = {
    {"const", CONSTTK},
    {"int", INTTK},
    {"char", CHARTK},
    {"void", VOIDTK},
    {"main", MAINTK},
    {"if", IFTK},
    {"else", ELSETK},
    {"do", DOTK},
    {"while", WHILETK},
    {"for", FORTK},
    {"scanf", SCANFTK},
    {"printf", PRINTFTK},
    {"return", RETURNTK}
};

const string symTypeStr[] = {
    "IDENFR",
    "INTCON", "CHARCON", "STRCON",
    "CONSTTK", "INTTK", "CHARTK",
    "VOIDTK", "MAINTK", "RETURNTK",
    "IFTK", "ELSETK",
    "DOTK", "WHILETK", "FORTK",
    "SCANFTK", "PRINTFTK",
    "PLUS", "MINU", "MULT", "DIV",
    "LSS", "LEQ", "GRE", "GEQ", "EQL", "NEQ",
    "ASSIGN",
    "SEMICN", "COMMA",
    "LPARENT", "RPARENT",
    "LBRACK", "RBRACK",
    "LBRACE", "RBRACE",
    "UNK"
};

bool isNewLine()
{
    return (c == '\n');
}

char getChar()
{
    c = fin.get();
    if (isNewLine()) line++;
    return c;
}

void retract()
{
    fin.seekg(-1, ios::cur);
    if (isNewLine()) line--;
}

void clearToken()
{
    token.clear();
}

void clearSym()
{
    symbol = UNK;
}

void catToken()
{
    token += c;
}

bool isReserved()
{
    if (reservedWords.find(token) == reservedWords.end())
        return false;
    return true;
}

bool isSpace()
{
    return isspace(c);
}

bool isLetter()
{
    return isalpha(c);
}

bool isDigit()
{
    return isdigit(c);
}

bool isZero()
{
    return (c == '0');
}

bool isUnderline()
{
    return (c == '_');
}

bool isSingleQuote()
{
    return (c == '\'');
}

bool isDoubleQuote()
{
    return (c == '\"');
}

bool isBackslash()
{
    return (c == '\\');
}

bool isPlus()
{
    return (c == '+');
}

bool isMinus()
{
    return (c == '-');
}

bool isMult()
{
    return (c == '*');
}

bool isDiv()
{
    return (c == '/');
}

bool isLess()
{
    return (c == '<');
}

bool isGreater()
{
    return (c == '>');
}

bool isEqual()
{
    return (c == '=');
}

bool isNot()
{
    return (c == '!');
}

bool isSemicolon()
{
    return (c == ';');
}

bool isComma()
{
    return (c == ',');
}

bool isLeftParent()
{
    return (c == '(');
}

bool isRightParent()
{
    return (c == ')');
}

bool isLeftBracket()
{
    return (c == '[');
}

bool isRightBracket()
{
    return (c == ']');
}

bool isLeftBrace()
{
    return (c == '{');
}

bool isRightBrace()
{
    return (c == '}');
}

bool isLegalChar()
{
    return (c == 32 || c == 33 || (c >= 35 && c <= 126));
}

bool isEOF()
{
    return (c == EOF);
}

void getSym(bool print)
{
    preLine = line;
    clearSym();
    clearToken();
    getChar();
    while (isSpace()) getChar();
    if (isLetter() || isUnderline())
    {
        while (isLetter() || isUnderline() || isDigit())
        {
            catToken();
            getChar();
        }
        retract();
        if (isReserved()) symbol = reservedWords[token];
        else symbol = IDENFR;
    }
    else if (isDigit())
    {
        if (isZero()) catToken();
        else
        {
            while (isDigit())
            {
                catToken();
                getChar();
            }
            retract();
        }
        symbol = INTCON;
    }
    else if (isSingleQuote())
    {
        getChar();
        if (!(isPlus() || isMinus() ||  isMult() ||
              isDiv() || isLetter() || isUnderline() ||
              isDigit())) error(ILLEGAL_CHAR);
        catToken();
        getChar();
        if (!isSingleQuote()) error(MISSING_SINGLE_QUOTE);
        symbol = CHARCON;
    }
    else if (isDoubleQuote())
    {
        getChar();
        while (!isDoubleQuote())
        {
            if (isNewLine())
            {
                error(MISSING_DOUBLE_QUOTE);
                break;
            }
            catToken();
            if (isBackslash()) catToken();
            getChar();
        }
        symbol = STRCON;
    }
    else if (isPlus())
    {
        catToken();
        symbol = PLUS;
    }
    else if (isMinus())
    {
        catToken();
        symbol = MINU;
    }
    else if (isMult())
    {
        catToken();
        symbol = MULT;
    }
    else if (isDiv())
    {
        catToken();
        symbol = DIV;
    }
    else if (isLess())
    {
        catToken();
        getChar();
        if (isEqual())
        {
            catToken();
            symbol = LEQ;
        }
        else
        {
            symbol = LSS;
            retract();
        }
    }
    else if (isGreater())
    {
        catToken();
        getChar();
        if (isEqual())
        {
            catToken();
            symbol = GEQ;
        }
        else
        {
            symbol = GRE;
            retract();
        }
    }
    else if (isEqual())
    {
        catToken();
        getChar();
        if (isEqual())
        {
            catToken();
            symbol = EQL;
        }
        else
        {
            symbol = ASSIGN;
            retract();
        }
    }
    else if (isNot())
    {
        catToken();
        getChar();
        if (isEqual()) catToken();
        else error();
        symbol = NEQ;
    }
    else if (isSemicolon())
    {
        catToken();
        symbol = SEMICN;
    }
    else if (isComma())
    {
        catToken();
        symbol = COMMA;
    }
    else if (isLeftParent())
    {
        catToken();
        symbol = LPARENT;
    }
    else if (isRightParent())
    {
        catToken();
        symbol = RPARENT;
    }
    else if (isLeftBracket())
    {
        catToken();
        symbol = LBRACK;
    }
    else if (isRightBracket())
    {
        catToken();
        symbol = RBRACK;
    }
    else if (isLeftBrace())
    {
        catToken();
        symbol = LBRACE;
    }
    else if (isRightBrace())
    {
        catToken();
        symbol = RBRACE;
    }
    else if (!isEOF()) error(OTHER_ERR);
    
    if (print)
        fout << symTypeStr[preSymbol] << " " << preToken << endl;
    preToken = token;
    preSymbol = symbol;
}
