#ifndef TREE_H
#define TREE_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "../tac/tac.h"

using std::vector;
using std::string;
using std::map;

class Visitor;
class Symbol;

class Tree{
  private:
	int ID;//record what it is(using enum)
  public:
	enum {TOPLEVEL=1,DECL,VARDEF,ARRAYINDEX,EXPRINITVAL,ARRAYINITVAL,FUNCDEF,FUNCFPARAM,BLOCK,ASSIGNSTMT,EXPRSTMT,
		  EMPTYSTMT,IFSTMT,WHILESTMT,BREAKSTMT,CONTINUESTMT,RETURNSTMT,LVAL,UNARYEXPR,
		  BINARYEXPR,STRINGEXPR,FUNCCALL,IDENT,INTCONST};
	Tree(int ID);
	int getID();
	virtual void accept(Visitor* v);
	virtual ~Tree() = 0;
};

class Ident : public Tree{
  private:
	Symbol* sym;
	string name;
	bool is_const;
  public:
	Ident(string name);
	void set_symbol(Symbol* sym);
	Symbol* get_symbol();
	void setconst();
	bool isConst() const;  
	string get_name();
	void accept(Visitor* v);
};

class Expr : public Tree{
  private:
	int val;
  public:
	bool is_constexp;
	bool is_condexp;
	bool is_intexp;
	TacOpd reg_id; 
	Expr(int ID);
	void set_val(int x);
	int get_val();
};

//Decl or FuncDef
class CompUnit : public Tree{
  public:
	CompUnit(int ID);
};

class TopLevel : public Tree{
  public:
	vector<CompUnit*> CU;
	TopLevel();
	TopLevel(vector<CompUnit*> CU);
	void addUnit(CompUnit* u);
	void accept(Visitor* v);
};

class ArrayIndex : public Tree{
  public:
	bool is_constAI; //If all Expr are const
	int length; 
	vector<Expr*> AIlist;
	ArrayIndex();
	ArrayIndex(vector<Expr*> AIlist);
	void insert(Expr* exp);
	int get_size();
	void accept(Visitor* v);
};

class InitVal : public Tree{
  public:
	bool is_constIV;
	InitVal(int ID);
};

class ExprInitVal : public InitVal{
  public:
	Expr* num;
	ExprInitVal(Expr* exp);
	void accept(Visitor* v);
};

class ArrayInitVal : public InitVal{
  public:
	vector<InitVal*> IVlist;
	std::vector<std::pair<int,Expr*>> init_value; 
	ArrayInitVal();
	void insert(InitVal* exp);
	int get_size();
	void accept(Visitor* v);
};

class VarDef : public Tree{
  public:
  	Ident* var;
	ArrayIndex* AI;
	InitVal* IV;
	int space_id; 
	bool global; 
	int base; 
	TacOpd reg_id; 
	bool is_const;
	VarDef(Ident* var,ArrayIndex* AI,InitVal* IV,bool is_const);
	void accept(Visitor* v);
};

class Decl : public CompUnit{
  public:
	vector<VarDef*> declist;
	Decl();
	void insert(VarDef* var);
	void accept(Visitor* v);
	void setConst(); 
	void setVar(); 
};

class FuncFParam : public Tree{
  public:
	Ident* var;
	vector<Expr*> AIlist;//If AIlist is empty,param is a number.Otherwise,param is an array and the first element of AIlist is 0
	VarDef *var_def; 
	FuncFParam(Ident* var,vector<Expr*> AIlist);
	void accept(Visitor* v);
};

class Stmt : public Tree{
  public:
	Stmt(int ID);
};

class Lval : public Expr{
  public:
	Ident* var;
	ArrayIndex* AI;
	VarDef *var_def;  
	bool is_param;
	Lval(Ident* var);
	void addAI(Expr* exp);
	void accept(Visitor* v);
};

class AssignStmt : public Stmt{
  public:
	Lval* left;
	Expr* right;
	AssignStmt(Lval* left,Expr* right);
	void accept(Visitor* v);
};

class ExprStmt : public Stmt{
  public:
	Expr* exp;
	ExprStmt(Expr* exp);
	void accept(Visitor* v);
};

class EmptyStmt : public Stmt{
  public:
	EmptyStmt();
	void accept(Visitor* v);
};

class IfStmt : public Stmt{
  public:
	Expr* cond;
	Stmt* ifst;
	Stmt* elst;
	IfStmt(Expr* cond,Stmt* ifst);
	IfStmt(Expr* cond,Stmt* ifst,Stmt* elst);
	void accept(Visitor* v);
};

class WhileStmt : public Stmt{
  public:
	Expr* cond;
	Stmt* whst;
	WhileStmt(Expr* cond,Stmt* whst);
	void accept(Visitor* v);
};

class BreakStmt : public Stmt{
  public:
	BreakStmt();
	void accept(Visitor* v);
};

class ContinueStmt : public Stmt{
  public:
	ContinueStmt();
	void accept(Visitor* v);
};

class ReturnStmt : public Stmt{
  public:
	Expr* exp;
	ReturnStmt();
	ReturnStmt(Expr* exp);
	void accept(Visitor* v);
};

class Block : public Stmt{
  public:
	vector<Decl*> declist;
	vector<Stmt*> stmtlist;
	vector<int> mg;//0 Decl 1 Stmt
	Block();
	void addDecl(Decl* dl);
	void addStmt(Stmt* st);
	void accept(Visitor* v);
};

class FuncDef : public CompUnit{
  private:
	int type;//function type,void or int
  public:
	Ident* var;
	vector<FuncFParam*> FPlist;
	Block* FuncBody;
	int tot_length;
	int func_id; // for code generator
	enum {VOID=1,INT};
	FuncDef(int type,Ident* var,vector<FuncFParam*> FPlist,Block* FuncBody);
	int get_type();
	void accept(Visitor* v);
};

class IntConst : public Expr{
  private:
	int num; 
  public:
	IntConst();
	IntConst(int num);
	void set_num(int num);
	int get_num();
	void accept(Visitor* v);
};

class UnaryExpr : public Expr{
  private:
	int type;//the type of unary operation(using enum)
  public:
	enum {POS=1,NEG,NOT};//+,-,!
	Expr* num;
	UnaryExpr(int type,Expr* num);
	int get_type();
	void accept(Visitor* v);
};

class BinaryExpr : public Expr{
  private:
	int type;//the type of binary operation(using enum)
  public:
	enum {ADD=1,SUB,MUL,DIV,MOD,LT,GT,LQ,GQ,EQ,NEQ,AND,OR};//+,-,*,/,%,<,>,<=,>=,==,!=,&&,||
	Expr* left;
	Expr* right;
	BinaryExpr(int type,Expr* left,Expr* right);
	int get_type(); 
	void accept(Visitor* v);
};

class StringExpr : public Expr{
	string str; 
  public:
	StringExpr(string str);
	string getStr() const; 
	void accept(Visitor* v);
};

class FuncCall : public Expr{
  public:
	Ident* func;
	vector<Expr*> params;
	FuncDef *func_def;
	FuncCall(Ident* func,vector<Expr*> params);
	void accept(Visitor* v);
};

class Visitor{
  public:
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


class Symbol{
  private:
	string name;
	int ID;
  public:
	enum {VARIABLE=1,ARRAYVARIABLE,FUNCTION};
	Symbol();
	Symbol(int ID,string name);
	string get_name();
	int getID();
	virtual bool isFunc() = 0;
};

class Variable : public Symbol{
  private:
	Expr* num;
	bool is_const;
	VarDef* var;
  public:
	Variable(string name);
	Variable(string name,Expr* num);
	void set_val(Expr* num);
	Expr* get_val();
	void set_var(VarDef* var);
	VarDef* get_var();
	bool isFunc();
	void setConst();
	bool isConst();
};

class ArrayVariable : public Symbol{
  private:
	bool is_const;
	VarDef* var;
  public:
	int length;
	vector<int> AI;
	vector<int> suf;
	map<int,Expr*> AI_val;
	ArrayVariable(string name,vector<int> AI);
	void set_val(vector<int> pos,Expr* val);
	Expr* get_val(vector<int> pos);
	void set_var(VarDef* var);
	VarDef* get_var();
	bool isFunc();
	void setConst();
	bool isConst();
};

class Function : public Symbol{
  private:
	FuncDef* fun;
	bool is_library;
  public:
	Function(string name);
	Function(string name,FuncDef* fun);
	void set_fun(FuncDef* fun);
	FuncDef* get_fun();
	bool isFunc();
	void setLibrary();
	bool isLibrary();
};

#endif
