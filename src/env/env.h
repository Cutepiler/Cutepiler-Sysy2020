/* This is the definition of environment variables */
#pragma once 

#include <string>
#include "../tree/tree.h"
// IO

extern std::string FILE_IN;
extern std::string FILE_OUT;
extern std::string AST_NAME; 
extern std::string PROG_IN;
extern std::string PROG_OUT;

extern bool PRINT_AST; 

extern bool EXEC_TAC; 

enum Target {
	AST, 
	TAC, 
	ASM,
	SSA
};

extern Target TARGET; 

// Test

extern bool RUN_TEST; 

// Optimization
// TODO: ID For different optimization

extern int OPT_LEVEL;

static const int ARR_INIT_THH = 8; 

// Space ID for BSS/Data start

static const int BSS_START = 2;
static const int DATA_START = 3;

// Machine Detail 

static int WORD_SIZE = 4; 

// IDs of special functions
static const int MAIN = 0;
static const int GETINT = 1;
static const int GETCH = 2;
static const int GETARRAY = 3;
static const int PUTINT = 4;
static const int PUTCH = 5;
static const int PUTARRAY = 6;
static const int PUTF = 7;
static const int STARTTIME = 8;
static const int STOPTIME = 9;
static const int MEMSET = 10; 

int get_libfunc_id(std::string name);
int libfunc_type(int id); 
// FuncDef::VOID or FuncDef::INT; 

// id base of TAC
// id start from BASE + 1

static const int STRING_BASE = 100000;
static const int USER_FUNC_BASE = 20; 

// Stack Space ID
static const int STACK_SPACE_ID = 1;
  
// callee-saved register length
static const int CALLEE_REGISTERS = 8 + 1; //func_stack_length += 8 + 1, r4 - r11, lr
