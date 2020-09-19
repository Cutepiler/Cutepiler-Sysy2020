#include "adhoc.h"
#include "../../tac/tac.h"
#include <map>

using std::map;

void branch_merging(Insts &insts)
{
    map<TacOpd, TacOpd> branch_map;
    map<TacOpd, TacPtr> return_map;
    for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ) {
        if (inst->opr == TacOpr::Labl) {
            auto cur = inst;
            while (cur->opr == TacOpr::Labl)
                cur = cur->succ;
            if (cur->opr == TacOpr::Branch) {
                branch_map[inst->opd1] = cur->opd1;
            }
            else if (cur->opr == TacOpr::Ret)
                return_map[inst->opd1] = cur;
        }
    }
    for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ) {
        if (inst->opr == TacOpr::Labl) continue;
        for (int i = 1; i <= 4; i++) {
            auto &opd = inst->getOpd(i);
            if (branch_map.count(opd)) {
                assert(opd.getType() == OpdType::Label);
                opd = branch_map[opd];
            } 
        }
        if (inst->opr == TacOpr::Branch) {
            auto des = inst->opd1;
            if (return_map.count(des)) {
                auto ret = return_map[des];
                inst->opr = ret->opr;
                inst->opd1 = ret->opd1;
                inst->opd2 = inst->opd3 = inst->opd4 = TacOpd();
            }
        } 
    }
}
