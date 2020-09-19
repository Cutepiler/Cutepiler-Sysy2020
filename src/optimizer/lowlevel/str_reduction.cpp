#include "str_reduction.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <tuple>
#include "../../util/cuteprint.h"
#include "../../env/env.h"

using std::make_shared;
using std::cerr;
using std::endl;
using std::tuple;
using std::make_tuple;

static const int N = WORD_SIZE * 8;

static int get_log(int num)
{
    for (int i = 0; i <= 31; i++)
        if (num == (1 << i))
            return i;
    return -1;
}

static bool opt_mul_pos(TacPtr inst, TacOpd opd, int imm)
{
    int res = get_log(imm);
    if (res != -1) {
        inst->opr = TacOpr::ASL;
        inst->opd2 = opd;
        inst->opd3 = TacOpd::newImme(res);
        return true;
    }
    for (int i = 0; i <= 31; i++) {
        if (((imm >> i) << i) != imm)
            break;
        int des = imm >> i;
        int res = get_log(des - 1);
        if (res != -1) {
            inst->opr = TacOpr::AddLS;
            inst->opd2 = opd;
            inst->opd3 = opd;
            inst->opd4 = TacOpd::newImme(res);
            if (i != 0) {
                inst->insert(make_shared<Tac>(TacOpr::ASL, inst->opd1, inst->opd1, TacOpd::newImme(i)));
            }
            return true;
        }
        res = get_log(des + 1);
        if (res != -1) {
            inst->opr = TacOpr::RsbLS;
            inst->opd2 = opd;
            inst->opd3 = opd;
            inst->opd4 = TacOpd::newImme(res);
            if (i != 0) {
                inst->insert(make_shared<Tac>(TacOpr::ASL, inst->opd1, inst->opd1, TacOpd::newImme(i)));
            }
            return true;
        }
    }
    return false;
}

static bool opt_mul_neg(TacPtr inst, TacOpd opd, int imm)
{
    int res = get_log(-imm);
    if (res != -1) {
        TacOpd reg1 = TacOpd::newReg();
        TacOpd des = inst->opd1;
        inst->opr = TacOpr::Mov;
        inst->opd1 = reg1;
        inst->opd2 = TacOpd::newImme(0);
        inst->opd3 = TacOpd();
        inst->insert(make_shared<Tac>(TacOpr::SubLS, des, reg1, des, TacOpd::newImme(res)));
        return true;
    }
    return false;
}

static bool opt_mul(TacPtr inst, TacOpd opd, int imm)
{
    if (imm >= 0)
        return opt_mul_pos(inst, opd, imm);
    else 
        return opt_mul_neg(inst, opd, imm);
}

std::tuple<int,int,int> multiplier(int d, int prec)
{
    int l = N - __builtin_clz(d-1), sh = l;
    unsigned long long m_low = (1ull << (N+l)) / d;
    unsigned long long m_high = ((1ull << (N+l)) + (1ull << (N+l-prec)))/d;
    while ((m_low >> 1) < (m_high >> 1) && sh > 0) {
        m_low >>= 1;
        m_high >>= 1;
        sh--;
    }
    return make_tuple(m_high, sh, l);
}

static int lowbit(int d)
{ return d & (-d); }

static bool opt_div(TacPtr inst, TacOpd opd, int imm)
{
    if (imm == 0)
        return false;
    assert (imm != 1 && imm != -1); // in peephole
    assert(opd.getType() == OpdType::Reg); // in constant propagation
    int flag = imm >= 0 ? 1 : -1;
    auto tar = inst->opd1;
    auto tmp = TacOpd::newReg(), res = TacOpd::newReg(), tmp2 = TacOpd::newReg();
    int d = abs(imm);
    auto [m, sh, l] = multiplier(d, N-1);
    TacPtr nxt;
    if (d == (1ull << l)) {
        // cerr << "Case 1" << endl;
        if (l != 1) {
            inst->insert(make_shared<Tac>(TacOpr::ASR, res, opd, TacOpd::newImme(l-1)));
            inst->succ->insert(make_shared<Tac>(TacOpr::AddLSR, res, opd, res, TacOpd::newImme(N-l)));
            inst->succ->succ->insert(make_shared<Tac>(TacOpr::ASR, tar, res, TacOpd::newImme(l)));
            nxt = inst->succ->succ->succ;
            inst->remove();
        } else {
            inst->insert(make_shared<Tac>(TacOpr::AddLSR, res, opd, opd, TacOpd::newImme(N-l)));
            inst->succ->insert(make_shared<Tac>(TacOpr::ASR, tar, res, TacOpd::newImme(l)));
            nxt = inst->succ->succ;
            inst->remove();
        }
    } else if (m < (1ull << (N-1))) {
        // cerr << "Case 2" << endl;
        inst->insert(make_shared<Tac>(TacOpr::Mov, tmp2, TacOpd::newImme(m)));
        inst->succ->insert(make_shared<Tac>(TacOpr::Smmul, res, opd, tmp2));
        inst->succ->succ->insert(make_shared<Tac>(TacOpr::ASR, tmp, opd, TacOpd::newImme(N-1))); // tmp = sign(opd)
        inst->succ->succ->succ->insert(make_shared<Tac>(TacOpr::RsbASR, tar, tmp, res, TacOpd::newImme(sh)));
        nxt = inst->succ->succ->succ->succ;
        inst->remove();
    } else {
        // cerr << "Case 3" << endl;
        inst->insert(make_shared<Tac>(TacOpr::Mov, tmp2, TacOpd::newImme(m-(1ull<<N))));
        inst->succ->insert(make_shared<Tac>(TacOpr::Smmul, res, tmp2, opd));
        inst->succ->succ->insert(make_shared<Tac>(TacOpr::Add, res, res, opd));
        inst->succ->succ->succ->insert(make_shared<Tac>(TacOpr::ASR, tmp, opd, TacOpd::newImme(N-1))); // tmp = sign(opd)
        inst->succ->succ->succ->succ->insert(make_shared<Tac>(TacOpr::RsbASR, tar, tmp, res, TacOpd::newImme(sh)));
        nxt = inst->succ->succ->succ->succ->succ;
        inst->remove();
    }
    if (flag == -1) {
        nxt->insert(make_shared<Tac>(TacOpr::Neg, tar, tar));
    }
    return true;
}

static bool opt_div(TacPtr inst, int imm, TacOpd opd)
{
    return false;
}

static bool opt_mod(TacPtr inst, TacOpd opd, int imm)
{
    if (imm == 0)
        return false;
    assert(imm != 1);
    assert(imm != (1 << 31)); 
    assert(imm > 0);          // in peephole
    int k = get_log(imm);
    if (k != -1) {
        auto tmp = TacOpd::newReg();
        if (k > 1) {
            inst->insert(make_shared<Tac>(TacOpr::ASR, tmp, opd, TacOpd::newImme(31)));
            inst->succ->insert(make_shared<Tac>(TacOpr::AddLSR, tmp, opd, tmp, TacOpd::newImme(32-k)));
            inst->succ->succ->insert(make_shared<Tac>(TacOpr::BIC, tmp, tmp, TacOpd::newImme(imm-1)));
            inst->succ->succ->succ->insert(make_shared<Tac>(TacOpr::Sub, inst->opd1, opd, tmp));
            inst->remove();
        } else {
            inst->insert(make_shared<Tac>(TacOpr::AddLSR, tmp, opd, opd, TacOpd::newImme(32-k)));
            inst->succ->insert(make_shared<Tac>(TacOpr::BIC, tmp, tmp, TacOpd::newImme(imm-1)));
            inst->succ->succ->insert(make_shared<Tac>(TacOpr::Sub, inst->opd1, opd, tmp));
            inst->remove();
        }
        return true;
    } else {
        auto tmp = TacOpd::newReg();
        inst->insert(make_shared<Tac>(TacOpr::Div, tmp, opd, TacOpd::newImme(imm)));
        inst->succ->insert(make_shared<Tac>(TacOpr::Mul, tmp, tmp, TacOpd::newImme(imm)));
        inst->succ->succ->insert(make_shared<Tac>(TacOpr::Sub, inst->opd1, opd, tmp));
        inst->remove();
        return true;
    }
}

static bool opt_mod(TacPtr inst, int imm, TacOpd opd)
{
    return false;
}

bool strength_reduction(TacPtr inst)
{
    TacOpd opd;
    int imm;
    switch (inst->opr) {
        case TacOpr::Mul:
            if (inst->opd2.getType() == OpdType::Reg
                && inst->opd3.getType() == OpdType::Reg)
                return false;
            if (inst->opd2.getType() == OpdType::Imme) {
                imm = inst->opd2.getVal();
                opd = inst->opd3;
                assert(opd.getType() == OpdType::Reg);
            } else {
                imm = inst->opd3.getVal();
                opd = inst->opd2;
                assert(opd.getType() == OpdType::Reg);
            }
            return opt_mul(inst, opd, imm);
        case TacOpr::Div:
            if (inst->opd2.getType() == OpdType::Reg
                && inst->opd3.getType() == OpdType::Reg)
                return false;
            if (inst->opd2.getType() == OpdType::Imme) {
                return opt_div(inst, inst->opd2.getVal(), inst->opd3);
            } else {
                return opt_div(inst, inst->opd2, inst->opd3.getVal());
            }
            break;
        case TacOpr::Mod:
            if (inst->opd2.getType() == OpdType::Reg
                && inst->opd3.getType() == OpdType::Reg)
                return false;
            if (inst->opd2.getType() == OpdType::Imme) {
                return opt_mod(inst, inst->opd2.getVal(), inst->opd3);
            } else {
                return opt_mod(inst, inst->opd2, inst->opd3.getVal());
            }
            break;
        default:
            break;
    }
    return false;
}

bool strength_reduction(FlowGraph &flowgraph)
{
    auto blocks = flowgraph.getBlocks();
    bool changed = false;
    for (auto block : blocks) {
        for (int i = 0; i < 2; i++) {
            std::vector<TacPtr> insts;
            for (auto inst = block->insts.head->succ; 
                    inst != block->insts.tail; inst = inst->succ) {
                insts.push_back(inst);
            }
            for (auto inst : insts) {
                changed = strength_reduction(inst) || changed;
            }
        }
    }
    flowgraph.calcVars();
    flowgraph.calcDefUses();
    return changed;
}
