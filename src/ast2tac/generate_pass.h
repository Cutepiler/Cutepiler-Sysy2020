#pragma once

/**
 * Pass 2: Generate TAC Code 
 *  In this pass, the compiler will traverse the AST 
 *  and generate TAC code 
 */

#include "../tac/tac.h"
#include "../tree/tree.h"
#include <list>
#include <vector> 
#include <algorithm>

class BaseInfo {
    vector<SpaceType> v;
public:   
    BaseInfo()
    {
        v.push_back(SpaceType::Abs); 
		v.push_back(SpaceType::Stack);
		v.push_back(SpaceType::BSS);
		v.push_back(SpaceType::Data);
    }
    int newSpace(SpaceType type)
    {
        int last = v.size(); 
        v.push_back(type);
        return last; 
    }
    SpaceType operator () (int space_id) const 
    {
		assert(space_id < v.size());
		assert(space_id >= 0);
        return v[space_id]; 
    } 
}; 

class GenVisitor : public Visitor { 
    TacOpd exit_label; 
	TacOpd while_label; 
    TacProg pg; 
    std::shared_ptr<TacFunc> func; 

	// [space_id, (base, reg)]
	std::map<int, std::pair<TacOpd, TacOpd>> global_var;
	std::set<int> global_defs;
	std::set<int> global_uses;

	std::vector<TacPtr> load_pos;
	std::vector<TacPtr> store_pos;
	
    bool global; 
	enum {READ, WRITE}; 
	BaseInfo space_type; 

	int data_pt;
	int bss_pt; 
	int func_id; 

	TacOpd get_reg(int reg_id); 
    void gen(TacPtr tac_code);
	void gen_fill_z(int space_id, TacOpd base, int length);
	void gen_array_init(int space_id, TacOpd base, int length, std::vector<std::pair<int,Expr*>> init);
	void get_offset(ArrayIndex *acc, ArrayIndex *siz, TacOpd tar_reg);
	void msg(std::string str);
	void expr_tail(TacOpd reg1, bool add_cmp); 

	TacOpd true_branch; 
	TacOpd false_branch; 
	bool is_cond; 
public: 
    GenVisitor(); 
	TacProg getProg() const; 
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

void my_assert(bool res);
