#include "adhoc.h"

bool strldr_optimization(TacProg &prog) {
    bool changed = false;
    std::set<TacPtr> toRemove;
    for (auto func : prog.funcs) {
        TacPtr laststr = nullptr;
        for (auto inst = func->insts.head->succ; inst != func->insts.tail;
             inst = inst->succ) {
            switch (inst->opr) {
                case TacOpr::Store:
                    laststr = inst;
                    break;
                case TacOpr::Load:
                    if (laststr != nullptr && laststr->opd2 == inst->opd2 &&
                        laststr->opd3 == inst->opd3 &&
                        func->reg.at(laststr->opd1) == func->reg.at(inst->opd1)) {
                        toRemove.insert(inst);
                    }
                    break;
                default:
                    if (laststr != nullptr) {
                        for (int i = 1; i <= 4; ++i) {
                            if (inst->isDef(i) && func->reg.at(inst->getOpd(i)) ==
                                                      func->reg.at(laststr->opd1)) {
                                laststr = nullptr;
                                break;
                            }
                        }
                    }
                    break;
            }
        }
    }
    return changed;
}