%{
#include "token_type.h"
#include "../../tree/display_ast.h"
#include "../../env/env.h"
#include <vector> 
#include <string> 
#include <cstdio> 
#include <cassert>
#include "../../util/cuteprint.h"

// define semantic type here
#ifndef YYSTYPE_IS_DECLARED
typedef TokenData YYSTYPE;
#define YYSTYPE_IS_DECLARED 1
#endif

#include <iostream>
#include <string>
#include "../../tree/tree.h"

using std::cerr; 
using std::endl;
using std::vector; 

// var and func in `lexer.l`
extern int yylex();
extern int linum; 
extern FILE * yyin;
extern FILE * yyout;


static TopLevel *program;

static void msg(std::string str)
{
    logger << "[Parser] Line " << linum << " :: " << str << endl; 
} 

void yyerror(const char *str)
{
    cerr << "[Error] Syntax Error at line " << linum << ": " << str << endl; 
}

%}

%token P_VOID P_INT P_CONST P_NUM P_STR
%token P_WHILE P_IF P_ELSE P_RETURN P_BREAK P_CONTINUE 
%token P_IDENTIFIER P_AND 
%token P_OR P_LE P_GE P_EQ P_NE 
%token P_MINUS P_POS
%token '+' '-' '*' '/' '%' '=' '<' '>' '!' 
%token ',' ';' '(' ')' '[' ']' '{' '}'

%left P_OR 
%left P_AND 
%nonassoc P_LE P_GE P_EQ P_NE '<' '>' 
%left '+' '-'
%left '*' '/' '%'
%nonassoc P_MINUS P_POS '!'
%nonassoc '['
%nonassoc ')' 
%nonassoc P_ELSE
%nonassoc P_VOID P_INT

%%

 /**  
  * Generation Rules Here 
  */

TopLevel    : /* empty */           
                { 
                    $$.top_level = new TopLevel; 
                    program = $$.top_level; 
                }
            | TopLevel Decl         
                { 
                    $$.top_level = $1.top_level; 
                    $$.top_level->addUnit($2.decl); 
                }
            | TopLevel FuncDef      
                {  
                    $$.top_level = $1.top_level;
                    $$.top_level->addUnit($2.func_def); 
                }
            ;

// 1. Var Definition Part 

Decl        : ConstDecl             
                {
                    $$.decl = $1.decl; 
                }
            | VarDecl               
                {
                    $$.decl = $1.decl; 
                }  
            ;

/**
 * Note: ConstDecl is more expressive than the standard,
 *  since the constant value needs not to be assigned an initial value
 *  in our implementation, i.e. the following statment is valid. 
 * "const int a, b = 10;"
 * Related check about "const" should be done during typecheck.
 */

ConstDecl   : P_CONST P_INT VarDefList ';' 
                { 
                    msg("Const Declaration");
                    $$.decl = $3.decl; 
                    $$.decl->setConst();  
                }
            ;

VarDecl     : P_INT VarDefList ';'         
                { 
                    msg("Variable Declaration"); 
                    $$.decl = $2.decl; 
                    $$.decl->setVar(); 
                }
            ;

VarDefList  : VarDef
                {
                    msg("Start of VarDef List"); 
                    $$.decl = new Decl(); 
                    $$.decl->insert($1.var_def);
                }
            | VarDefList ',' VarDef 
                {
                    msg("Append VarDef List");
                    $$.decl = $1.decl; 
                    assert($$.decl != nullptr); 
                    $$.decl->insert($3.var_def);
                }
            ;

/* Variable defined is considered as array define of zero dimension */
VarDef      : P_IDENTIFIER ArrayIndex '=' InitVal 
                { 
                    msg("VarDef with default value");  
                    $$.var_def = new VarDef(new Ident($1.str), 
                                            $2.array_index, 
                                            $4.init_val, false);
                }   
            | P_IDENTIFIER ArrayIndex             
                { 
                    msg("VarDef without default value");
                    $$.var_def = new VarDef(new Ident($1.str),
                                            $2.array_index,
                                            nullptr, false);  
                }
            ;

/* Note: Maybe empty, i.e. not an array index */
ArrayIndex  : ArrayIndex '[' Expr ']'       
                {
                    msg("ArrayIndex list insert, size = " 
                        + std::to_string($1.array_index->get_size()) 
                        + " + 1");
                    $$.array_index = $1.array_index; 
                    assert($$.array_index != nullptr);
                    $$.array_index->insert($3.expr);
                }
            | /* empty */                   
                {  
                    msg("Start of ArrayIndex list"); 
                    $$.array_index = new ArrayIndex(); 
                }
            ;

InitVal     : Expr                          
                {  
                    msg("Init value as expression"); 
                    $$.init_val = new ExprInitVal($1.expr);
                    // Init value of single element 
                }
            | '{' '}'                       
                {
                    msg("Init value as (empty) array assignment"); 
                    $$.init_val = new ArrayInitVal(); 
                }
            | '{' InitValList '}'           
                {  
                    msg("Init value as (non-empty) array assignment"); 
                    $$.init_val = $2.array_init_val;
                }
            ;

InitValList : InitValList ',' InitVal       
                {
                    msg("Append InitVal List");
                    $$.array_init_val = $1.array_init_val; 
                    $$.array_init_val->insert($3.init_val);
                }
            | InitVal                       
                {
                    msg("Start of InitVal List"); 
                    $$.array_init_val = new ArrayInitVal();
                    $$.array_init_val->insert($1.init_val);
                }
            ;

// 2. Function Definition Part 

FuncDef     : P_INT P_IDENTIFIER '(' FuncFParamListE ')' Block 
                { 
                    msg("Function define with integer return value"); 
                    $$.func_def = new FuncDef(FuncDef::INT, 
                                            new Ident($2.str),
                                            $4.fparam_list, 
                                            $6.block);
                }
            | P_VOID P_IDENTIFIER '(' FuncFParamListE ')' Block 
                { 
                    msg("Function define with no return value");  
                    $$.func_def = new FuncDef(FuncDef::VOID, 
                                            new Ident($2.str),
                                            $4.fparam_list, 
                                            $6.block);
                }
            ;

FuncFParamListE : FuncFParamList 
                    { 
                        $$.fparam_list = $1.fparam_list;
                    }
                | /* empty */    
                    { 
                        $$.fparam_list.clear(); 
                    }
                ;

FuncFParamList : FuncFParamList ',' FuncFParam         
                    {  
                        msg("Append function formal param list");
                        $$.fparam_list = $1.fparam_list; 
                        $$.fparam_list.push_back($3.fparam); 
                    } 
               | FuncFParam                            
                    {  
                        msg("Start of function formal param List"); 
                        $$.fparam_list.push_back($1.fparam);
                    }
               ;

FuncFParam  : P_INT P_IDENTIFIER                       
                {  
                    msg("Integer as formal parameter");
                    $$.fparam = new FuncFParam(new Ident($2.str), vector<Expr*>());
                }
            | P_INT P_IDENTIFIER '[' ']' ArrayIndex    
                {
                    msg("Array as formal parameter");
                    assert($5.array_index != nullptr); 
                    vector<Expr*> ai_list = $5.array_index->AIlist; 
                    delete $5.array_index;  
                    // will not used 
                    vector<Expr*> ai_list_p = {nullptr};
                    for (auto expr : ai_list) {
                        ai_list_p.push_back(expr); 
                    }
                    $$.fparam = new FuncFParam(new Ident($2.str), ai_list_p); 
                }     
            ;

// 3. Statements 

Block       : '{' StmtList '}'  
                { 
                    $$.block = $2.block;
                } 
            ;

/** 
 * Note that a statement list may contain 
 *  both statement and declaration;
 */
StmtList    : StmtList Stmt    
                {
                    msg("Append statement list with statement");
                    $$.block = $1.block; 
                    $$.block->addStmt($2.stmt);
                }
            | StmtList Decl
                {
                    msg("Append statement list with declaration");
                    $$.block = $1.block;
                    $$.block->addDecl($2.decl); 
                }
            | /* empty */       
                { 
                    msg("Start of statement list"); 
                    $$.block = new Block(); 
                }
            ;

Stmt        : LVal  '=' Expr  ';'  
                {
                    msg("Assign statement");
                    $$.stmt = new AssignStmt($1.lval, $3.expr);
                }
            | Block                
                {
                    msg("Block statement");
                    $$.stmt = $1.block; 
                } 
            | Expr  ';'            
                {
                    msg("Expr statement");
                    $$.stmt = new ExprStmt($1.expr); 
                }
            | ';'                  
                {
                    msg("Empty statement");
                    $$.stmt = new EmptyStmt(); 
                }
            | P_IF '(' Expr ')' Stmt 
                { 
                    msg("If statement");
                    $$.stmt = new IfStmt($3.expr, $5.stmt);
                }
            | P_IF '(' Expr ')' Stmt P_ELSE Stmt 
                { 
                    msg("If-else statement"); 
                    $$.stmt = new IfStmt($3.expr, $5.stmt, $7.stmt);
                }
            | P_WHILE '(' Expr ')' Stmt 
                {
                    msg("While statement");
                    $$.stmt = new WhileStmt($3.expr, $5.stmt);
                }
            | P_BREAK ';'               
                { 
                    msg("Break statement");
                    $$.stmt = new BreakStmt(); 
                }
            | P_CONTINUE ';'            
                {
                    msg("Continue statement");
                    $$.stmt = new ContinueStmt(); 
                }
            | P_RETURN  ';'            
                { 
                    msg("Return statement with no return value");
                    $$.stmt = new ReturnStmt(); 
                }
            | P_RETURN Expr ';'         
                {
                    msg("Return statement with integer value");
                    $$.stmt = new ReturnStmt($2.expr);
                }
            ;

// 4. Expression 

LVal        : P_IDENTIFIER ArrayIndex  
                { 
                    $$.lval = new Lval(new Ident($1.str));
                    $$.lval->AI = $2.array_index; 
                }
            ;
 
Expr        : P_NUM                     
                { 
                    $$.expr = new IntConst($1.val); 
                }
            | P_STR                     
                { 
                    $$.expr = new StringExpr($1.str);
                }
            | LVal                      
                {
                    // left value is a special expression
                    $$.expr = $1.lval; 
                }
            | FuncCall                  
                {
                    $$.expr = $1.func_call;
                }
            | '(' Expr ')'              
                {
                    $$.expr = $2.expr; 
                }
            | '+' Expr %prec P_POS      
                {
                    $$.expr = new UnaryExpr(UnaryExpr::POS, $2.expr); 
                }
            | '-' Expr %prec P_MINUS    
                {
                    $$.expr = new UnaryExpr(UnaryExpr::NEG, $2.expr);
                }
            | '!' Expr                  
                {
                    $$.expr = new UnaryExpr(UnaryExpr::NOT ,$2.expr);
                }
            | Expr '+' Expr             
                {
                    $$.expr = new BinaryExpr(BinaryExpr::ADD, $1.expr, $3.expr); 
                }
            | Expr '-' Expr             
                {
                    $$.expr = new BinaryExpr(BinaryExpr::SUB, $1.expr, $3.expr);
                }
            | Expr '*' Expr             
                { 
                    $$.expr = new BinaryExpr(BinaryExpr::MUL, $1.expr, $3.expr);
                }
            | Expr '/' Expr             
                {
                    $$.expr = new BinaryExpr(BinaryExpr::DIV, $1.expr, $3.expr);
                }
            | Expr '%' Expr             
                {
                    $$.expr = new BinaryExpr(BinaryExpr::MOD, $1.expr, $3.expr);
                }
            | Expr '<' Expr             
                {
                    $$.expr = new BinaryExpr(BinaryExpr::LT, $1.expr, $3.expr);
                }
            | Expr '>' Expr             
                {
                    $$.expr = new BinaryExpr(BinaryExpr::GT, $1.expr, $3.expr);
                }
            | Expr P_LE Expr            
                {
                    $$.expr = new BinaryExpr(BinaryExpr::LQ, $1.expr, $3.expr);
                }
            | Expr P_GE Expr            
                { 
                    $$.expr = new BinaryExpr(BinaryExpr::GQ, $1.expr, $3.expr);
                }
            | Expr P_EQ Expr            
                {
                    $$.expr = new BinaryExpr(BinaryExpr::EQ, $1.expr, $3.expr);
                }
            | Expr P_NE Expr            
                {
                    $$.expr = new BinaryExpr(BinaryExpr::NEQ, $1.expr, $3.expr);
                }
            | Expr P_AND Expr           
                {
                    $$.expr = new BinaryExpr(BinaryExpr::AND, $1.expr, $3.expr);
                }
            | Expr P_OR Expr            
                {
                    $$.expr = new BinaryExpr(BinaryExpr::OR, $1.expr, $3.expr);
                }
            ;

FuncCall    : P_IDENTIFIER '(' FuncRParamListE ')' 
                {
                    $$.func_call = new FuncCall(new Ident($1.str), $3.expr_list);
                }
            ;

FuncRParamList  : Expr                     
                    { 
                        msg("Start of function real param List"); 
                        $$.expr_list = std::vector<Expr*>({$1.expr});
                    }
                | FuncRParamList ',' Expr  
                    {
                        msg("Append function real param list");
                        $$.expr_list = $1.expr_list; 
                        $$.expr_list.push_back($3.expr);
                    }
                ;

FuncRParamListE : FuncRParamList           
                    { 
                        $$.expr_list = $1.expr_list; 
                    }
                | /* empty */              
                    { 
                        $$.expr_list = std::vector<Expr*>(); 
                    }
                ;
%%

TopLevel* run_parser() {
    yyin = fopen(FILE_IN.c_str(), "r+");
    yyout = fopen(FILE_OUT.c_str(), "w+");
    program = nullptr; 
    int ret = yyparse();
    if (ret != 0)
        program = nullptr;  
    else if (PRINT_AST) {
        DisplayVisitor v(4, AST_NAME); 
        program->accept(&v);
    }
    fclose(yyin);
    fclose(yyout);
    return program;
}
