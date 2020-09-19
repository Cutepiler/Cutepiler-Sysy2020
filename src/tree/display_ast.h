#include "tree.h"
#include <fstream> 

void ast_print(std::string str); 

class CuteDisplayer {
private: 
    int indent; 
    int tab_size; 
	std::string file_name; 
	std::ofstream oss; 
public:
    CuteDisplayer(); 
    CuteDisplayer(int tab_size);
	CuteDisplayer(int tab_size, std::string file_name);
    void msg(std::string str); 
    void value(std::string name, std::string val); 
    void node(std::string name, int id);
    void incIdent(); 
    void decIdent(); 
};

class DisplayVisitor : public Visitor {
    CuteDisplayer cute; 
public: 
    DisplayVisitor(); 
	DisplayVisitor(int tab_size, std::string file_name);
    virtual void visitTree(Tree* that);
	virtual void visitIdent(Ident* that);
	virtual void visitTopLevel(TopLevel* that);
	virtual void visitArrayIndex(ArrayIndex* that);
	virtual void visitExprInitVal(ExprInitVal* that);
	virtual void visitArrayInitVal(ArrayInitVal* that);
	virtual void visitVarDef(VarDef* that);
	virtual void visitDecl(Decl* that);
	virtual void visitFuncFParam(FuncFParam* that);
	virtual void visitLval(Lval* that);
	virtual void visitAssignStmt(AssignStmt* that);
	virtual void visitExprStmt(ExprStmt* that);
	virtual void visitEmptyStmt(EmptyStmt* that);
	virtual void visitIfStmt(IfStmt* that);
	virtual void visitWhileStmt(WhileStmt* that);
	virtual void visitBreakStmt(BreakStmt* that);
	virtual void visitContinueStmt(ContinueStmt* that);
	virtual void visitReturnStmt(ReturnStmt* that);
	virtual void visitBlock(Block* that);
	virtual void visitFuncDef(FuncDef* that);
	virtual void visitIntConst(IntConst* that);
	virtual void visitUnaryExpr(UnaryExpr* that);
	virtual void visitBinaryExpr(BinaryExpr* that);
	virtual void visitStringExpr(StringExpr* that);
	virtual void visitFuncCall(FuncCall* that);
};
