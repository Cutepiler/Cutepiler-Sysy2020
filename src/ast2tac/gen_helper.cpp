#include "generate_pass.h"
#include <iostream>
#include <string> 
#include "../util/cuteprint.h"
#include <cassert>
#include "../env/env.h"

using std::cerr;
using std::endl; 
using std::string; 
using std::to_string;
using std::pair;
using std::vector;
using std::make_shared;

void GenVisitor::msg(std::string str)
{
//    logger << "[Code Generator] " << str << endl; 
} 
 
TacOpd GenVisitor::get_reg(int reg_id)
{
    return TacOpd(reg_id, OpdType::Reg); 
}

void GenVisitor::gen(TacPtr tac_code)
{
    assert(func != nullptr); 
    assert(!global); 
    func->insts.push_back(tac_code); 
//    logger << "[Code Generated] " << tac_code->to_string() << endl; 
}

void GenVisitor::gen_fill_z(int space_id, TacOpd base, int length)
{
    msg("Fill zero for array (space_id = " + to_string(space_id) + ")" 
        + " of length " + to_string(length));
    auto bas_pos = TacOpd::newReg();
    auto ret = TacOpd::newReg(); 
    if (length > 32 || base.getType() == OpdType::Reg) {
        gen(make_shared<Tac>(TacOpr::Addr, bas_pos, base, TacOpd::newImme(space_id)));
        gen(make_shared<Tac>(TacOpr::Param, TacOpd::newImme(WORD_SIZE*length), TacOpd::newImme(3)));
        gen(make_shared<Tac>(TacOpr::Param, TacOpd::newImme(0), TacOpd::newImme(2)));
        gen(make_shared<Tac>(TacOpr::Param, bas_pos, TacOpd::newImme(1)));
        gen(make_shared<Tac>(TacOpr::Call, TacOpd::newImme(MEMSET), ret));
    } else {
        for (int i = 0; i < length; i++) {
            gen(make_shared<Tac>(TacOpr::Store, 
                                TacOpd::newImme(0), 
                                TacOpd::newImme(base.getVal() + i*WORD_SIZE), 
                                TacOpd::newImme(space_id)));
        }
    }
}

void GenVisitor::gen_array_init(int space_id, TacOpd base, int length, vector<pair<int, Expr*>> init_val)
{
    assert(length <= ARR_INIT_THH);
    sort(init_val.begin(), init_val.end()); 
    int pos = 0;
    auto counter = TacOpd::newReg(); 
    auto tmp = TacOpd::newReg(); 
    gen(make_shared<Tac>(TacOpr::Mov, 
                counter, 
                base));
    for (int i = 0; i < length; i++) {
        if (pos >= init_val.size() || init_val[pos].first != i) {
            gen(make_shared<Tac>(TacOpr::Store,
                        TacOpd::newImme(0), 
                        counter, 
                        TacOpd::newImme(space_id)));
        } else {
            auto expr = init_val[pos++].second; 
            expr->reg_id = tmp; 
            expr->accept(this);
            assert(expr->reg_id == tmp);  
            gen(make_shared<Tac>(TacOpr::Store, 
                        expr->reg_id, 
                        counter, 
                        TacOpd::newImme(space_id)));
        }
        gen(make_shared<Tac>(TacOpr::Add,
                    counter,
                    counter, 
                    TacOpd::newImme(WORD_SIZE)));
    }
}

void GenVisitor::get_offset
(ArrayIndex *acc, ArrayIndex *siz, TacOpd tar_reg)
{
    assert(tar_reg.getType() == OpdType::Reg);
    assert(acc->get_size() <= siz->get_size());
    msg("Computing index polynomial, with dim = " + to_string(siz->get_size()) + 
        ", seek_dim = " + to_string(siz->get_size()));
    
    int dim_acc = acc->get_size();
    int dim_siz = siz->get_size(); 

    auto index = tar_reg;
    auto mul = TacOpd::newReg(); 
    auto tmp = TacOpd::newReg(); 
    gen(make_shared<Tac>(TacOpr::Mov, index, TacOpd::newImme(0)));
    gen(make_shared<Tac>(TacOpr::Mov, mul, TacOpd::newImme(1)));
    for (int i = dim_siz-1; i >= 0; i--) {
        if (i < dim_acc) {
            acc->AIlist[i]->reg_id = tmp;
            acc->AIlist[i]->accept(this);
            gen(make_shared<Tac>(TacOpr::Mul, 
                        tmp, 
                        tmp, 
                        mul));
            gen(make_shared<Tac>(TacOpr::Add, 
                        index,
                        index,
                        tmp));
        }
        if (i > 0) {
            if (siz->AIlist[i]->is_constexp) {
                gen(make_shared<Tac>(TacOpr::Mul, 
                            mul, 
                            mul,
                            TacOpd::newImme(siz->AIlist[i]->get_val())));
            } else {
                assert(siz->AIlist[i]->reg_id.getType() == OpdType::Reg);
                gen(make_shared<Tac>(TacOpr::Mul,
                            mul,
                            mul,
                            siz->AIlist[i]->reg_id));
            } 
            // the size array can be a formal parameter
            // hence not necessarily be constant
        }
    } 
    gen(make_shared<Tac>(TacOpr::Mul,
                         index,
                         index,
                         TacOpd::newImme(WORD_SIZE)));
}

/* TODO: not using Horner's Rule, for better instruction-level parallelism
 *       but may use more registers, further test needed. 
 */ 

