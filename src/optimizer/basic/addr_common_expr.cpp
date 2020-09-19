#include "../../flowgraph/flowgraph.h"
#include "../../tac/tac.h"
#include "work_list_easy.h"
#include <map>
#include <iostream>
#include <vector>

using std::map;
using std::set;
using std::cerr;
using std::endl;
using std::make_shared;
using std::make_pair;
using std::pair;
using std::vector;

bool elim_addr_expr(BBPtr bb)
{
    cerr << "Hello" << endl;
    map<pair<TacOpd, TacOpd>, TacOpd> addr_map;
    bool changed = false;
    for (auto inst = bb->insts.head->succ; 
            inst != bb->insts.tail; inst = inst->succ) {
        if (inst->opr != TacOpr::Addr)
            continue;
        auto pos = make_pair(inst->opd2, inst->opd3);
        if (addr_map.count(pos)) {
            inst->opr = TacOpr::Mov;
            inst->opd2 = addr_map[pos];
            inst->opd3 = TacOpd();
            changed = true;
        } else {
            addr_map[pos] = inst->opd1;
            cerr << "Address " << inst->to_string() << endl;
        }
    }
    return changed;
}

bool elim_addr_expr(FlowGraph &flowgraph)
{
    return false;
    auto blocks = flowgraph.getBlocks();
    bool changed = false;
    for (auto bb : blocks)
        changed = elim_addr_expr(bb) || changed;
    return changed;
}
