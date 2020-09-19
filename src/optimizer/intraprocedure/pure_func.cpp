#include "intra_driver.h"
#include "../../env/env.h"

using std::shared_ptr;
using std::function;
using std::set;
using std::vector;
using std::map;
using std::pair;
using std::make_shared;
using std::make_pair;

bool is_dirty(shared_ptr<TacFunc> func, function<SpaceType(int)> &fbase, set<int> &dirty_func)
{
    for (auto inst : func->insts) {
        switch (inst.opr) {
            case TacOpr::Store:
                assert(inst.opd3.getType() == OpdType::Imme);
                if (fbase(inst.opd3.getVal()) != SpaceType::Stack)
                    return true;
                break;
            case TacOpr::Call:
                assert(inst.opd1.getType() == OpdType::Imme);
                if (dirty_func.count(inst.opd1.getVal()))
                    return true;
                break;
            default:
                break;
        }
    }
    return false;
}

void compute_pure_func(TacProg &pg)
{
    for (auto func : pg.funcs)
        func->is_pure = true;
    bool changed;
    do {
        changed = false;
        set<int> dirty_funcs = {GETINT, GETARRAY, PUTINT, PUTARRAY, 
                                STARTTIME, STOPTIME, MEMSET,
                                PUTF, GETCH, PUTCH};
        for (auto func : pg.funcs)
            if (!func->is_pure)
                dirty_funcs.insert(func->id);
        for (auto func : pg.funcs)
            if (is_dirty(func, pg.fbase, dirty_funcs) && func->is_pure) {
                func->is_pure = false;
                changed = true;
            }
    } while (changed);
}

struct FCall {
    int id;
    vector<TacOpd> params;
    friend bool operator == (const FCall &a, const FCall &b)
    {
        if (a.id != b.id)
            return false;
        if (a.params.size() != b.params.size())
            return false;
        int siz = a.params.size();
        for (int i = 0; i < siz; i++)
            if (a.params[i] != b.params[i])
                return false;
        return true;
    }
    friend bool operator < (const FCall &a, const FCall &b)
    {
        if (a.id != b.id)
            return a.id < b.id;
        if (a.params.size() != b.params.size())
            return a.params.size() < b.params.size();
        return a.params < b.params;
    }
};

bool pure_func_merging(BBPtr bb, const set<int> &pure_funcs)
{
    map<FCall, TacOpd> pure_map;
    vector<pair<TacPtr, TacPtr>> replace;
    for (auto inst = bb->insts.head->succ; 
            inst != bb->insts.tail; inst = inst->succ) {
        if (inst->opr == TacOpr::Call && pure_funcs.count(inst->opd1.getVal())) {
            FCall func; 
            func.id = inst->opd1.getVal();
            auto pred = inst->pred;
            while (pred != bb->insts.head && pred->opr == TacOpr::Param) {
                func.params.push_back(pred->opd1);
                pred = pred->pred;
            }
            if (!pure_map.count(func))
                pure_map[func] = inst->opd2;
            else {
                auto move_inst = make_shared<Tac>(TacOpr::Mov, inst->opd2, pure_map[func]);
                replace.push_back(make_pair(inst, move_inst));
            }
        }
    }
    if (replace.empty())
        return false;
    for (auto [inst, new_inst] : replace) {
        inst->insert(new_inst);
        auto pred = inst->pred;
        inst->remove();
        while (pred->opr == TacOpr::Param) {
            auto tmp = pred;
            pred = pred->pred;
            tmp->remove();
        }
    }
    return true;
}

bool pure_func_merging(FlowGraph &flowgraph, const set<int> &pure_funcs)
{
    bool changed = false;
    auto blocks = flowgraph.getBlocks();
    for (auto bb : blocks)
        changed = pure_func_merging(bb, pure_funcs) || changed;
    return changed;
}
