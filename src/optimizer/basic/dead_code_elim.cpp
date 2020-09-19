#include "dead_code_elim.h"
#include <queue>

bool dead_code_elimination(FlowGraph &g, const std::set<int> pure_funcs) {
    std::queue<TacOpd> varsList;
    std::set<TacOpd> varsUsed;
    std::vector<TacPtr> toRemove;
    for (auto bb : g.getBlocks()) {
        for (auto inst : bb->insts) {
            switch (inst.opr) {
                case TacOpr::Ret:
                case TacOpr::Param:
                    if (inst.isUse(1)) varsList.push(inst.opd1);
                    break;
                case TacOpr::Store:
                    if (inst.isUse(1)) varsList.push(inst.opd1);
                    if (inst.isUse(2)) varsList.push(inst.opd2);
                    break;
                case TacOpr::Beqz:
                case TacOpr::Bnez:
                    if (inst.isUse(1)) varsList.push(inst.opd1);
                    break;
                default:
                    break;
            }
        }
    }
    auto addVar = [&](const TacOpd &opd) -> void {
        if (varsUsed.find(opd) == varsUsed.end()) varsList.push(opd);
    };
    while (!varsList.empty()) {
        auto var = varsList.front();
        varsList.pop();
        if (varsUsed.find(var) != varsUsed.end()) continue;
        varsUsed.insert(var);
        if (g.def.find(var) == g.def.end()) continue;
        auto pos = g.def[var];
        if (pos->isPhi()) {
            for (auto p : pos->getBB()->phi[var]) addVar(p.second);
        } else {
            if (pos->getTac()->isUse(1)) addVar(pos->getTac()->opd1);
            if (pos->getTac()->isUse(2)) addVar(pos->getTac()->opd2);
            if (pos->getTac()->isUse(3)) addVar(pos->getTac()->opd3);
        }
    }
    for (auto bb : g.getBlocks()) {
        std::set<TacOpd> toRemove;
        for (auto [var, val] : bb->phi) {
            if (varsUsed.find(var) == varsUsed.end()) toRemove.insert(var);
        }
        for (auto var : toRemove) bb->phi.erase(var);
    }
    for (auto bb : g.getBlocks()) {
        for (auto inst = bb->insts.head->succ; inst != bb->insts.tail;
             inst = inst->succ) {
            if (inst->opr == TacOpr::Call) {
                if (!pure_funcs.count(inst->opd1.getVal())) continue;
                if (inst->opd2.empty() || !varsUsed.count(inst->opd2)) {
                    toRemove.push_back(inst);
                }
            } else if (inst->isDef(1) && varsUsed.find(inst->opd1) == varsUsed.end())
                toRemove.push_back(inst);
        }
    }
    for (auto inst : toRemove) inst->remove();
    g.calcVars();
    g.calcDefUses();
    return !toRemove.empty();
}