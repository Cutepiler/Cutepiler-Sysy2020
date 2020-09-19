/* This is the type of token stream for lexer and parser */
#pragma once 

#include <string>
#include <vector> 
#include "../../tree/tree.h"

struct TokenData {
    int val;
    std::string str;
    Tree *ast_node;
    TopLevel *top_level;
    CompUnit *comp_unit; 
    Decl *decl; 
    FuncDef *func_def;
    FuncFParam *fparam; 
    VarDef *var_def;
    InitVal *init_val; 
    Ident *ident; 
    Expr *expr;
    FuncCall *func_call; 
    ArrayIndex *array_index;
    Block *block; 
    Stmt *stmt; 
    Lval *lval; 
    ArrayInitVal *array_init_val; 
    std::vector<Expr*> expr_list; 
    std::vector<Tree*> ast_list; 
    std::vector<FuncFParam*> fparam_list; 
    TokenData();
    TokenData(int val);
    TokenData(std::string str); 
    TokenData(Tree *ast_node); 
    TokenData(std::vector<Tree*> ast_list); 
    Expr* getExpr(); 
    std::vector<Expr*> getExprList(); 
};

