#include <map>
#include <set>
#include <stack>
#include <vector>

#include "flowgraph.h"

namespace SSA {
    std::map<BBPtr, int> bb2id;
    std::vector<BBPtr> id2bb;

    std::vector<std::set<TacOpd>> varsDef;
    std::vector<std::set<TacOpd>> varsPhi;
    std::vector<std::set<int>> DF;

    std::map<int, int> count;
    std::map<int, std::stack<int>> stack;

    void calcVarsDefed(BBPtr block) {
        int id = bb2id[block];
        for (auto inst : block->insts) {
            for (int i = 1; i <= 3; ++i) {
                if (inst.isDef(i)) varsDef[id].insert(inst.getOpd(i));
            }
        }
    }

    void calcDF(BBPtr block) {
        int id = bb2id[block];
        for (auto pre : block->pred) {
            for (auto dom : pre->doms) {
                if (BasicBlock::strictDominate(dom, block)) continue;
                DF[bb2id[dom]].insert(id);
            }
        }
    }

    void calcVarsPhi() {
        std::map<TacOpd, std::set<int>> orig;
        for (int id = 0; id < id2bb.size(); ++id) {
            for (auto var : varsDef[id]) {
                orig[var].insert(id);
            }
        }
        for (auto [var, W] : orig) {
            while (!W.empty()) {
                auto n = *W.begin();
                W.erase(W.begin());
                for (auto Y : DF[n]) {
                    if (varsPhi[Y].find(var) == varsPhi[Y].end()) {
                        varsPhi[Y].insert(var);
                        if (orig[var].find(Y) == orig[var].end()) {
                            W.insert(Y);
                        }
                    }
                }
            }
        }
    }

    int getTop(int regID) {
        if (stack[regID].empty()) stack[regID].push(0);
        return stack[regID].top();
    }

    void renameUse(TacOpd &opd) {
        if (opd.getType() != OpdType::Reg) return;
        opd.addSubscript(getTop(opd.getId()));
    }

    void renameDef(TacOpd &opd) {
        assert(opd.getType() == OpdType::Reg);
        int i = ++count[opd.getId()];
        stack[opd.getId()].push(i);
        opd.addSubscript(i);
    }

    void rename(BBPtr bb) {
        int id = bb2id[bb];
        std::set<TacOpd> afterRename;
        for (auto var : varsPhi[id]) {
            auto v = var;
            renameDef(v);
            afterRename.insert(v);
        }
        varsPhi[id] = afterRename;
        for (auto inst = bb->insts.head->succ; inst != bb->insts.tail; inst = inst->succ) {
            for (int i = 1; i <= 3; ++i) {
                if (inst->isUse(i)) renameUse(inst->getOpd(i));
            }
            for (int i = 1; i <= 3; ++i) {
                if (inst->isDef(i)) renameDef(inst->getOpd(i));
            }
        }
        for (auto s : bb->succ) {
            int sid = bb2id[s];
            for (auto var : varsPhi[sid]) {
                auto v = var.getOrigin();
                renameUse(v);
                s->phi[var.getOrigin()][bb] = v;
            }
        }
        for (auto ch : bb->child) rename(ch);
        for (auto var : varsPhi[id]) {
            stack[var.getOrigin().getId()].pop();
        }
        for (auto inst : bb->insts) {
            for (int i = 1; i <= 3; ++i) {
                if (inst.isDef(i)) {
                    stack[inst.getOpd(i).getOrigin().getId()].pop();
                }
            }
        }
    }

}

void FlowGraph::toSSA() {
    using namespace SSA;
    int totBlock = blocks.size();
    id2bb = getBlocks();
    for (int i = 0; i < totBlock; ++i) bb2id[id2bb[i]] = i;
    varsDef.clear();
    varsPhi.clear();
    DF.clear();
    varsDef.resize(totBlock);
    varsPhi.resize(totBlock);
    DF.resize(totBlock);

    for (auto block : blocks) calcVarsDefed(block);
    for (auto block : blocks) calcDF(block);
    calcVarsPhi();
    rename(startBlock);
    for (auto block : blocks) {
        int id = bb2id[block];
        std::map<TacOpd, std::map<BBPtr, TacOpd>> newPhi;
        for (auto v : varsPhi[id]) {
            auto var = v.getOrigin();
            newPhi[v] = block->phi[var];
        }
        block->phi = newPhi;
    }
}