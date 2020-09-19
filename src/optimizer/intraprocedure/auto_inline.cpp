#include "../../flowgraph/flowgraph.h"
#include "../../tac/tac.h"
#include <vector>
#include <cassert>
#include <iostream>

using std::cerr;
using std::map;
using std::endl;
using std::vector;
using std::shared_ptr;
using std::make_shared;
using FuncPtr = shared_ptr<TacFunc>;

bool should_be_inlined(FuncPtr func, const std::set<int> pure_funcs)
{
    if (func->length > 0) {
        return false;
    }
    if (pure_funcs.count(func->id)) return false;
    int cnt = 0;
    int cnt_branch = 0;
    for (auto &inst : func->insts) {
        cnt++;
        if (inst.opr == TacOpr::Call) {
            return false;
        }
        if (inst.opr == TacOpr::Beqz || inst.opr == TacOpr::Bnez) {
            cnt_branch++;
        }
    }
    if (!func->hasLoop) return true;
    if (cnt_branch > 1) return false;
    return true;
}

void do_inline(FuncPtr caller, FuncPtr callee)
{
    vector<TacPtr> delist;
    for (auto inst = caller->insts.head->succ;
            inst != caller->insts.tail; inst = inst->succ) {
        if (inst->opr == TacOpr::Call && inst->opd1.getVal() == callee->id) {
            delist.push_back(inst);
        }
    }
    for (auto call_inst : delist) {
        auto param = call_inst->pred;
        auto cur = callee->paramId.begin();
        while (param != caller->insts.head && param->opr == TacOpr::Param) {
            assert(cur != callee->paramId.end());
            param->opr = TacOpr::Mov;
            param->opd2 = param->opd1;
            param->opd1 = TacOpd(*cur, OpdType::Reg);
            param = param->pred;
            cur++;
        }
        assert(cur == callee->paramId.end());
        auto label = TacOpd::newLabel();
        map<TacOpd, TacOpd> label_map;
        for (auto inst : callee->insts) {
            if (inst.opr == TacOpr::Labl)
                label_map[inst.opd1] = TacOpd::newLabel();
        }
        for (auto inst : callee->insts) {
            assert(inst.opr != TacOpr::Call);
            if (inst.opr != TacOpr::Ret) {
                for (int i = 1; i <= 4; i++) {
                    auto &opd = inst.getOpd(i);
                    if (opd.getType() == OpdType::Label)
                        opd = label_map[opd];
                }
                call_inst->pred->insert(inst);
            } else {
                assert(inst.opr == TacOpr::Ret);
                if (inst.opd1.getType() == OpdType::Null) {
                    call_inst->pred->insert(make_shared<Tac>(TacOpr::Branch, label));
                } else {
                    assert(call_inst->opd2.getType() == OpdType::Reg);
                    call_inst->pred->insert(make_shared<Tac>(TacOpr::Mov, call_inst->opd2, inst.opd1));
                    call_inst->pred->insert(make_shared<Tac>(TacOpr::Branch, label));
                }
            }
        }
        call_inst->pred->insert(make_shared<Tac>(TacOpr::Labl, label));
        call_inst->remove();
    }
}

bool auto_inline(TacProg &prog, const std::set<int> &pure_funcs)
{
    bool changed = false;
    vector<FuncPtr> func_inline;
    for (auto func = prog.funcs.begin(); func != prog.funcs.end(); func++) {
        if (should_be_inlined(*func, pure_funcs) && (*func)->id != 0) {
            func_inline.push_back(*func);
            changed = true;
        }
    }
    for (auto func : func_inline) {
        for (auto caller : prog.funcs) {
            do_inline(caller, func);
        }
    }
    for (auto func : func_inline)
        prog.funcs.remove(func);
    return changed;
}
