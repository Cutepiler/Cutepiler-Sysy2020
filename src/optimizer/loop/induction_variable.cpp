#include "loop.h"
#include <map>
#include <vector>
#include <iostream>
#include <tuple>

using std::vector;
using std::map;
using std::make_shared;
using std::tuple;
using std::make_tuple;
using std::make_pair;
using std::set;

using std::cerr;
using std::endl;

bool Loop::is_invariant(const TacOpd &opd) 
{
    return !defs.has_def(opd);
}

bool operator == (const AffineVar &a, const AffineVar &b)
{
    return a.a == b.a && a.b == b.b;
}

TacOpd Loop::opd_add(const TacOpd &a, const TacOpd &b)
{
    if (a.getType() == OpdType::Imme && b.getType() == OpdType::Imme)
        return TacOpd::newImme(a.getVal() + b.getVal());
    else if (a.getType() == OpdType::Imme && a.getVal() == 0) 
        return b;
    else if (b.getType() == OpdType::Imme && b.getVal() == 0)
        return a;
    else {
        auto reg = TacOpd::newReg();
        add_pre(make_shared<Tac>(TacOpr::Add, reg, a, b));
        return reg;
    }
}

TacOpd Loop::opd_sub(const TacOpd &a, const TacOpd &b)
{
    if (a.getType() == OpdType::Imme && b.getType() == OpdType::Imme)
        return TacOpd::newImme(a.getVal() - b.getVal());
    else if (b.getType() == OpdType::Imme && b.getVal() == 0)
        return a;
    else {
        auto reg = TacOpd::newReg();
        add_pre(make_shared<Tac>(TacOpr::Sub, reg, a, b));
        return reg;
    }
}

TacOpd Loop::opd_neg(const TacOpd &a)
{
    if (a.getType() == OpdType::Imme)
        return TacOpd::newImme(-a.getVal());
    else {
        auto reg = TacOpd::newReg();
        add_pre(make_shared<Tac>(TacOpr::Neg, reg, a));
        return reg;
    }
}

TacOpd Loop::opd_mul(const TacOpd &a, const TacOpd &b)
{
    if (a.getType() == OpdType::Imme && b.getType() == OpdType::Imme)
        return TacOpd::newImme(a.getVal() * b.getVal());
    else if (a.getType() == OpdType::Imme && a.getVal() == 1) 
        return b;
    else if (b.getType() == OpdType::Imme && b.getVal() == 1)
        return a;
    else {
        auto reg = TacOpd::newReg();
        add_pre(make_shared<Tac>(TacOpr::Mul, reg, a, b));
        return reg;
    }
}


AffineVar Loop::affine_add(const AffineVar &var, const TacOpd &opd)
{
    return (AffineVar) {var.a, opd_add(var.b, opd)};
}

AffineVar Loop::affine_sub(const AffineVar &var, const TacOpd &opd)
{
    return (AffineVar) {var.a, opd_sub(var.b, opd)};
}

AffineVar Loop::affine_sub(const TacOpd &opd, const AffineVar &var)
{
    return (AffineVar) {opd_neg(var.a), opd_sub(opd, var.b)};
}

AffineVar Loop::affine_mul(const AffineVar &var, const TacOpd &opd)
{
    return (AffineVar) {opd_mul(opd, var.a), opd_mul(opd, var.b)}; 
}

void Loop::inductionVar(const set<int> &pure_funcs) 
{
    assert(node_inside.count(pre_header));
    node_inside.erase(pre_header);
    index_table.clear();
    computeDefs(pure_funcs);

    auto update_map = [this](map<TacOpd, AffineVar> &ind_map, const Tac &inst) 
    {
        if (ind_map.count(inst.opd1))
            return false;
        switch (inst.opr) {
            case TacOpr::Neg:
                if (ind_map.count(inst.opd2)) {
                    auto [a, b] = ind_map[inst.opd2];
                    ind_map[inst.opd1] = (AffineVar) {opd_neg(a), opd_neg(b)};
                    return true;
                }
                break;
            case TacOpr::Add:
                if (ind_map.count(inst.opd2) && ind_map.count(inst.opd3)) {
                    auto [a, b] = ind_map[inst.opd2];
                    auto [c, d] = ind_map[inst.opd3];
                    ind_map[inst.opd1] = (AffineVar) {opd_add(a, c), opd_add(b, d)};
                    return true; 
                } else if (ind_map.count(inst.opd2) && is_invariant(inst.opd3)) {
                    ind_map[inst.opd1] = affine_add(ind_map[inst.opd2], inst.opd3);
                    return true;
                } else if (ind_map.count(inst.opd3) && is_invariant(inst.opd2)) {
                    ind_map[inst.opd1] = affine_add(ind_map[inst.opd3], inst.opd2);
                    return true;    
                } 
            case TacOpr::Sub:
                if (ind_map.count(inst.opd2) && ind_map.count(inst.opd3)) {
                    auto [a, b] = ind_map[inst.opd2];
                    auto [c, d] = ind_map[inst.opd3];
                    ind_map[inst.opd1] = (AffineVar) {opd_sub(a, c), opd_sub(b, d)};
                    return true; 
                } else if (ind_map.count(inst.opd2) && is_invariant(inst.opd3)) {
                    ind_map[inst.opd1] = affine_sub(ind_map[inst.opd2], inst.opd3);
                    return true;
                } else if (ind_map.count(inst.opd3) && is_invariant(inst.opd2)) {
                    ind_map[inst.opd1] = affine_sub(inst.opd2, ind_map[inst.opd3]);
                    return true;    
                } 
            case TacOpr::Mul:
                if (ind_map.count(inst.opd2) && is_invariant(inst.opd3)) {
                    ind_map[inst.opd1] = affine_mul(ind_map[inst.opd2], inst.opd3);
                    return true;
                } else if (ind_map.count(inst.opd3) && is_invariant(inst.opd2)) {
                    ind_map[inst.opd1] = affine_mul(ind_map[inst.opd3], inst.opd2);
                    return true;    
                } 
                break;
            case TacOpr::Mov:
                assert(false);
            default:
                break;
        }
        return false;
    };

    vector<tuple<TacOpd, BBPtr, TacOpd>> phi_list;

    for (auto &[base, phi_src] : header->phi) {
        assert(phi_src.size() == header->pred.size());
        map<TacOpd, AffineVar> ind_map;
        bool changed;
        vector<BBPtr> bblist;
        for (auto &[src, opd] : phi_src) {
            if (!node_inside.count(src))
                continue;
            bblist.push_back(src);
        }
        if (bblist.empty())
            continue;
        ind_map[base] = (AffineVar) {TacOpd::newImme(1), TacOpd::newImme(0)};
        do {
            changed = false;
            for (auto block : node_inside) {
                if (block == header)
                    continue;
                for (auto inst : block->insts) {
                    changed = update_map(ind_map, inst) || changed;
                }
            }
        } while (changed);
        bool flag = true;
        for (auto bb : bblist) {
            if (!ind_map.count(phi_src[bb])) {
                flag = false;
                break;
            }
        }
        if (!flag) continue;
        auto res = ind_map[phi_src[bblist[0]]];
        for (auto bb : bblist) {
            if (!(ind_map[phi_src[bb]] == res)) {
                flag = false;
                continue;
            }
        }
        if (!flag) continue;
        if (res.a.getType() != OpdType::Imme || res.a.getVal() != 1)
            continue;
        // now, this variable must be induction variable 
        //  with value x <- x + res.b;
        auto inc = res.b;

        induction_set[base] = inc;

        /* strength reduction algorithm */

        vector<TacPtr> delist;
        vector<TacOpd> optlist;

        auto lowbit = [](int i) { return i & (-i); };
        auto is_power = [lowbit](int i) { return i == lowbit(i); };
        auto is_one = [is_power](const TacOpd &opd) {
            return opd.getType() == OpdType::Imme && opd.getVal() == 1;
        };

        auto reduction = [&optlist, &delist, &ind_map, &is_one]
                    (const TacOpd &base, TacPtr inst) {
            auto reg = TacOpd::newReg();
            switch (inst->opr) {
                case TacOpr::Neg:
                case TacOpr::Add:
                case TacOpr::Sub:
                case TacOpr::Mul:
                    if (ind_map.count(inst->opd1)) {
                        if (is_one(ind_map[inst->opd1].a)) {
                            inst->pred->insert(make_shared<Tac>(TacOpr::Mul, reg, base, ind_map[inst->opd1].a));
                            inst->opr = TacOpr::Add;
                            inst->opd2 = reg;
                            inst->opd3 = ind_map[inst->opd1].b;
                        } else {
                            delist.push_back(inst);
                            optlist.push_back(inst->opd1);
                        }
                    } 
                    break;
                default:
                    break;
            }
        };

        for (auto block : node_inside) {
            if (block == header)
                continue;
            for (auto inst = block->insts.head->succ;
                    inst != block->insts.tail; inst = inst->succ) {
                reduction(base, inst);
            }
        }

        /* now, replace the definition of induction variables into addition */

        for (auto de : delist) {
            de->remove();
        }

        for (auto var : optlist) {
            auto a = ind_map[var].a, b = ind_map[var].b;
            TacOpd step = TacOpd::newReg();
            add_pre(make_shared<Tac>(TacOpr::Mul, step, a, inc));
            // cerr << "Induction Var " << var.name() << " " << a.name() << " " << b.name() << " " << step.name() << endl;
            for (auto [pred, val] : phi_src) {
                if (node_inside.count(pred)) {
                    // inside the loop
                    assert(pred != pre_header);
                    TacOpd tmp = TacOpd::newReg();
                    pred->insts.push_back(make_shared<Tac>(TacOpr::Add, tmp, var, step));
                    phi_list.push_back(make_tuple(var, pred, tmp));
                } else {
                    // outside of the loop
                    assert(pred == pre_header);
                    TacOpd tmp = TacOpd::newReg(), tmp2 = TacOpd::newReg();
                    add_pre(make_shared<Tac>(TacOpr::Mul, tmp, val, a));
                    add_pre(make_shared<Tac>(TacOpr::Add, tmp2, tmp, b));
                    phi_list.push_back(make_tuple(var, pred, tmp2));
                }
            }
        }
    }
    
    for (auto [var, pred, val] : phi_list) {
        header->phi[var][pred] = val;
    }
    node_inside.insert(pre_header);
}
