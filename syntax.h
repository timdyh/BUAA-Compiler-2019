#ifndef syntax_h
#define syntax_h

#include <string>
#include <map>
#include <vector>

using namespace std;

enum kinds {CONST, VAR, ARRAY, FUNC};
enum types {INT, CHAR, VOID};

struct Entry {
    int kind;       /* Options: {CONST, VAR, ARRAY, FUNC} */
    int type;       /* For CONST, VAR and ARRAY, means its own type. Options: {INT, CHAR}
                       For FUNC, means return value type. Options: {INT, CHAR, VOID}
                     */
    int value;      /* For CONST, means its value.
                       For ARRAY, means max length.
                       For FUNC, means whether it is a leaf function.
                     */
    int offset;     /* For VAR and ARRAY, means its own offset.
                       For FUNC, means the total offset of all its local variables and arrays.
                     */
};

struct Parm {
    int type;       /* Options: {INT, CHAR} */
    string name;
};

extern map<string, Entry> localSymTable;
extern map<string, Entry> globalSymTable;
extern map<string, map<string, Entry>> totLocalSymTable;
extern map<string, string> strTable;

bool inLocalSymTable(string name);
bool inGlobalSymTable(string name);
int integer(string *opd = NULL);
void defineConst();
void declareConst();
void defineVar();
void defineVar_3();
void declareVar();
void declareVar_3();
void declareHead();
void paraTable();
void paraValTable();
void defineRetFunc();
void defineRetFunc_3();
void defineNonRetFunc();
void defineNonRetFunc_2();
void callRetFunc(string *opd = NULL);
void callRetFunc_2(string *opd = NULL);
void callNonRetFunc();
void callNonRetFunc_2();
int factor(string *opd);
int term(string *opd);
int expr(string *opd);
void readStatement();
void writeStatement();
void retStatement();
void assignStatement();
void assignStatement_2(string token);
void condition(string opd, bool opposite);
void condStatement();
void step(string *opd);
void loopStatement();
void statement();
void compStatement();
void statementList();
void mainFunc();
void mainFunc_2();
void program();

#endif /* syntax_h */
