#include "../../flowgraph/flowgraph.h"
#include "merge_basic_blocks.h"
#include <cassert>

void merge_basic_blocks(FlowGraph &flowgraph)
{
    flowgraph.cleanBB();
    bool changed = false;
    auto blocks = flowgraph.getBlocks();
    do {
        changed = false;
        for (auto bb : blocks) {
            if (bb->succ.size() == 1) {
                auto nxt = *bb->succ.begin();
                if (nxt->pred.size() != 1 || nxt->succ.size() > 1)
                    continue;
                assert(nxt->phi.empty());
                for (auto inst = bb->insts.tail->pred; inst != bb->insts.head; ) {
                    auto tmp = inst->pred;
                    inst->remove();
                    nxt->insts.head->insert(inst);
                    inst = tmp;
                    changed = true;
                }
            }
        }
    } while (changed);
}
