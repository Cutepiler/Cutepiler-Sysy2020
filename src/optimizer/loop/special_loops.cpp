#include "loop.h"
#include "../../flowgraph/pointer_analysis.h"
#include "../../flowgraph/work_iteration.h"
#include "../basic/work_list_easy.h"
#include <map>
#include <vector>
#include <set>

using std::map;
using std::vector;
using std::set;
using std::make_shared;

void Loop::accLoop(FlowGraph &flowgraph, const std::set<int> &pure_funcs, const TacProg &prog)
{
    if (!children.empty()) {
        for (auto chl : children)
            chl->accLoop(flowgraph, pure_funcs, prog);
        return;
    }
    node_inside.erase(pre_header);
    ptz = make_shared<PointerAnalyzer>(prog, flowgraph);
    computeDefs(pure_funcs);
    map<TacOpd, vector<TacPtr>> pt_map;
    for (auto block : node_inside) {
        for (auto inst = block->insts.head->succ;
                inst != block->insts.tail; inst = inst->succ) {
            switch (inst->opr) {
                case TacOpr::Load:
                    if (is_pointer(inst->opd2))
                        pt_map[inst->opd2].push_back(inst);
                    break;
                case TacOpr::Store:
                    if (is_pointer(inst->opd2)) {
                        pt_map[inst->opd2].push_back(inst);
                    }
                    break;
                case TacOpr::Call:
                    if (!pure_funcs.count(inst->opd1.getVal())) {
                        node_inside.insert(pre_header);
                        return;
                    }
                default: break;
            }
        }
    }
    bool dirty = false;
    for (auto [opd, use_pos] : pt_map) {
        if (defs.has_def(opd)) continue;
        assert(use_pos.size() != 0);
        for (auto inst : use_pos)
            assert(inst->opd3.getVal() == 0);
        vector<TacPtr> stores;
        for (auto block : node_inside)
            for (auto inst = block->insts.head->succ; 
                    inst != block->insts.tail; inst = inst->succ) {
                if (inst->opr != TacOpr::Store)
                    continue;
                if (inst->opd2 == opd) 
                    continue;
                stores.push_back(inst);
            }
        if (!ptz->independent(stores, 
                make_shared<Tac>(TacOpr::Load, TacOpd::newReg(), opd, TacOpd::newImme(0)))) continue;
        // std::cerr << opd << std::endl;
        auto reg = TacOpd::newReg();
        dirty = true;
        add_pre(make_shared<Tac>(TacOpr::Load, reg, opd, TacOpd::newImme(0)));
        for (auto inst : use_pos) {
            switch (inst->opr) {
                case TacOpr::Load:
                    inst->opr = TacOpr::Mov;
                    inst->opd3 = TacOpd();
                    inst->opd2 = reg;
                    break;
                case TacOpr::Store:
                    inst->opr = TacOpr::Mov;
                    inst->opd2 = inst->opd1;
                    inst->opd1 = reg;
                    inst->opd3 = TacOpd();
                    break;
                default: assert(false);
            }
        }
        for (auto block : node_inside)
            for (auto nxt : block->succ)
                if (!node_inside.count(nxt)) {
                    // std::cerr << "Outside: " << nxt->getId() << std::endl;
                    // exit point
                    nxt->insts.push_front(make_shared<Tac>(TacOpr::Store, reg, opd, TacOpd::newImme(0)));
                }
    }
    node_inside.insert(pre_header);
}
