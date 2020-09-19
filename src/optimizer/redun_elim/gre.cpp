#include "../basic/work_list_easy.h"
#include "redundancy_elim.h"

bool defIt(BBPtr block, const TacOpd &opd) {
    for (auto inst = block->insts.head->succ; inst != block->insts.tail;
         inst = inst->succ) {
        if (opd == getDes(inst)) return true;
    }
    return false;
}

TacOpd eliminate_single(TacPtr tac, int id, DAG &g, const std::set<int> &pure_funcs,
                        const std::map<TacPtr, std::set<TacPtr>> &liveLoad) {
    auto c = Computation(tac, pure_funcs);
    if (c.empty()) return TacOpd();

    std::map<BBPtr, Computation> comp;
    std::map<BBPtr, std::set<BBPtr>> from;
    std::map<BBPtr, TacOpd> toUse;

    comp[g.bbInOrder[id]] = c;

    for (int i = id; i >= 0; --i) {
        auto bb = g.bbInOrder[i];
        if (!comp.count(bb)) continue;
        auto c = comp[bb];
        if (i != id) {
            auto tac = findComp(g.comps[bb], c, liveLoad);
            if (tac != nullptr) {
                toUse[bb] = getDes(tac);
                continue;
            }
        }
        for (auto opd : c.opds) {
            if (defIt(bb, opd)) {
                return TacOpd();
            }
        }
        if (g.pred[bb].empty()) {
            return TacOpd();
        }
        for (auto p : g.pred[bb]) {
            auto nc = c.rename(bb->phi, p);
            if (comp.count(p) && !comp[p].same(nc)) {
                return TacOpd();
            }
            comp[p] = nc;
            from[p].insert(bb);
        }
    }

    for (int i = 0; i <= id; ++i) {
        auto bb = g.bbInOrder[i];
        if (!comp.count(bb)) continue;
        if (toUse.count(bb)) continue;
        auto newopd = TacOpd::newReg();
        toUse[bb] = newopd;
        for (auto p : bb->pred) {
            auto np = g.alias[p];
            assert(toUse.count(np));
            bb->phi[newopd][p] = toUse[np];
        }
    }
    assert(toUse.count(g.bbInOrder[id]));
    return toUse[g.bbInOrder[id]];
}

bool global_redundancy_elimintaion(DAG &g, FlowGraph &flowgraph,
                                   const std::set<int> &pure_funcs,
                                   const TacProg &prog) {
    bool changed = false;
    auto liveLoad = flowgraph.getLiveLoad(prog);
    for (int i = 0; i < g.bbInOrder.size(); ++i) {
        auto bb = g.bbInOrder[i];
        auto inst = bb->insts.head->succ;
        while (inst != bb->insts.tail) {
            auto newopd = eliminate_single(inst, i, g, pure_funcs, liveLoad);
            if (!newopd.empty()) {
                auto c = Computation(inst, pure_funcs);
                g.comps[bb].erase(c);
                convert2Move(g, inst, newopd);
                inst = inst->succ;
                copy_propagation(flowgraph);
                changed = true;
            } else {
                inst = inst->succ;
            }
        }
    }
    return changed;
}