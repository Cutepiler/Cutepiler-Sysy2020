#include "flowgraph.h"

namespace Dominator {
    std::map<BBPtr, int> bb2id;
    std::vector<BBPtr> id2bb;
    std::vector<bool> visit;

    void dfs(BBPtr cur, BBPtr del) {
        if (cur == del || visit[bb2id[cur]]) return;
        visit[bb2id[cur]] = true;
        for (auto nxt : cur->succ) dfs(nxt, del);
    }

    void addDom(BBPtr cur, BBPtr dom) {
        if (visit[bb2id[cur]]) return;
        if (cur->doms.find(dom) != cur->doms.end()) return;
        cur->doms.insert(dom);
        for (auto nxt : cur->succ) addDom(nxt, dom);
    }
}

void FlowGraph::calcDominator() {
    using namespace Dominator;
    int totBlock = blocks.size();
    for (auto block : blocks) {
        block->doms.clear();
        block->idom = nullptr;
    }
    id2bb = getBlocks();
    for (int i = 0; i < totBlock; ++i) bb2id[id2bb[i]] = i;
    visit.resize(totBlock);
    for (auto d : blocks) {
        for (int i = 0; i < totBlock; ++i) visit[i] = false;
        dfs(startBlock, d);
        addDom(d, d);
    }

    for (auto block : blocks) {
        for (auto d : block->doms) {
            if (d != block && (block->idom == nullptr || BasicBlock::dominate(block->idom, d)))
                block->idom = d;
        }
        if (block->idom != nullptr) block->idom->child.insert(block);
    }
}
