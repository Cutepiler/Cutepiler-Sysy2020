#include "token_type.h"
#include <cassert>

using std::vector; 

TokenData::TokenData(int val)
{
    this->val = val; 
}

TokenData::TokenData()
{ }

TokenData::TokenData(std::string str)
{
    this->str = str; 
}

TokenData::TokenData(Tree *ast_node)
{
    this->ast_node = ast_node;
}

TokenData::TokenData(std::vector<Tree*> ast_list)
{
    this->ast_list = ast_list; 
}

Expr* TokenData::getExpr()
{
    Expr *expr = dynamic_cast<Expr*>(ast_node);
    assert(expr != nullptr); 
    return expr; 
}

/* For ArrayIndex: 
 *     ast_list -> ast_list' : vector<Expr*>
 * Halt if it fails (which is unexpected) 
 */
vector<Expr*> TokenData::getExprList()
{
    vector<Expr*> expr_list; 
    for (auto node : ast_list) {
        Expr *expr = dynamic_cast<Expr*>(node);
        assert(expr != nullptr); 
        expr_list.push_back(expr);
    }   
    return expr_list; 
}


