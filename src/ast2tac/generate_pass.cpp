/**
 * Pass 2: Generate TAC Code 
 *  In this pass, the compiler will traverse the AST 
 *  and generate TAC code 
 */

#include "generate_pass.h"
#include <cassert> 
#include <list>
#include <vector>
#include <map> 
#include <fstream>
#include "../tac/tac.h"
#include "../env/env.h"

using std::make_pair; 
using std::list;
using std::vector;
using std::cerr; 
using std::endl; 
using std::to_string; 
using std::map; 
using std::make_shared;

void my_assert(bool res)
{
    if (!res) {
        std::ofstream fout(FILE_OUT);
        fout << ".syntax unified" << std::endl;
        fout << ".globl main" << std::endl;
        fout << ".type main, %function" << std::endl;
        fout << "main:"<<std::endl;
        fout << "bx lr"<<std::endl;
        exit(0);
    }
}

GenVisitor::GenVisitor() 
{ 
    func = nullptr; 
    msg("Start code generation");
}

void GenVisitor::visitTree(Tree *that) { my_assert(false); }
void GenVisitor::visitIdent(Ident *that) 
{ msg("Visiting Ident, do nothing"); }

void GenVisitor::visitTopLevel(TopLevel *that)
{
    msg("Visiting TopLevel, go through all the CompUnits");
    global = true;  
    pg.bss.length = pg.data.length = 0; 
    is_cond = false; 
    func_id = USER_FUNC_BASE; 
    for (auto comp_unit : that->CU) {
        comp_unit->accept(this); 
    }
    pg.fbase = space_type; 
}

void GenVisitor::visitArrayIndex(ArrayIndex *that)
{
    my_assert(false); // should not be called directly 
}

void GenVisitor::visitExprInitVal(ExprInitVal *that)
{
    my_assert(false); // should not be called directly
}

void GenVisitor::visitArrayInitVal(ArrayInitVal *that)
{
    my_assert(false); // should not be called directly
}

void GenVisitor::visitVarDef(VarDef *that)
{
    msg("Variable definition, dimension = " + to_string(that->AI->get_size())
        + ", global = " + to_string(global));
    that->global = global;  
    if (that->AI->get_size() == 0) {
        // Variable definition
        that->reg_id = TacOpd::newReg();
        // assign a register for the variable
        if (that->IV == nullptr) { 
            if (global) {
                that->space_id = space_type.newSpace(SpaceType::BSS);
                that->base = pg.bss.length * WORD_SIZE; 
                msg("Varabile base = " + to_string(that->base));
                that->reg_id = TacOpd::newImme(that->base);
                pg.bss.length++; 
                pg.space_length[that->space_id] = 1;
                pg.space_base[that->space_id] = that->base;
                // insert the variable into bss 
            }
        } else {
            my_assert(that->IV->getID() == Tree::EXPRINITVAL);
            auto expr_iv = dynamic_cast<ExprInitVal*>(that->IV);
            auto expr = expr_iv->num; 
            if (global) {
                // global variable
                my_assert(expr->is_constexp); 
                that->space_id = space_type.newSpace(SpaceType::Data);
                that->base = pg.data.length * WORD_SIZE; 
                that->reg_id = TacOpd::newImme(that->base); 
                msg("Variable base = " + to_string(that->base));
                pg.space_length[that->space_id] = 1;
                pg.space_base[that->space_id] = that->base;
                pg.data.store.push_back(expr->get_val());
                pg.data.length++; 
                // insert the variable into data
            } else {
                // local variable
                expr->reg_id = that->reg_id; 
                expr->accept(this); 
            }
        }
    } else {
        // Array definition  
        // auto array_iv = dynamic_cast<ArrayInitVal*>(that->IV);
        auto def_index = that->AI; 
        my_assert(def_index->is_constAI);
        if (global) {
            // global array
            if (that->IV == nullptr) {
                // no init value, put into bss 
                that->space_id = space_type.newSpace(SpaceType::BSS);
                that->base = WORD_SIZE * pg.bss.length; 
                msg("Array base = " + to_string(that->base));
                that->reg_id = TacOpd::newImme(that->base); 
                pg.bss.length += def_index->length;
                pg.space_length[that->space_id] = def_index->length;
                pg.space_base[that->space_id] = that->base;
            } else {
                // has init value, which must be constant 
                auto array_iv = dynamic_cast<ArrayInitVal*>(that->IV);
                vector<int> v;
                v.resize(def_index->length);
                that->space_id = space_type.newSpace(SpaceType::Data);
                that->base = WORD_SIZE * pg.data.length; 
                msg("Array base = " + to_string(that->base));
                that->reg_id = TacOpd::newImme(that->base); 
                pg.data.length += def_index->length; 
                pg.space_length[that->space_id] = def_index->length;
                pg.space_base[that->space_id] = that->base;
                // prepare array 
                for (auto pr : array_iv->init_value) {
                    int pos = pr.first;
                    Expr *res = pr.second; 
                    my_assert(res->is_constexp);
                    v[pos] = res->get_val(); 
                }
                // now we get right array, insert into static storage 
                for (auto x : v) {
                    pg.data.store.push_back(x); 
                }
                my_assert(pg.data.store.size() == pg.data.length);
            }
        } else {
            // local array, allocate using ArrDef 
            that->space_id = space_type.newSpace(SpaceType::Stack);
            that->reg_id = TacOpd::newImme(WORD_SIZE * that->base);
            pg.space_length[that->space_id] = def_index->length;
            pg.space_base[that->space_id] = that->base;
            /* _that->base_ has been computed */ 
            if (that->IV == nullptr) {
                // no init value, do nothing 
            } else {
                // set init value manually
                auto array_iv = dynamic_cast<ArrayInitVal*>(that->IV);
                if (def_index->length > ARR_INIT_THH) {
                    auto tmp = TacOpd::newReg(); 
                    gen_fill_z(that->space_id, that->reg_id, def_index->length); 
                    for (auto pr : array_iv->init_value) {
                        int pos = pr.first;
                        Expr *res = pr.second; 
                        // generate code for the result expression 
                        // store the result into res->reg_id 
                        if (!res->is_constexp || res->get_val() != 0) {
                            res->reg_id = TacOpd::newReg(); 
                            res->accept(this); 
                            gen(make_shared<Tac>(TacOpr::Add, 
                                        tmp,
                                        that->reg_id,
                                        TacOpd::newImme(WORD_SIZE * pos)));
                            // compute address
                            gen(make_shared<Tac>(TacOpr::Store, 
                                        res->reg_id, 
                                        tmp,  
                                        TacOpd::newImme(that->space_id)));
                        }
                        // write with store operation 
                    }
                } else {
                    gen_array_init(that->space_id, that->reg_id, 
                                   def_index->length, 
                                   array_iv->init_value); 
                }
            }
        }
    }
}

void GenVisitor::visitDecl(Decl *that)
{
    msg("Visiting variable/array declaration");
    for (auto var_def : that->declist) {
        var_def->accept(this); 
    }
}

void GenVisitor::visitFuncFParam(FuncFParam *that)
{
    my_assert(false);
}

void GenVisitor::visitLval(Lval *that)
{
    auto tmp_cond = is_cond;
    my_assert(that->reg_id.getType() == OpdType::Reg); 
    msg("Visiting Lval as an expression"); 
    my_assert(that->var_def != nullptr); //this 
    if (that->is_constexp) {
        gen(make_shared<Tac>(TacOpr::Mov, that->reg_id, TacOpd::newImme(that->get_val())));
    } else if (that->var_def->AI->get_size() != 0) {
        // array access
        auto target_ai = that->var_def->AI;
        auto index = TacOpd::newReg(); 
        my_assert(target_ai != nullptr); //this
        is_cond = false;
        get_offset(that->AI, target_ai, index);
        is_cond = tmp_cond;
        gen(make_shared<Tac>(TacOpr::Add, 
                    index,
                    index, 
                    that->var_def->reg_id));
        // firstly compute the offset 
        if (that->AI->get_size() == that->var_def->AI->get_size()) {
            // array access into element
            gen(make_shared<Tac>(TacOpr::Load, 
                        that->reg_id,
                        index,
                        TacOpd::newImme(that->var_def->space_id)));
        } else {
            // array access to find address, as argument
            gen(make_shared<Tac>(TacOpr::Addr, 
                        that->reg_id, 
                        index,
                        TacOpd::newImme(that->var_def->space_id)));
        }
    } else if (!that->var_def->global) {
        // use a local variable;
        my_assert(that->var_def->reg_id.getType() == OpdType::Reg);
        gen(make_shared<Tac>(TacOpr::Mov, 
                    that->reg_id, 
                    that->var_def->reg_id));
    } else {
        // use a global variable; 
        my_assert(space_type(that->var_def->space_id) != SpaceType::Stack);
        int id = that->var_def->space_id;
        if (!global_var.count(id))
            global_var[id] = make_pair(that->var_def->reg_id, TacOpd::newReg());
        global_uses.insert(id);
        // cerr << "Global use pos: " << id << endl;
        gen(make_shared<Tac>(TacOpr::Mov, that->reg_id, global_var[id].second));
        /* gen(make_shared<Tac>(TacOpr::Load,
                    that->reg_id,
                    that->var_def->reg_id,
                    TacOpd::newImme(that->var_def->space_id)));*/
    }
#ifndef NAIVE_COND
    expr_tail(that->reg_id, true);
#endif 
}

void GenVisitor::visitAssignStmt(AssignStmt *that)
{
    msg("Visiting assignment statement");
    auto tmp_cond = is_cond;
    auto lval = that->left; 
    auto expr = that->right; 
    auto reg_id = TacOpd::newReg();
    expr->reg_id = reg_id;
    expr->accept(this);  
    my_assert(lval != nullptr);
    my_assert(lval->AI != nullptr);
    // compute right value, for array assignment
    if (lval->AI->get_size() == 0) {
        // assignment to a variable 
        my_assert(lval->var_def != nullptr); 
        msg("Assignment to a variable, global = " 
            + to_string(lval->var_def->global));
        if (lval->var_def->global) {
            msg("I'm here!");
            int id = lval->var_def->space_id;
            // cerr << "global def pos " << id << endl;
            auto base = lval->var_def->reg_id;
            if (!global_var.count(id))
                global_var[id] = make_pair(base, TacOpd::newReg());
            global_defs.insert(id);
            gen(make_shared<Tac>(TacOpr::Mov, global_var[id].second, reg_id));
            /* gen(make_shared<Tac>(TacOpr::Store,
                        reg_id, 
                        lval->var_def->reg_id,
                        TacOpd::newImme(lval->var_def->space_id))); */
        } else {
            my_assert(lval->var_def != nullptr); 
            auto target = lval->var_def->reg_id;
            gen(make_shared<Tac>(TacOpr::Mov,
                        target, 
                        reg_id));
        }
    } else {
        // write to array 
        auto target_ai = lval->var_def->AI; 
        my_assert(target_ai != nullptr); 
        my_assert(lval->AI->get_size() == target_ai->get_size());
        auto index = TacOpd::newReg(); 
        is_cond = false;
        get_offset(lval->AI, target_ai, index);
        is_cond = tmp_cond;
        gen(make_shared<Tac>(TacOpr::Add,
                    index,
                    index,
                    lval->var_def->reg_id)); // base
        gen(make_shared<Tac>(TacOpr::Store, 
                    reg_id, 
                    index,
                    TacOpd::newImme(lval->var_def->space_id)));
    }
}

void GenVisitor::visitExprStmt(ExprStmt *that)
{
    msg("Visiting expression statement, just evaluate"); 
    that->exp->reg_id = TacOpd::newReg(); 
    that->exp->accept(this); 
}

void GenVisitor::visitEmptyStmt(EmptyStmt *that)
{
    msg("Visiting empty statement, do nothing");
}

void GenVisitor::visitIfStmt(IfStmt *that)
{
    if (that->elst != nullptr)
        msg("Visiting if-else statement"); 
    else 
        msg("Visiting if statement");
    my_assert(!global);
    auto expr_reg = TacOpd::newReg(); 
    auto if_branch = TacOpd::newLabel(); 
    auto else_branch = TacOpd::newLabel();
    auto exit_pos = TacOpd::newLabel(); 
    that->cond->reg_id = expr_reg;   
    true_branch = if_branch;
    false_branch = else_branch;   
    // gen conditional expr
    is_cond = true; 
    that->cond->accept(this); 
    is_cond = false; 
    if (that->elst != nullptr) {
        gen(make_shared<Tac>(TacOpr::Labl, if_branch));
        that->ifst->accept(this); 
        gen(make_shared<Tac>(TacOpr::Branch, exit_pos));
        gen(make_shared<Tac>(TacOpr::Labl, else_branch));
        that->elst->accept(this); 
        gen(make_shared<Tac>(TacOpr::Labl, exit_pos));
    } else {
        gen(make_shared<Tac>(TacOpr::Labl, if_branch));
        that->ifst->accept(this); 
        gen(make_shared<Tac>(TacOpr::Labl, else_branch));
    }
}

void GenVisitor::visitWhileStmt(WhileStmt *that)
{
    msg("Visiting while statement");
    my_assert(!global);
    auto tmp_exit = exit_label;  // store the labels for outer loops
    auto tmp_while = while_label; 
    exit_label = TacOpd::newLabel(); // exit label
    while_label = TacOpd::newLabel(); // go back label 
    auto cond_reg = TacOpd::newReg(); 
    true_branch = TacOpd::newLabel(); // set true/false branch
    false_branch = exit_label;        // for condition expr
    gen(make_shared<Tac>(TacOpr::Labl, while_label));
    that->cond->reg_id = cond_reg; 
    // gen condition expr
    is_cond = true; 
    that->cond->accept(this);
    is_cond = false; 
    gen(make_shared<Tac>(TacOpr::Labl, true_branch));
    that->whst->accept(this); 
    gen(make_shared<Tac>(TacOpr::Branch, while_label));
    gen(make_shared<Tac>(TacOpr::Labl, exit_label));
    exit_label = tmp_exit;
    while_label = tmp_while;
}

void GenVisitor::visitBreakStmt(BreakStmt *that)
{
    msg("Visiting break statement, with exit_label.id = " 
        + to_string(exit_label.getId()));
    my_assert(!global);
    gen(make_shared<Tac>(TacOpr::Branch, exit_label));
}

void GenVisitor::visitContinueStmt(ContinueStmt *that)
{
    msg("Visiting continue statement, with while_label.id = " 
        + to_string(while_label.getId()));
    my_assert(!global);
    gen(make_shared<Tac>(TacOpr::Branch, while_label));    
}

void GenVisitor::visitReturnStmt(ReturnStmt *that)
{
    msg("Visiting return statement, has_return_val = " 
        + to_string(that->exp != nullptr)); 
    my_assert(!global);
    if (that->exp != nullptr) {
        auto reg = TacOpd::newReg(); 
        that->exp->reg_id = reg;
        that->exp->accept(this); 
        gen(make_shared<Tac>(TacOpr::Ret, reg));
    } else {
        gen(make_shared<Tac>(TacOpr::Ret));
    }
    store_pos.push_back(func->insts.tail->pred);
}

void GenVisitor::visitBlock(Block *that)
{
    msg("Visiting block");
    my_assert(!global); 
    auto decl = that->declist.begin();
    auto stmt = that->stmtlist.begin(); 
    for (auto index : that->mg) {
        if (index == 0) {
            my_assert(*decl != nullptr);
            (*decl)->accept(this); 
            decl++;
        } else if (index == 1) {
            my_assert(*stmt != nullptr);
            (*stmt)->accept(this);
            stmt++; 
        } else my_assert(false); 
    }
}

void GenVisitor::visitIntConst(IntConst *that)
{
    msg("Visiting integer constant");
    my_assert(that->reg_id.getType() == OpdType::Reg);
    gen(make_shared<Tac>(TacOpr::Mov,
                that->reg_id,
                TacOpd::newImme(that->get_num())));
#ifndef NAIVE_COND
    expr_tail(TacOpd::newImme(that->get_num()), true);
#endif 
}

void GenVisitor::visitUnaryExpr(UnaryExpr *that)
{
    msg("Visiting unary expression");
    my_assert(that->reg_id.getType() == OpdType::Reg);
    that->num->reg_id = that->reg_id; 
    auto reg = that->reg_id; 
    auto tmp_true = true_branch;
    auto tmp_false = false_branch; 
    auto tmp_cond = is_cond; 
    switch(that->get_type()) {
        case UnaryExpr::POS: break; 
        case UnaryExpr::NEG: 
            that->num->accept(this); 
            gen(make_shared<Tac>(TacOpr::Neg, reg, reg));
#ifndef NAIVE_COND
            expr_tail(reg, true);
#endif 
            break;
        case UnaryExpr::NOT: 
#ifdef NAIVE_COND
            that->num->accept(this); 
            gen(make_shared<Tac>(TacOpr::Not, reg, reg));
#else 
            std::swap(true_branch, false_branch);
            that->num->accept(this); 
#endif
            break;  
        default: my_assert(false);  
    }
    true_branch = tmp_true; 
    false_branch = tmp_false; 
    is_cond = tmp_cond; 
}

void GenVisitor::expr_tail(TacOpd reg1, bool add_cmp)
{
    if (is_cond) {
        if (add_cmp) {
            auto tmp_reg = TacOpd::newReg();
            gen(make_shared<Tac>(TacOpr::Ne, tmp_reg, reg1, TacOpd::newImme(0)));
            gen(make_shared<Tac>(TacOpr::Beqz, tmp_reg, false_branch));
            gen(make_shared<Tac>(TacOpr::Branch, true_branch));
        } else {
            gen(make_shared<Tac>(TacOpr::Beqz, reg1, false_branch));
            gen(make_shared<Tac>(TacOpr::Branch, true_branch));
        }
    }
}

void GenVisitor::visitBinaryExpr(BinaryExpr *that)
{
    msg("Visiting binary expression");
    my_assert(that->reg_id.getType() == OpdType::Reg);
    auto tmp_true = true_branch; 
    auto tmp_false = false_branch; 
    auto tmp_cond = is_cond; 
    if (that->get_type() == BinaryExpr::AND) {
        my_assert(is_cond);
        that->left->reg_id = that->reg_id; 
        that->right->reg_id = that->reg_id; 
        auto mid_branch = TacOpd::newLabel(); 
        true_branch = mid_branch; 
        that->left->accept(this); 
        gen(make_shared<Tac>(TacOpr::Labl, mid_branch));
        true_branch = tmp_true;
        false_branch = tmp_false; 
        that->right->accept(this); 
    } else if (that->get_type() == BinaryExpr::OR) {
        my_assert(is_cond);
        that->left->reg_id = that->reg_id; 
        that->right->reg_id = that->reg_id; 
        auto mid_branch = TacOpd::newLabel(); 
        false_branch = mid_branch; 
        that->left->accept(this);
        gen(make_shared<Tac>(TacOpr::Labl, mid_branch));
        true_branch = tmp_true;
        false_branch = tmp_false; 
        that->right->accept(this); 
    } else {
        auto reg1 = that->reg_id;
        auto reg2 = TacOpd::newReg(); 
        that->left->reg_id = reg1;
        that->right->reg_id = reg2; 
        is_cond = false;            // its children are no longer condition expr
        that->left->accept(this); 
        that->right->accept(this); 
        is_cond = tmp_cond; 
        TacOpr res;
        bool add_cmp = false;  
        switch(that->get_type()) {
            case BinaryExpr::ADD: res = TacOpr::Add; add_cmp = true; break; 
            case BinaryExpr::SUB: res = TacOpr::Sub; add_cmp = true; break; 
            case BinaryExpr::MUL: res = TacOpr::Mul; add_cmp = true; break;
            case BinaryExpr::DIV: res = TacOpr::Div; add_cmp = true; break;
            case BinaryExpr::MOD: res = TacOpr::Mod; add_cmp = true; break;
            case BinaryExpr::LT:  res = TacOpr::Lt;  break;
            case BinaryExpr::GT:  res = TacOpr::Gt;  break;
            case BinaryExpr::LQ:  res = TacOpr::Le;  break;
            case BinaryExpr::GQ:  res = TacOpr::Ge;  break;
            case BinaryExpr::EQ:  res = TacOpr::Eq;  break;
            case BinaryExpr::NEQ: res = TacOpr::Ne;  break;
#ifdef NAIVE_COND
            case BinaryExpr::AND: res = TacOpr::And; break;
            case BinaryExpr::OR:  res = TacOpr::Or;  break; 
#endif
            default: break; 
        }
        gen(make_shared<Tac>(res, reg1, reg1, reg2));
#ifndef NAIVE_COND
        expr_tail(reg1, add_cmp);
#endif 
    }
    is_cond = tmp_cond; 
    true_branch = tmp_true;
    false_branch = tmp_false; 
}

void GenVisitor::visitStringExpr(StringExpr *that)
{
    msg("Visiting string expression");
    my_assert(that->reg_id.getType() == OpdType::Reg);
    my_assert(!is_cond);
    int space_id = space_type.newSpace(SpaceType::Data);
    int base = WORD_SIZE * pg.data.length; 
    string str = that->getStr();
    int siz = (str.size() + 1 + 3) / 4; 
    int *val = new int[siz];
    for (int i = 0; i < siz; i++) 
        val[i] = 0; 
    char *val_char = (char*)val;
    for (int i = 0; i < str.size(); i++) {
        *val_char = str[i];
        val_char++;
    }
    pg.data.length += siz; 
    for (int i = 0; i < siz; i++) {
        pg.data.store.push_back(val[i]);
    }
    delete[] val; 
    gen(make_shared<Tac>(TacOpr::Mov, 
                that->reg_id,
                TacOpd::newImme(base)));
    gen(make_shared<Tac>(TacOpr::Addr, 
                that->reg_id,
                that->reg_id, 
                TacOpd::newImme(space_id)));
#ifndef NAIVE_COND
    expr_tail(that->reg_id, true);
#endif 
}

void GenVisitor::visitFuncCall(FuncCall *that)
{
    msg("Visiting function call expression");
    my_assert(that->reg_id.getType() == OpdType::Reg);
    my_assert(!global);
    /* evaluate the parameters, and push them into the stack 
     * in reverse order 
     */
    auto reg = that->reg_id; 
    vector<TacOpd> reg_seq; 
    for (auto itr = that->params.begin(); 
              itr != that->params.end(); itr++) {
        auto expr = *itr; 
        expr->reg_id = TacOpd::newReg();
        expr->accept(this); 
        reg_seq.push_back(expr->reg_id);
    }
    auto pos_before = func->insts.tail->pred;
    int tot = reg_seq.size(), cur = 0;
    for (auto itr = reg_seq.rbegin(); itr != reg_seq.rend(); ++itr) {
        gen(make_shared<Tac>(TacOpr::Param, *itr, TacOpd::newImme(tot - cur)));
        cur++;
    }
    int lib_id = get_libfunc_id(that->func->get_name()); 
    my_assert(that->func_def != nullptr || lib_id != -1);
    int type = lib_id == -1 ? 
                that->func_def->get_type() : 
                libfunc_type(lib_id); 
    int id = lib_id == -1 ? 
             that->func_def->func_id : 
             lib_id; 
    if (type == FuncDef::VOID) {
        gen(make_shared<Tac>(TacOpr::Call, 
                    TacOpd::newImme(id)));
    } else if (type == FuncDef::INT) {
        gen(make_shared<Tac>(TacOpr::Call, 
                    TacOpd::newImme(id),
                    reg));
    } else { 
        my_assert(false);
    } 
    auto pos_after = func->insts.tail->pred;
    if (lib_id == -1) {
        store_pos.push_back(pos_before->succ);
        load_pos.push_back(pos_after);
    }
#ifndef NAIVE_COND
    expr_tail(that->reg_id, true);
#endif 
}

void GenVisitor::visitFuncDef(FuncDef *that)
{
    msg("Visiting function definition, name = " + 
        that->var->get_name());
    my_assert(global);
    if (that->var->get_name() == "main") {
        that->func_id = 0;
    } else { 
        that->func_id = ++func_id; // claim a new function 
    }
    global = false;
    global_var.clear();
    global_uses.clear();
    global_defs.clear();

    load_pos.clear();
    store_pos.clear();
    
    // Step 1 : Filling the parameter list
    vector<int> param_id; 
    for (auto func_fp : that->FPlist) {
        auto var_def = func_fp->var_def; 
        if (var_def->AI->get_size() == 0) {
            msg("claim a variable as parameter");
            var_def->reg_id = TacOpd::newReg(); 
            param_id.push_back(var_def->reg_id.getId());
        } else {
            msg("claim an array as parameter"); 
            my_assert(var_def != nullptr);
            var_def->reg_id = TacOpd::newReg();
            var_def->space_id = 0; // set : absolute address
            my_assert(space_type(0) == SpaceType::Abs);
            param_id.push_back(var_def->reg_id.getId());
        }
    }
    func = make_shared<TacFunc>(that->var->get_name(), param_id, 
                       that->func_id, that->tot_length);
    // Step 2 : dealing with formal parameters as array
    //          compute size of each dimension 
    for (auto func_fp : that->FPlist) {
        auto var_def = func_fp->var_def;
        var_def->global = false; 
        if (var_def->AI->get_size() != 0) {
            for (auto expr : var_def->AI->AIlist) {
                if (expr == nullptr) continue;
                if (expr->is_constexp) {
                    expr->reg_id = TacOpd::newImme(expr->get_val());
                } else {
                    expr->reg_id = TacOpd::newReg();
                    expr->accept(this);
                }
            }
        }
    }
    // Step 3 : copy all the parameters
    for (auto func_fp : that->FPlist) {
        auto var_def = func_fp->var_def;
        auto before = var_def->reg_id;
        var_def->reg_id = TacOpd::newReg(); 
        gen(make_shared<Tac>(TacOpr::Mov, var_def->reg_id, before));
    }
    auto pos_start = func->insts.tail->pred;
    // Step 4 : generate code for function body 
    that->FuncBody->accept(this);
    auto pos_end = func->insts.tail->pred;
    load_pos.push_back(pos_start);
    store_pos.push_back(pos_end->succ);

    for (auto before : load_pos) {
        for (auto use_id : global_uses) {
            auto [base, reg] = global_var[use_id];
            before->insert(make_shared<Tac>(TacOpr::Load, reg, base, TacOpd::newImme(use_id)));
            before = before->succ;
        }
        for (auto def_id : global_defs) {
            auto [base, reg] = global_var[def_id];
            before->insert(make_shared<Tac>(TacOpr::Load, reg, base, TacOpd::newImme(def_id)));
            before = before->succ;
        }
    }
    for (auto after : store_pos) {
        for (auto def_id : global_defs) {
            auto [base, reg] = global_var[def_id];
            after->pred->insert(make_shared<Tac>(TacOpr::Store, reg, base, TacOpd::newImme(def_id)));
        }
    }

    if (that->get_type() == FuncDef::VOID)
        gen(make_shared<Tac>(TacOpr::Ret));
    else 
        gen(make_shared<Tac>(TacOpr::Ret, TacOpd::newImme(0)));
    pg.funcs.push_back(func); 
    func = nullptr; 
    global = true; 
}

TacProg GenVisitor::getProg() const
{
    return pg; 
}
