#include "../util/setopr.h"
#include "flowgraph.h"
#include "pointer_analysis.h"

static std::vector<TacPtr> getLoadStore(BBPtr block) {
    std::vector<TacPtr> result;
    for (auto inst = block->insts.head->succ; inst != block->insts.tail;
         inst = inst->succ) {
        switch (inst->opr) {
            case TacOpr::Load:
            case TacOpr::Store:
                result.push_back(inst);
                break;
            default:
                break;
        }
    }
    return result;
}

static void kill(std::set<TacPtr> &live, TacPtr store, const PointerAnalyzer &pa) {
    std::set<TacPtr> toRemove;
    for (auto load : live) {
        if (!pa.independent(store, load)) toRemove.insert(load);
    }
    for (auto inst : toRemove) live.erase(inst);
}

static bool analyzeBlock(const std::vector<TacPtr> &insts, const std::set<TacPtr> &liveIn,
                  std::set<TacPtr> &liveOut, const PointerAnalyzer &pa) {
    int siz = liveOut.size();
    liveOut = liveIn;
    for (auto inst : insts) {
        switch (inst->opr) {
            case TacOpr::Load:
                liveOut.insert(inst);
                break;
            case TacOpr::Store:
                kill(liveOut, inst, pa);
                break;
            default:
                assert(false);
        }
    }
    return liveOut.size() != siz;
}

std::map<TacPtr, std::set<TacPtr>> FlowGraph::getLiveLoad(const TacProg &prog) {
    auto pa = PointerAnalyzer(prog, *this);

    std::map<TacPtr, std::set<TacPtr>> result;
    std::map<BBPtr, std::set<TacPtr>> liveIn, liveOut;
    std::map<BBPtr, std::vector<TacPtr>> lsInsts;

    std::vector<BBPtr> bblist;
    std::set<BBPtr> ex_bb;
    std::set<TacPtr> nothing;
    dfs_blocks(startBlock, bblist, ex_bb);

    for (auto bb : bblist) lsInsts[bb] = getLoadStore(bb);

    // block-level analysis
    bool changed;
    do {
        changed = false;
        for (auto bb : bblist) {
            liveIn[bb] = nothing;
            for (auto pre : bb->pred) {
                liveIn[bb] += liveOut[pre];
            }
            changed =
                analyzeBlock(lsInsts[bb], liveIn[bb], liveOut[bb], pa) || changed;
        }
    } while (changed);

    // inst-level analysis
    for (auto bb : bblist) {
        auto live = liveIn[bb];
        for (auto inst : lsInsts[bb]) {
            switch (inst->opr) {
                case TacOpr::Load:
                    result[inst] = live;
                    live.insert(inst);
                    break;
                case TacOpr::Store:
                    kill(live, inst, pa);
                    break;
                default:
                    assert(false);
            }
        }
    }

    return result;
}