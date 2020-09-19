#include <string>
#include "env.h"

std::string FILE_IN;
std::string FILE_OUT;
std::string AST_NAME; 
std::string PROG_IN;
std::string PROG_OUT;

bool PRINT_AST = false; 

int OPT_LEVEL = 0;

// Target / Directly interpret 

Target TARGET; 
bool EXEC_TAC = false;
bool RUN_TEST = false; 

// return -1 if is NOT a lib function 
int get_libfunc_id(std::string name)
{
    if (name == "getint") 
        return GETINT;
    else if (name == "getch")
        return GETCH; 
    else if (name == "getarray")
        return GETARRAY; 
    else if (name == "putint")
        return PUTINT; 
    else if (name == "putch")
        return PUTCH; 
    else if (name == "putarray")
        return PUTARRAY; 
    else if (name == "putf")
        return PUTF; 
    else if (name == "starttime")
        return STARTTIME;
    else if (name == "stoptime")
        return STOPTIME; 
    else if (name == "memset")
        return MEMSET; 
    else 
        return -1; 
}

int libfunc_type(int id)
{
    int INT = FuncDef::INT; 
    int VOID = FuncDef::VOID; 
    switch (id) {
        case GETINT: 
        case GETCH:  
        case GETARRAY: 
        case MEMSET:
            return INT; 
        case PUTINT: 
        case PUTCH: 
        case PUTARRAY:  
        case PUTF:  
        case STARTTIME: 
        case STOPTIME:
            return VOID;
        default: assert(false);  
    }
}
