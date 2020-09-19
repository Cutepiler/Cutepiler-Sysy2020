#include "typecheck.h"
#include "../ast2tac/generate_pass.h"

#include <cstdio>
#include <iostream>

#define max(a,b) (a>b?a:b)

using std::pair;
using std::make_pair;
using std::endl;
using std::to_string;
using std::cout;

Scope::Scope(int _type):type(_type){
	size = 0;
	arr_size = 0;
	symbolmap.clear();
}

int Scope::get_type(){
	return type;
}

void Scope::insert(string name,Symbol* sym){
	symbolmap[name] = sym;
	my_assert(sym != nullptr);
	if (sym->getID() == Symbol::VARIABLE) size++;
	else if (sym->getID() == Symbol::ARRAYVARIABLE){
		ArrayVariable* v = dynamic_cast<ArrayVariable*>(sym);
		my_assert(v != nullptr);
		size += v->length;
		arr_size += v->length;
	}
}

Symbol* Scope::find(string name){
	return symbolmap[name];
}


TypeCheck::TypeCheck(){
	scopestack.clear();
	errormsg.clear();
	has_error = 0;
}

void TypeCheck::msg(string str){
	if (str[0] == 'E' && str[1] == 'r' && str[2] == 'r') has_error = 1;
	errormsg.push_back(str);
	if (has_error)
		my_assert(false);
}

void TypeCheck::Insert(Symbol* sym){
	my_assert(sym != nullptr);
	if (sym->getID() == Symbol::VARIABLE){
		now_size++;
	}
	else if (sym->getID() == Symbol::ARRAYVARIABLE){
		ArrayVariable* v = dynamic_cast<ArrayVariable*>(sym);
		my_assert(v != nullptr);
		now_size += v->length;
		now_arr_size += v->length;
		tot_size += v->length;
	}
	max_size = max(max_size,now_size);
}

void TypeCheck::visitTree(Tree* that){
	my_assert(false);
}

void TypeCheck::visitIdent(Ident* that){
	Symbol* sym;
	int len = scopestack.size();
	for(int i=len-1;i>=0;i--){
		sym = scopestack[i]->find(that->get_name());
		if (sym != nullptr){
			that->set_symbol(sym);
			return;
		}
	}
	that->set_symbol(nullptr);
}

void TypeCheck::visitTopLevel(TopLevel* that){
	Symbol* sym;
	Scope* gb = new Scope(Scope::GLOBAL);
	scopestack.push_back(gb);
	Function* fo = new Function("getint");
	fo->setLibrary();
	gb->insert("getint", fo);
	fo = new Function("getch");
	fo->setLibrary();
	gb->insert("getch", fo);
	fo = new Function("getarray");
	fo->setLibrary();
	gb->insert("getarray", fo);
	fo =new Function("putint");
	fo->setLibrary();
	gb->insert("putint", fo);
	fo =new Function("putch");
	fo->setLibrary();
	gb->insert("putch", fo);
	fo =new Function("putarray");
	fo->setLibrary();
	gb->insert("putarray", fo);
	fo =new Function("putf");
	fo->setLibrary();
	gb->insert("putf", fo);
	fo =new Function("starttime");
	fo->setLibrary();
	gb->insert("starttime", fo);
	fo =new Function("stoptime");
	fo->setLibrary();
	gb->insert("stoptime", fo);
	fo =new Function("memset");
	fo->setLibrary();
	gb->insert("memset", fo);
	for (auto p : that->CU){
		my_assert(p != nullptr);
		p->accept(this);
	}
	sym = gb->find("main");
	if (sym == nullptr || !sym->isFunc()) msg("Error: Function main not found");
	else{
		Function* func = dynamic_cast<Function*>(sym);
		my_assert(func != nullptr);
		if (func->get_fun()->get_type() != FuncDef::INT) msg("Error: The type of function main must be int");
	}
	scopestack.pop_back();
}

void TypeCheck::visitArrayIndex(ArrayIndex* that){
	that->is_constAI = 1;
	that->length = 1;
	for (auto e : that->AIlist){
		if (e != nullptr){
			e->accept(this);
			if (!e->is_constexp){
				that->is_constAI = 0;
				that->length = 0;
			}
			else{
				if (e->is_constexp && e->get_val() < 0) msg("Error: Array index cannot be negative");
				else if (e->is_constexp && e->get_val() != 0) that->length*= e->get_val();//array parameters can also calculate the length
			}
		}
	}
}

void TypeCheck::visitExprInitVal(ExprInitVal* that){
	that->num->accept(this);
	that->is_constIV = that->num->is_constexp;
}

void TypeCheck::visitArrayInitVal(ArrayInitVal* that){
	that->is_constIV = 1;
	for(auto p : that->IVlist){
		my_assert(p != nullptr);
		p->accept(this);
		that->is_constIV&=p->is_constIV;
	}
}

int TypeCheck::Incpos(ArrayVariable* v,vector<int> *pos){
	int m=(*pos).size()-1;
	while (m>=0){
		(*pos)[m]++;
		if ((*pos)[m] == v->AI[m]){
			(*pos)[m] = 0;
			m--;
		}
		else break;
	}
	return m;
}

int TypeCheck::AssignIV(ArrayVariable* v,ArrayInitVal* IV,vector<int> *pos,int high){
	for(auto p : IV->IVlist){
		if (p->getID() == Tree::EXPRINITVAL){
			ExprInitVal* q = dynamic_cast<ExprInitVal*>(p);
			my_assert(q != nullptr);
			v->set_val(*pos,q->num);
			int ps = 0,bs;
			for(int i=0;i<v->suf.size();i++){
				if (i==v->suf.size()-1) bs=1;
				else bs=v->suf[i+1];
				ps+=(*pos)[i]*bs;
			}
			int m = Incpos(v,pos);
			if (m <= high) return m;
		}
		else{
			ArrayInitVal* q = dynamic_cast<ArrayInitVal*>(p);
			my_assert(q != nullptr);
			int m = (*pos).size()-1;
			if ((*pos)[m] != 0) return -12138;
			while (m>=0){
				if ((*pos)[m] == 0) m--;
				else break;
			}
			if (m <= high) m = high + 1;
			int mm = AssignIV(v,q,pos,m);
			if (mm == -12138) return -12138;
			if (mm <= high) return mm;
		}
	}
	int m;
	do{
		m = Incpos(v,pos);
	}while (m > high);
	return m;
}

void TypeCheck::visitVarDef(VarDef* that){
	Symbol* sym;
	sym = scopestack.back()->find(that->var->get_name());
	if (sym != nullptr){
		msg("Error: Redifinition of identifier "+that->var->get_name());
		return;
	}
	that->var->accept(this);
	if (that->AI != nullptr) that->AI->accept(this);
	if (that->IV != nullptr) that->IV->accept(this);
	if (that->IV != nullptr && that->is_const && !that->IV->is_constIV) msg("Error: Constant definition must use constant initial value");
	my_assert(that->AI != nullptr);
	if (that->AI->AIlist.empty()){
		sym = new Variable(that->var->get_name());
		that->var->set_symbol(sym);
		my_assert(scopestack.back() != nullptr);
		scopestack.back()->insert(that->var->get_name(),sym);
		Insert(sym);
		Variable* v = dynamic_cast<Variable*>(sym);
		my_assert(v != nullptr);
		if (that->is_const) v->setConst();
		v->set_var(that);
		if (that->IV != nullptr && that->IV->getID() == Tree::ARRAYINITVAL){
			msg("Error: Initial value type mismatch");
		}
		else if (that->IV != nullptr){
			ExprInitVal* eiv = dynamic_cast<ExprInitVal*>(that->IV);
			my_assert(eiv != nullptr);
			v->set_val(eiv->num);
		}
	}
	else{
		vector<int> dim;
		for(auto p : that->AI->AIlist){
			if (p == nullptr){
				dim.push_back(0);
				continue;
			}
			dim.push_back(p->get_val());
		}
		sym = new ArrayVariable(that->var->get_name(),dim);
		that->var->set_symbol(sym);
		my_assert(scopestack.back() != nullptr);
		scopestack.back()->insert(that->var->get_name(),sym);
		that->base = this->now_arr_size;
		ArrayVariable* av = dynamic_cast<ArrayVariable*>(sym);
		my_assert(av != nullptr);
		Insert(sym);
		ArrayVariable* v = dynamic_cast<ArrayVariable*>(sym);
		my_assert(v != nullptr);
		if (that->is_const) v->setConst();
		v->set_var(that);
		if (that->IV != nullptr && that->IV->getID() == Tree::EXPRINITVAL){
			msg("Error: Initial value type mismatch");
		}
		else if (that->IV != nullptr){
			vector<int> pos;
			int length = that->AI->AIlist.size();
			while (length){
				pos.push_back(0);
				length--;
			}
			ArrayInitVal* aiv = dynamic_cast<ArrayInitVal*>(that->IV);
			my_assert(aiv != nullptr);
			if (AssignIV(v,aiv,&pos,-1) == -12138) msg("Error: Incompatible initial value assignment");
			/*for(int i=0;i<v->AI_val.size();i++){
				aiv->init_value.push_back(make_pair(i,v->AI_val[i]));
			}*/
			for (auto pa : v->AI_val){
				aiv->init_value.push_back(pa);
			}
		}
	}
}

void TypeCheck::visitDecl(Decl* that){
	for(auto p : that->declist){
		my_assert(p != nullptr);
		p->accept(this);
	}
}

void TypeCheck::visitFuncFParam(FuncFParam* that){
	that->var_def = new VarDef(that->var,new ArrayIndex(that->AIlist),nullptr,0);
	that->var_def->accept(this);
}

void TypeCheck::visitLval(Lval* that){
	that->var->accept(this);
	that->AI->accept(this);//Lval->AI is constructed in constructor
	if (that->var->get_symbol() == nullptr){
		msg("Error: Identifier "+that->var->get_name()+" is not defined");
	}
	else{
		Symbol* sym = that->var->get_symbol();
		my_assert(sym != nullptr);
		if (sym->getID() == Symbol::VARIABLE){
			Variable* v = dynamic_cast<Variable*>(sym);
			my_assert(v != nullptr);
			Expr* num = v->get_val();
			if (num != nullptr && num->is_constexp){
				that->set_val(num->get_val());
			}
			that->is_constexp = v->isConst();
			that->var_def = v->get_var();
			if (that->var_def == nullptr) msg("Issue found!");
			if (!that->AI->AIlist.empty()) msg("Error: Identifier "+that->var->get_name()+" is a variable,not a array");
		}
		else if (sym->getID() == Symbol::ARRAYVARIABLE){
			ArrayVariable* v = dynamic_cast<ArrayVariable*>(sym);
			my_assert(v != nullptr);
			bool is_const = true;
			vector<int> pos;
			pos.clear();
			for (auto e : that->AI->AIlist){
				if (e != nullptr){
					if (!e->is_constexp){
						is_const = false;
						break;
					}
					else pos.push_back(e->get_val());
				}
			}
			if (is_const && v->isConst()){
				that->is_constexp = true;
				that->set_val(v->get_val(pos)->get_val());
				msg("Lval val: " + to_string(that->get_val()));
			}
			that->var_def = v->get_var();
			if (that->var_def == nullptr) msg("Issue found!");
			if (!that->is_param && that->AI->AIlist.size() != v->AI.size()) msg("Error: The number of dimensions of array "+that->var->get_name()+" is inconsistent");
		}
		else msg("Error: Identifier "+that->var->get_name()+" is a function,not a valid lval");
	}
}

void TypeCheck::visitAssignStmt(AssignStmt* that){
	that->left->accept(this);
	that->right->accept(this);
	if (!that->right->is_intexp) msg("Error: Incompatible type int = void");
	if (that->left->var->isConst()) msg("Error: Lval in AssignStmt cannot be a constant");
}

void TypeCheck::visitExprStmt(ExprStmt* that){
	that->exp->accept(this);
}

void TypeCheck::visitEmptyStmt(EmptyStmt* that){

}

void TypeCheck::visitIfStmt(IfStmt* that){
	that->cond->accept(this);
	if (!that->cond->is_condexp) msg("Error: Expr in IfStmt must be Cond");
	if (that->ifst->getID() == Tree::BLOCK) scopestack.push_back(new Scope(Scope::LOCAL));
	that->ifst->accept(this);
	if (that->ifst->getID() == Tree::BLOCK){
		this->now_size -= scopestack.back()->size;
		this->now_arr_size -= scopestack.back()->arr_size;
		scopestack.pop_back();
	}
	if (that->elst != nullptr){
		if (that->elst->getID() == Tree::BLOCK) scopestack.push_back(new Scope(Scope::LOCAL));
		that->elst->accept(this);
		if (that->elst->getID() == Tree::BLOCK){
			this->now_size -= scopestack.back()->size;
			this->now_arr_size -= scopestack.back()->arr_size;
			scopestack.pop_back();
		}
	}
}

void TypeCheck::visitWhileStmt(WhileStmt* that){
	that->cond->accept(this);
	if (!that->cond->is_condexp) msg("Error: Expr in IfStmt must be Cond");
	if (that->whst->getID() == Tree::BLOCK) scopestack.push_back(new Scope(Scope::LOOP));
	that->whst->accept(this);
	if (that->whst->getID() == Tree::BLOCK){
		this->now_size -= scopestack.back()->size;
		this->now_arr_size -= scopestack.back()->arr_size;
		scopestack.pop_back();
	}
}

void TypeCheck::visitBreakStmt(BreakStmt* that){
	bool flag = 0;
	for(int i=scopestack.size()-1;i>=0;i--)
		if (scopestack[i]->get_type() == Scope::LOOP){
			flag = 1;
			break;
		}
	if (!flag) msg("Error: Break statement should be used in the loop");
}

void TypeCheck::visitContinueStmt(ContinueStmt* that){
	bool flag = 0;
	for(int i=scopestack.size()-1;i>=0;i--)
		if (scopestack[i]->get_type() == Scope::LOOP){
			flag = 1;
			break;
		}
	if (!flag) msg("Error: Continue statement should be used in the loop");
}

void TypeCheck::visitReturnStmt(ReturnStmt* that){
	if (that->exp != nullptr) that->exp->accept(this);
	if (this->nowfunc == nullptr) msg("Error: Return statement should be used in function");
	else{
		if (that->exp != nullptr){
			if (this->nowfunc->get_type() == FuncDef::VOID) msg("Error: Void type function should not have a return value");
			else if (!that->exp->is_intexp) msg("Error: Int type function should have a int type return value");
		}
		else{
			if (this->nowfunc->get_type() == FuncDef::INT) msg("Error: Int type function should have a return value");
		}
	}
}

void TypeCheck::visitBlock(Block* that){
	int j=0,k=0;
	for(int i=0;i<that->mg.size();i++){
		if (that->mg[i] == 0){
			that->declist[j]->accept(this);
			j++;
		}
		else{
			if(that->stmtlist[k]->getID() == Tree::BLOCK) scopestack.push_back(new Scope(Scope::LOCAL));
			that->stmtlist[k]->accept(this);
			if(that->stmtlist[k]->getID() == Tree::BLOCK) scopestack.pop_back();
			k++;
		}
	}
}

void TypeCheck::visitFuncDef(FuncDef* that){
	Symbol* sym;
	sym = scopestack.back()->find(that->var->get_name());
	if (sym != nullptr){
		msg("Error: Redifinition of identifier "+that->var->get_name());
		return;
	}
	this->nowfunc = that;
	Function* fc = new Function(that->var->get_name(),that);
	that->var->accept(this);
	that->var->set_symbol(fc);
	scopestack.back()->insert(that->var->get_name(),fc);
	scopestack.push_back(new Scope(Scope::FORMAL));
	for(auto fp : that->FPlist) fp->accept(this);
	scopestack.push_back(new Scope(Scope::FUNC));
	this->now_size = this->now_arr_size = this->max_size = this->tot_size = 0;
	if (that->FuncBody != nullptr) that->FuncBody->accept(this);
	that->tot_length = /* that->FPlist.size() + */ this->tot_size;
	scopestack.pop_back();
	scopestack.pop_back();
	this->nowfunc = nullptr;
}

void TypeCheck::visitIntConst(IntConst* that){
}

void TypeCheck::visitUnaryExpr(UnaryExpr* that){
	that->num->accept(this);
	that->is_constexp = that->num->is_constexp;
	that->is_condexp = that->num->is_condexp;
	that->is_intexp = that->num->is_intexp;
	switch (that->get_type()){
		case UnaryExpr::POS :
			if (that->num->is_constexp && that->num->is_intexp) that->set_val(that->num->get_val());
			that->is_condexp = 0;
			break;
		case UnaryExpr::NEG :
			if (that->num->is_constexp && that->num->is_intexp) that->set_val(-that->num->get_val());
			that->is_condexp = 0;
			break;
		case UnaryExpr::NOT :
			if (that->num->is_constexp && that->num->is_condexp) that->set_val(!(that->num->get_val()));
			break;
		default :
			my_assert(false);
	}
}

void TypeCheck::visitBinaryExpr(BinaryExpr* that){
	that->left->accept(this);
	that->right->accept(this);
	that->is_constexp = that->left->is_constexp & that->right->is_constexp;
	that->is_condexp = that->left->is_condexp & that->right->is_condexp;
	that->is_intexp = that->left->is_intexp & that->right->is_intexp;
	switch (that->get_type()){
		case BinaryExpr::ADD :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() + that->right->get_val());
			that->is_condexp = 0;
			break;
		case BinaryExpr::SUB :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() - that->right->get_val());
			that->is_condexp = 0;
			break;
		case BinaryExpr::MUL :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() * that->right->get_val());
			that->is_condexp = 0;
			break;
		case BinaryExpr::DIV :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() / that->right->get_val());
			that->is_condexp = 0;
			break;
		case BinaryExpr::MOD :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() % that->right->get_val());
			that->is_condexp = 0;
			break;
		case BinaryExpr::LT :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() < that->right->get_val());
			that->is_condexp = 1;
			break;
		case BinaryExpr::GT :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() > that->right->get_val());
			that->is_condexp = 1;
			break;
		case BinaryExpr::LQ :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() <= that->right->get_val());
			that->is_condexp = 1;
			break;
		case BinaryExpr::GQ :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() >= that->right->get_val());
			that->is_condexp = 1;
			break;
		case BinaryExpr::EQ :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() == that->right->get_val());
			that->is_condexp = 1;
			break;
		case BinaryExpr::NEQ :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() != that->right->get_val());
			that->is_condexp = 1;
			break;
		case BinaryExpr::AND :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() && that->right->get_val());
			that->is_condexp = 1;
			break;
		case BinaryExpr::OR :
			if (that->is_constexp && that->is_intexp) that->set_val(that->left->get_val() || that->right->get_val());
			that->is_condexp = 1;
			break;
		default :
			my_assert(false);
	}
}

void TypeCheck::visitStringExpr(StringExpr* that){
	that->is_constexp = that->is_condexp = that->is_intexp = 0;
}

void TypeCheck::visitFuncCall(FuncCall* that){
	Symbol* sym;
	my_assert(that->func != nullptr);
	that->func->accept(this);
	sym = that->func->get_symbol();
	if (sym == nullptr) msg("Error: Function " + that->func->get_name() + " not found");
	else if (sym->getID() != Symbol::FUNCTION) msg("Error: Identifier "+sym->get_name()+" is not a function");
	else{
		Function* fc = dynamic_cast<Function*>(sym);
		my_assert(fc != nullptr);
		if (!fc->isLibrary()){
			that->func_def = fc->get_fun();
			my_assert(fc->get_fun() != nullptr);
			if (fc->get_fun()->get_type() == FuncDef::VOID) that->is_intexp = 0;
			if (that->params.size() != fc->get_fun()->FPlist.size()) msg("Error: Number of parameters does not match function "+fc->get_name());
		}
	}
	for(auto e : that->params){
		my_assert(e != nullptr);
		e->accept(this);
	}
}

ostream& operator <<(ostream& out,const TypeCheck& src){
	bool flag = 1;
	if (!src.has_error && flag) out << "[TypeCheck] Type check succeeded" << endl;
	else{
		for(auto s : src.errormsg){
			out << "[TypeCheck] " + s << endl;
		}
	}
	return out;
}