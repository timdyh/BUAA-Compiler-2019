#include "lex.h"
#include "syntax.h"
#include "err.h"
#include "mips.h"
#include "inter.h"

int main()
{
    fin.open("testfile.txt");
    fout.open("output.txt");
    ferr.open("error.txt");
    fmips.open("mips.txt");
    finter.open("inter.txt");
    
    getSym(false);
    program();
    translate();
    
    fin.close();
    fout.close();
    ferr.close();
    fmips.close();
    finter.close();
    
    return 0;
}
