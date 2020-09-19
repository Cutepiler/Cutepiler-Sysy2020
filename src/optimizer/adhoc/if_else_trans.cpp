#include "adhoc.h"

bool match_simple_if_else(BBPtr block)
{
    if (block->succ.size() <= 1) return false;
    auto true_branch = block->getTrueBranch();
    auto false_branch = block->getFalseBranch();
    if (!false_branch->insts.empty()) return false;
    if (true_branch->succ.size() != 1 || false_branch->succ.size() != 1)
        return false;
    auto true_succ = *true_branch->succ.begin();
    auto false_succ = *false_branch->succ.begin();
    if (true_succ != false_succ) return false;
    return true;
}

bool simple_if_else_trans(BBPtr block)
{
    assert(match_simple_if_else(block));
    auto true_branch = block->getTrueBranch();
    auto false_branch = block->getFalseBranch();
    auto true_succ = *true_branch->succ.begin();
    block->setFalseBranch(block, true_succ);
    true_succ->pred.erase(block);
    block->swapBranches();
    return true;
}

bool simple_if_else_trans(FlowGraph &flowgraph)
{
    auto blocks = flowgraph.getBlocks();
    bool changed = false;
    for (auto block : blocks) {
        if (match_simple_if_else(block))
            changed = simple_if_else_trans(block) || changed; 
    }
    return changed;
}
