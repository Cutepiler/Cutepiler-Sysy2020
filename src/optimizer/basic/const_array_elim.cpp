#include "work_list_easy.h"
#include "../../flowgraph/flowgraph.h"
#include "../../env/env.h"
#include <set>

using std::set;

bool const_array_elim(FlowGraph &flowgraph, const TacProg &pg)
{
    set<int> dirty;
    auto blocks = flowgraph.getBlocks();
    for (auto func : pg.funcs) {
        if (func->id == flowgraph.func->id)
            continue;
        for (auto inst : func->insts) {
            switch (inst.opr) {
                case TacOpr::Addr:
                case TacOpr::Store:
                    // std::cerr << inst.to_string() << std::endl;
                    dirty.insert(inst.opd3.getVal());
                    break;
                default:
                    break;
            }
        }
    }
    for (auto block : blocks) {
        for (auto inst : block->insts) {
            switch (inst.opr) {
                case TacOpr::Addr:
                case TacOpr::Store:
                    dirty.insert(inst.opd3.getVal());
                    break;
                default:
                    break;
            }
        }
    }
    bool changed = false;
    
    for (auto block : blocks) {
        for (auto inst = block->insts.head->succ;
                inst != block->insts.tail; inst = inst->succ) {
            if (inst->opr == TacOpr::Load) {
                if (inst->opd3.getVal() == 0)
                    continue;
                if (dirty.count(inst->opd3.getVal()))
                    continue;
                if (inst->opd2.getType() != OpdType::Imme)
                    continue;
                // std::cerr << "H " << inst->to_string() << std::endl;
                int value;
                switch (pg.fbase(inst->opd3.getVal())) {
                    case SpaceType::BSS:
                        inst->opr = TacOpr::Mov;
                        inst->opd2 = TacOpd::newImme(0);
                        inst->opd3 = inst->opd4 = TacOpd();
                        changed = true;
                        break;
                    case SpaceType::Data:
                        assert(inst->opd2.getVal() % WORD_SIZE == 0);
                        value = pg.data.store.at(inst->opd2.getVal()/WORD_SIZE);
                        inst->opr = TacOpr::Mov;
                        inst->opd2 = TacOpd::newImme(value);
                        inst->opd3 = inst->opd4 = TacOpd();
                        changed = true;
                        break;
                    default:
                        continue;
                }
            }
        }
    }
    return changed;
}
