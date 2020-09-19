/* display AST */
#include "display_ast.h"
#include "tree.h"
#include <iostream>
#include <string>
#include <cassert>
#include <fstream> 

using std::string; 
using std::to_string; 
using std::cout; 

// TODO: output to file 
void ast_print(std::string str)
{
    cout << str; 
}

CuteDisplayer::CuteDisplayer()
{
    indent = 0;
    tab_size = 4; 
}

CuteDisplayer::CuteDisplayer(int tab_size)
{
    this->tab_size = tab_size; 
}

CuteDisplayer::CuteDisplayer(int tab_size, string file_name)
{
    this->tab_size = tab_size; 
    this->file_name = file_name; 
    oss.open(file_name); 
}

void CuteDisplayer::msg(string str) 
{
    for (int i = 0; i < tab_size*indent; i++)
        oss << " ";
    oss << str + "\n"; 
}

void CuteDisplayer::value(string name, string val)
{
    msg("- [" + name + "] : " + val + ";"); 
}

void CuteDisplayer::node(string name, int id)
{
    msg(name + " (id = " + to_string(id) + ")");
}

void CuteDisplayer::incIdent()
{
    indent++; 
}

void CuteDisplayer::decIdent()
{
    indent--;
}

DisplayVisitor::DisplayVisitor() { }
DisplayVisitor::DisplayVisitor(int tab_size, string file_name) 
{
    cute = CuteDisplayer(tab_size, file_name);
}

void DisplayVisitor::visitTree(Tree *that)
{
    cute.node("Tree", that->getID());
}

void DisplayVisitor::visitTopLevel(TopLevel *that)
{
    cute.node("TopLevel", that->getID());
    cute.value("# CompUnit", to_string(that->CU.size())); 
    cute.incIdent();
    {
        for (auto comp_unit : that->CU) {
            comp_unit->accept(this);
        }
    }
    cute.decIdent(); 
}

void DisplayVisitor::visitIdent(Ident *that)
{
    cute.node("Ident", that->getID());
    cute.value("name", that->get_name());
    cute.value("is_const", to_string(that->isConst())); 
}

void DisplayVisitor::visitArrayIndex(ArrayIndex *that)
{
    cute.node("ArrayIndex", that->getID());
    cute.value("dimension", to_string(that->get_size()));
    cute.incIdent(); 
    {
        for (auto expr : that->AIlist) {
            expr->accept(this); 
        }
    } 
    cute.decIdent(); 
}

void DisplayVisitor::visitExprInitVal(ExprInitVal *that)
{
    cute.node("ExprInitVal", that->getID());
    cute.incIdent();
    {
        that->num->accept(this);
    }
    cute.decIdent();
}

void DisplayVisitor::visitArrayInitVal(ArrayInitVal *that)
{
    cute.node("ArrayInitVal", that->getID());
    cute.value("# terms", to_string(that->IVlist.size()));
    cute.incIdent(); 
    {
        for (auto init_val : that->IVlist) {
            init_val->accept(this);
        }
    }
    cute.decIdent();
}

void DisplayVisitor::visitVarDef(VarDef *that)
{
    cute.node("VarDef", that->getID());
    cute.value("is_const", to_string(that->is_const));
    cute.incIdent();
    {
        that->var->accept(this);
        that->AI->accept(this); 
        if (that->IV != nullptr) {
            that->IV->accept(this);
        }
    }
    cute.decIdent(); 
}

void DisplayVisitor::visitDecl(Decl *that)
{
    cute.node("Decl", that->getID());
    cute.value("dimension", to_string(that->declist.size())); 
    cute.incIdent();
    {
        for (auto var_def : that->declist)
        {
            var_def->accept(this);
        }
    }
    cute.decIdent(); 
}

void DisplayVisitor::visitFuncFParam(FuncFParam *that)
{
    cute.node("FuncFParam", that->getID());
    cute.value("# params", to_string(that->AIlist.size()));
    cute.incIdent();
    {
        that->var->accept(this);
        for (auto ai_list : that->AIlist) {
            ai_list->accept(this);
        }
    }
    cute.decIdent(); 
}

void DisplayVisitor::visitLval(Lval *that)
{
    cute.node("Lval", that->getID());
    cute.incIdent();
    {
        that->var->accept(this);
        that->AI->accept(this); 
    }
    cute.decIdent(); 
}

void DisplayVisitor::visitAssignStmt(AssignStmt *that)
{
    cute.node("AssignStmt", that->getID());
    cute.incIdent();
    {
        that->left->accept(this);
        that->right->accept(this); 
    }
    cute.decIdent(); 
}

void DisplayVisitor::visitExprStmt(ExprStmt *that)
{
    cute.node("ExprStmt", that->getID()); 
    cute.incIdent(); 
    {
        that->exp->accept(this); 
    }
    cute.decIdent();
}

void DisplayVisitor::visitEmptyStmt(EmptyStmt *that)
{
    cute.node("EmptyStmt", that->getID());
}

void DisplayVisitor::visitIfStmt(IfStmt *that)
{
    cute.node("IfStmt", that->getID()); 
    cute.value("has else", to_string(that->elst != nullptr));
    cute.incIdent(); 
    {
        that->cond->accept(this);
        that->ifst->accept(this);
        if(that->elst != nullptr)
            that->elst->accept(this); 
    }
    cute.decIdent(); 
}

void DisplayVisitor::visitWhileStmt(WhileStmt *that)
{
    cute.node("WhileStmt", that->getID()); 
    cute.incIdent(); 
    {
        that->cond->accept(this);
        that->whst->accept(this);
    }
    cute.decIdent(); 
}

void DisplayVisitor::visitBreakStmt(BreakStmt *that)
{
    cute.node("BreakStmt", that->getID());
}

void DisplayVisitor::visitContinueStmt(ContinueStmt *that)
{
    cute.node("ContinueStmt", that->getID());
}

void DisplayVisitor::visitReturnStmt(ReturnStmt *that)
{
    cute.node("ReturnStmt", that->getID());
    cute.value("has return value", to_string(that->exp != nullptr));
    cute.incIdent();
    {
        if (that->exp != nullptr)
            that->exp->accept(this); 
    } 
    cute.decIdent();
}

void DisplayVisitor::visitBlock(Block *that)
{
    cute.node("Block", that->getID());
    cute.value("# decls", to_string(that->declist.size()));
    cute.value("# stmts", to_string(that->stmtlist.size()));
    cute.incIdent();
    {
        auto decl = that->declist.begin();
        auto stmt = that->stmtlist.begin();
        for (int index : that->mg) {
            if (index == 0) {
                assert(decl != that->declist.end());
                (*decl)->accept(this);
                decl++;
            } else {
                assert(stmt != that->stmtlist.end());
                (*stmt)->accept(this);
                stmt++;
            }
        }
    }
    cute.decIdent(); 
}

void DisplayVisitor::visitFuncDef(FuncDef *that)
{
    cute.node("FuncDef", that->getID());
    string type_name; 
    switch(that->get_type()) {
        case FuncDef::VOID: type_name = "void"; break;
        case FuncDef::INT: type_name = "int"; break;
        default: assert(false); break;  
    } 
    cute.value("type", type_name);
    cute.value("# formal param", to_string(that->FPlist.size())); 
    cute.incIdent();
    {
        that->var->accept(this);
        for (auto fp : that->FPlist) {
            fp->accept(this);
        }
        that->FuncBody->accept(this);
    }
    cute.decIdent(); 
}

void DisplayVisitor::visitUnaryExpr(UnaryExpr *that)
{
    cute.node("UnaryExpr", that->getID());
    string type_name; 
    switch(that->get_type()) {
        case UnaryExpr::POS: type_name = "\'+\'"; break; 
        case UnaryExpr::NEG: type_name = "\'-\'"; break;
        case UnaryExpr::NOT: type_name = "\'!\'"; break; 
        default: assert(false); break; 
    }
    cute.value("type", type_name); 
    cute.incIdent();
    {
        that->num->accept(this); 
    }
    cute.decIdent();
}

void DisplayVisitor::visitBinaryExpr(BinaryExpr *that)
{
    cute.node("BinaryExpr", that->getID());
    string type_name; 
    switch(that->get_type()) {
        case BinaryExpr::ADD: type_name = "\'+\'"; break;
        case BinaryExpr::SUB: type_name = "\'-\'"; break;
        case BinaryExpr::MUL: type_name = "\'*\'"; break; 
        case BinaryExpr::DIV: type_name = "\'/\'"; break; 
        case BinaryExpr::MOD: type_name = "\'%\'"; break; 
        case BinaryExpr::LT:  type_name = "\'<\'"; break; 
        case BinaryExpr::GT:  type_name = "\'>\'"; break; 
        case BinaryExpr::LQ:  type_name = "\'<=\'"; break; 
        case BinaryExpr::GQ:  type_name = "\'>=\'"; break; 
        case BinaryExpr::EQ:  type_name = "\'==\'"; break; 
        case BinaryExpr::NEQ: type_name = "\'!=\'"; break; 
        case BinaryExpr::AND: type_name = "\'&&\'"; break; 
        case BinaryExpr::OR:  type_name = "\'||\'"; break;
        default: assert(false); break; 
    }
    cute.value("type", type_name); 
    cute.incIdent(); 
    {
        that->left->accept(this); 
        that->right->accept(this); 
    }
    cute.decIdent(); 
}

void DisplayVisitor::visitStringExpr(StringExpr *that)
{
    cute.node("StringExpr", that->getID());
    cute.value("str", that->getStr());
} 

void DisplayVisitor::visitIntConst(IntConst *that)
{
    cute.node("IntConst", that->getID());
    cute.value("value", to_string(that->get_num()));
} 

void DisplayVisitor::visitFuncCall(FuncCall *that)
{
    cute.node("FuncCall", that->getID()); 
    cute.value("# params", to_string(that->params.size()));
    cute.incIdent(); 
    {
        that->func->accept(this); 
        for (auto expr : that->params) {
            expr->accept(this); 
        }
    }
    cute.decIdent(); 
}

// TODO: More displayer 