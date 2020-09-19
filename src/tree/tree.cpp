#include "tree.h"
#include <iostream>
#include <string>

using std::string;

Tree::Tree(int _ID):ID(_ID){}
int Tree::getID(){return ID;}
void Tree::accept(Visitor* v){
	v->visitTree(this);
}
Tree::~Tree(){}


Ident::Ident(string _name):Tree(IDENT),name(_name){
	sym = nullptr;
	is_const = false;
}

void Ident::set_symbol(Symbol* _sym){
	sym=_sym;
}

Symbol* Ident::get_symbol(){
	return sym;
}

void Ident::setconst(){
	is_const=1;
}

string Ident::get_name(){
	return name;
}

void Ident::accept(Visitor* v){
	v->visitIdent(this);
}

bool Ident::isConst() const 
{
	return is_const; 
}

Expr::Expr(int ID):Tree(ID){  
	is_intexp = 1;
	is_condexp = 1;
	is_constexp = 0;
	val = 0;
}

void Expr::set_val(int x){
	val=x;
}

int Expr::get_val(){
	return val;
}


CompUnit::CompUnit(int ID):Tree(ID){}


TopLevel::TopLevel():Tree(TOPLEVEL){CU.clear();}

TopLevel::TopLevel(vector<CompUnit*> _CU):Tree(TOPLEVEL),CU(_CU){}

void TopLevel::addUnit(CompUnit* u){
	CU.push_back(u);
}

void TopLevel::accept(Visitor* v){
	v->visitTopLevel(this);
}

ArrayIndex::ArrayIndex():Tree(ARRAYINDEX){
	length = 0;
	is_constAI = false;
	AIlist.clear();
}

ArrayIndex::ArrayIndex(vector<Expr*> _AIlist):Tree(ARRAYINDEX),AIlist(_AIlist){
	length = 0;
	is_constAI = false;
}

void ArrayIndex::insert(Expr* exp){
	AIlist.push_back(exp);
}

int ArrayIndex::get_size(){
	return AIlist.size();
}

void ArrayIndex::accept(Visitor* v){
	v->visitArrayIndex(this);
}

InitVal::InitVal(int _ID):Tree(_ID){
	is_constIV = false;
}


ExprInitVal::ExprInitVal(Expr* exp):InitVal(EXPRINITVAL),num(exp){}

void ExprInitVal::accept(Visitor* v){
	v->visitExprInitVal(this);
}


ArrayInitVal::ArrayInitVal():InitVal(ARRAYINITVAL){
	IVlist.clear();
	init_value.clear();
}

void ArrayInitVal::insert(InitVal* exp){
	IVlist.push_back(exp);
}

int ArrayInitVal::get_size(){
	return IVlist.size();
}

void ArrayInitVal::accept(Visitor* v){
	v->visitArrayInitVal(this);
}

VarDef::VarDef(Ident* _var,ArrayIndex* _AI,InitVal* _IV,bool _is_const):Tree(VARDEF),var(_var),AI(_AI),IV(_IV),is_const(_is_const){
	global = false;
}

void VarDef::accept(Visitor* v){
	v->visitVarDef(this);
}

Decl::Decl():CompUnit(DECL){
	declist.clear();
}

void Decl::insert(VarDef* var){
	declist.push_back(var);
}

void Decl::accept(Visitor* v){
	v->visitDecl(this);
}

Stmt::Stmt(int ID):Tree(ID){}


Lval::Lval(Ident* _var):Expr(LVAL),var(_var){
	is_param = false;
	AI = new ArrayIndex();
	var_def = nullptr;
}

void Lval::addAI(Expr* exp){
	AI->insert(exp);
}

void Lval::accept(Visitor* v){
	v->visitLval(this);
}


AssignStmt::AssignStmt(Lval* _left,Expr* _right):Stmt(ASSIGNSTMT),left(_left),right(_right){}

void AssignStmt::accept(Visitor* v){
	v->visitAssignStmt(this);
}


ExprStmt::ExprStmt(Expr* _exp):Stmt(EXPRSTMT),exp(_exp){}

void ExprStmt::accept(Visitor* v){
	v->visitExprStmt(this);
}


EmptyStmt::EmptyStmt():Stmt(EMPTYSTMT){}

void EmptyStmt::accept(Visitor* v){
	v->visitEmptyStmt(this);
}


IfStmt::IfStmt(Expr* _cond,Stmt* _ifst):Stmt(IFSTMT),cond(_cond),ifst(_ifst){
	elst=nullptr;
}

IfStmt::IfStmt(Expr* _cond,Stmt* _ifst,Stmt* _elst):Stmt(IFSTMT),cond(_cond),ifst(_ifst),elst(_elst){}

void IfStmt::accept(Visitor* v){
	v->visitIfStmt(this);
}


WhileStmt::WhileStmt(Expr* _cond,Stmt* _whst):Stmt(WHILESTMT),cond(_cond),whst(_whst){}

void WhileStmt::accept(Visitor* v){
	v->visitWhileStmt(this);
}


BreakStmt::BreakStmt():Stmt(BREAKSTMT){}

void BreakStmt::accept(Visitor* v){
	v->visitBreakStmt(this);
}


ContinueStmt::ContinueStmt():Stmt(CONTINUESTMT){}

void ContinueStmt::accept(Visitor* v){
	v->visitContinueStmt(this);
}


ReturnStmt::ReturnStmt():Stmt(RETURNSTMT){
	exp=nullptr;
}

ReturnStmt::ReturnStmt(Expr* _exp):Stmt(RETURNSTMT),exp(_exp){}

void ReturnStmt::accept(Visitor* v){
	v->visitReturnStmt(this);
}


Block::Block():Stmt(BLOCK){
	declist.clear();
	stmtlist.clear();
	mg.clear();
}

void Block::addDecl(Decl* dl){
	declist.push_back(dl);
	mg.push_back(0);
}

void Block::addStmt(Stmt* st){
	stmtlist.push_back(st);
	mg.push_back(1);
}

void Block::accept(Visitor* v){
	v->visitBlock(this);
}


FuncFParam::FuncFParam(Ident* _var,vector<Expr*> _AIlist):Tree(FUNCFPARAM),var(_var),AIlist(_AIlist){
	var_def = nullptr;
}

void FuncFParam::accept(Visitor* v){
	v->visitFuncFParam(this);
}


FuncDef::FuncDef(int _type,Ident* _var,vector<FuncFParam*> _FPlist,Block* _FuncBody):CompUnit(FUNCDEF),type(_type),var(_var),FPlist(_FPlist),FuncBody(_FuncBody){
	tot_length = 0;
	func_id = -1;
}

int FuncDef::get_type(){
	return type;
}

void FuncDef::accept(Visitor* v){
	v->visitFuncDef(this);
}

void Decl::setConst()
{
	for (auto decl : declist) {
		decl->is_const = true; 
	}
}

void Decl::setVar()
{
	for (auto decl : declist) {
		decl->is_const = false; 
	}
}


IntConst::IntConst():Expr(INTCONST){
	num = 0;
	set_val(0);
	is_constexp = 1;
}

IntConst::IntConst(int _num):Expr(INTCONST),num(_num){
	set_val(_num);
	is_constexp = 1;
}

void IntConst::set_num(int _num){
	num=_num;
}

int IntConst::get_num(){
	return num;
}

void IntConst::accept(Visitor* v){
	v->visitIntConst(this);
}


UnaryExpr::UnaryExpr(int _type,Expr* _num):Expr(UNARYEXPR),type(_type),num(_num){}

int UnaryExpr::get_type(){
	return type;
}

void UnaryExpr::accept(Visitor* v){
	v->visitUnaryExpr(this);
}


BinaryExpr::BinaryExpr(int _type,Expr* _left,Expr* _right):Expr(BINARYEXPR),type(_type),left(_left),right(_right){}

int BinaryExpr::get_type(){
	return type;
}

void BinaryExpr::accept(Visitor* v){
	v->visitBinaryExpr(this);
}

StringExpr::StringExpr(std::string str):Expr(STRINGEXPR) 
{
	this->str = str; 
}

void StringExpr::accept(Visitor* v){
	v->visitStringExpr(this);
}

string StringExpr::getStr() const
{
	return this->str; 
}


FuncCall::FuncCall(Ident* _func,vector<Expr*> _params):Expr(FUNCCALL),func(_func),params(_params){
	func_def = nullptr;
	for(auto p : _params){
		if (p->getID() == Tree::LVAL){
			Lval* q = dynamic_cast<Lval*>(p);
			q->is_param = 1;
		}
	}
}

void FuncCall::accept(Visitor* v){
	v->visitFuncCall(this);
}

void Visitor::visitTree(Tree* that){}
void Visitor::visitIdent(Ident* that){}
void Visitor::visitTopLevel(TopLevel* that){}
void Visitor::visitArrayIndex(ArrayIndex* that){}
void Visitor::visitExprInitVal(ExprInitVal* that){}
void Visitor::visitArrayInitVal(ArrayInitVal* that){}
void Visitor::visitVarDef(VarDef* that){}
void Visitor::visitDecl(Decl* that){}
void Visitor::visitFuncFParam(FuncFParam* that){}
void Visitor::visitLval(Lval* that){}
void Visitor::visitAssignStmt(AssignStmt* that){}
void Visitor::visitExprStmt(ExprStmt* that){}
void Visitor::visitEmptyStmt(EmptyStmt* that){}
void Visitor::visitIfStmt(IfStmt* that){}
void Visitor::visitWhileStmt(WhileStmt* that){}
void Visitor::visitBreakStmt(BreakStmt* that){}
void Visitor::visitContinueStmt(ContinueStmt* that){}
void Visitor::visitReturnStmt(ReturnStmt* that){}
void Visitor::visitBlock(Block* that){}
void Visitor::visitFuncDef(FuncDef* that){}
void Visitor::visitIntConst(IntConst* that){}
void Visitor::visitUnaryExpr(UnaryExpr* that){}
void Visitor::visitBinaryExpr(BinaryExpr* that){}
void Visitor::visitStringExpr(StringExpr* that){}
void Visitor::visitFuncCall(FuncCall* that){}


Symbol::Symbol(){}

Symbol::Symbol(int _ID,string _name):ID(_ID),name(_name){}

string Symbol::get_name(){
	return name;
}

int Symbol::getID(){
	return ID;
}


Variable::Variable(string name):Symbol(VARIABLE,name){
	is_const = 0;
	num = new IntConst(0);
}

Variable::Variable(string name,Expr* _num):Symbol(VARIABLE,name),num(_num){
	is_const = 0;
}

void Variable::set_val(Expr* _num){
	num=_num;
}

Expr* Variable::get_val(){
	return num;
}

void Variable::set_var(VarDef* _var){
	var = _var;
}

VarDef* Variable::get_var(){
	return var;
}

bool Variable::isFunc(){
	return false;
}

void Variable::setConst(){
	is_const = 1;
}

bool Variable::isConst(){
	return is_const;
}


ArrayVariable::ArrayVariable(string name,vector<int> _AI):Symbol(ARRAYVARIABLE,name),AI(_AI){
	suf = AI;
	for(int i=suf.size()-2;i>=0;i--) suf[i]*=suf[i+1];
	is_const = 0;
	length = 1;
	for(auto p : _AI) length*=p;
	AI_val.clear();
}

void ArrayVariable::set_val(vector<int> pos,Expr* val){
	int ps = 0,bs;
	for(int i=0;i<suf.size();i++){
		if (i==suf.size()-1) bs=1;
		else bs=suf[i+1];
		ps+=pos[i]*bs;
	}
	AI_val[ps] = val;
}

Expr* ArrayVariable::get_val(vector<int> pos){
	int ps = 0,bs;
	for(int i=0;i<suf.size();i++){
		if (i==suf.size()-1) bs=1;
		else bs=suf[i+1];
		ps+=pos[i]*bs;
	}
	if (AI_val.find(ps) == AI_val.end()) return new IntConst(0);
	else return AI_val[ps];
}

void ArrayVariable::set_var(VarDef* _var){
	var = _var;
}

VarDef* ArrayVariable::get_var(){
	return var;
}

bool ArrayVariable::isFunc(){
	return false;
}

void ArrayVariable::setConst(){
	is_const = 1;
}

bool ArrayVariable::isConst(){
	return is_const;
}


Function::Function(string name):Symbol(FUNCTION,name){
	is_library = false;
}

Function::Function(string name,FuncDef* _fun):Symbol(FUNCTION,name),fun(_fun){
	is_library = false;
}

void Function::set_fun(FuncDef* _fun){
	fun=_fun;
}

FuncDef* Function::get_fun(){
	return fun;
}

bool Function::isFunc(){
	return true;
}

void Function::setLibrary(){
	is_library = true;
}

bool Function::isLibrary(){
	return is_library;
}