#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "../tree/tree.h"
#include <iostream>
#include <map>
#include <vector>

using std::map;
using std::vector;
using std::ostream;

class Scope{
  private:
	int type;
  public:
	int size;
	int arr_size;
	enum {GLOBAL=1,LOOP,LOCAL,FORMAL,FUNC};
	map<string,Symbol*> symbolmap;
	Scope(int type);
	int get_type();
	void insert(string name,Symbol* sym);
	Symbol* find(string name);
};

class TypeCheck : public Visitor{
  public:
	vector<Scope*> scopestack;
	vector<string> errormsg;
	FuncDef* nowfunc;
	int now_size;
	int now_arr_size;
	int max_size;
	int tot_size;
	bool has_error;
	void msg(string str);
	TypeCheck();
	void Insert(Symbol* sym);
	int gethigh(ArrayVariable* v,vector<int> *pos);
	int Incpos(ArrayVariable* v,vector<int> *pos);
	int AssignIV(ArrayVariable* v,ArrayInitVal* IV,vector<int> *pos,int high);
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
	friend ostream& operator <<(ostream& out,const TypeCheck& src);
};

#endif

