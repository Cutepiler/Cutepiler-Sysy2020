#include "../../flowgraph/flowgraph.h"
#include "../../tac/tac.h"
#include "work_list_easy.h"
#include <map>
#include <iostream>
#include <vector>

using std::map;
using std::cerr;
using std::endl;
using std::make_shared;
using std::make_pair;
using std::pair;
using std::vector;
using std::set;

bool temp_addr_elim(FlowGraph &flowgraph)
{
    bool changed = false;
    auto blocks = flowgraph.getBlocks();
    map<TacOpd, TacPtr> addr_map;
    map<TacOpd, pair<TacOpd, TacOpd>> family;  
    /* find variables from Addr */
    for (auto bb : blocks) {
        for (auto inst = bb->insts.head->succ;
                inst != bb->insts.tail; inst = inst->succ) {
            if (inst->opr == TacOpr::Addr) {
                auto new_reg = TacOpd::newReg();
                addr_map[new_reg] = inst;
                family[inst->opd1] = make_pair(new_reg, inst->opd2);
                family[new_reg] = make_pair(new_reg, inst->opd2);
            }
        }
    }
    /* collect variables v := v' + d for v' in addr_map */
    bool here_changed;
    vector<pair<TacPtr, TacPtr>> ins_list;
    do {
        here_changed = false;
        for (auto bb : blocks) {
            for (auto inst = bb->insts.head->succ;
                inst != bb->insts.tail; inst = inst->succ) {
                if (family.count(inst->opd1))
                    continue;
                bool res2 = family.count(inst->opd2);
                bool res3 = family.count(inst->opd3);
                TacOpd new_reg = TacOpd::newReg();
                switch (inst->opr) {
                    case TacOpr::Add:
                        if (res2 && !res3) {
                            ins_list.push_back(make_pair(inst, 
                                make_shared<Tac>(TacOpr::Add, 
                                                new_reg, 
                                                family[inst->opd2].second,
                                                inst->opd3)));
                            family[inst->opd1] = make_pair(family[inst->opd2].first, new_reg);
                            here_changed = true;
                        } 
                        if (res3 && !res2) {
                            ins_list.push_back(make_pair(inst, 
                                make_shared<Tac>(TacOpr::Add, 
                                                new_reg, 
                                                family[inst->opd3].second,
                                                inst->opd2)));
                            family[inst->opd1] = make_pair(family[inst->opd3].first, new_reg);
                            here_changed = true;
                        }
                        break;
                    case TacOpr::Sub:
                        if (res2 && !res3) {
                            ins_list.push_back(make_pair(inst, 
                                make_shared<Tac>(TacOpr::Sub, 
                                                new_reg, 
                                                family[inst->opd2].second,
                                                inst->opd3)));
                            family[inst->opd1] = make_pair(family[inst->opd2].first, new_reg);
                            here_changed = true;
                        }
                        break;
                    default: break;
                }
            }
        }
    } while (here_changed);
    /* reduction */
    std::set<TacOpd> opt_set;
    for (auto bb : blocks) {
        for (auto inst = bb->insts.head->succ;
                inst != bb->insts.tail; inst = inst->succ) {
            if (inst->opr != TacOpr::Load && inst->opr != TacOpr::Store)
                continue;
            if (inst->opd3.getType() != OpdType::Imme || inst->opd3.getVal() != 0)
                continue;
            if (!family.count(inst->opd2))
                continue;
            auto def = addr_map[family[inst->opd2].first];
            inst->opd2 = family[inst->opd2].second;
            inst->opd3 = def->opd3;
            changed = true;
        }
    }
    if (changed) {
        for (auto [inst, add] : ins_list) {
            inst->insert(add);
        }
    }
    flowgraph.calcVars();
    flowgraph.calcDefUses();
    return changed;
}

bool load_store_elim_general(BBPtr bb, std::function<SpaceType(int)> fbase)
{
    map<int, map<TacOpd, TacOpd>> stack_info, other_info;
    bool changed = false;
    for (auto inst = bb->insts.head->succ; 
            inst != bb->insts.tail; inst = inst->succ) {
        int space;
        map<int, map<TacOpd, TacOpd>> *store_info;
        switch (inst->opr) {
            case TacOpr::Load:
                assert(inst->opd3.getType() == OpdType::Imme);
                space = inst->opd3.getVal();
                store_info = fbase(space) == SpaceType::Stack ? &stack_info : &other_info;
                if (store_info->count(space) && store_info->at(space).count(inst->opd2)) {
                    inst->opr = TacOpr::Mov;
                    inst->opd2 = store_info->at(space)[inst->opd2];
                    inst->opd3 = TacOpd();
                    changed = true;
                }
                break;
            case TacOpr::Call:
                stack_info.clear();
                other_info.clear();
                break;
            case TacOpr::Store:
                assert(inst->opd3.getType() == OpdType::Imme);
                space = inst->opd3.getVal();
                if (space == 0) 
                    other_info.clear();
                if (fbase(space) != SpaceType::Stack && other_info.count(0))
                    other_info[0].clear();
                store_info = fbase(space) == SpaceType::Stack ? &stack_info : &other_info;
                if (store_info->count(space) && inst->opd2.getType() == OpdType::Reg)
                    store_info->at(space).clear();
                store_info->operator[](space)[inst->opd2] = inst->opd1;
                break;
            default: break;
        }
    }
    return changed;
}

bool store_store_elim_general(BBPtr bb, std::function<SpaceType(int)> fbase)
{
    // TODO: a better algorithm may be needed
    map<int, set<TacOpd>> store_info;
    bool changed = false;
    vector<TacPtr> delete_list;
    for (auto inst = bb->insts.tail->pred; inst != bb->insts.head; inst = inst->pred) {
        int space;
        switch (inst->opr) {
            case TacOpr::Call:
            case TacOpr::Load:
            case TacOpr::Ret:
                store_info.clear();
                break;
            case TacOpr::Store:
                space = inst->opd3.getVal();
                if (store_info[space].count(inst->opd2))
                    delete_list.push_back(inst);
                store_info[space].insert(inst->opd2);
                break;
            default:
                break;
        }
    }
    for (auto de : delete_list) {
        de->remove();
        changed = true;
    }
    return changed;
} 

bool load_store_elim_general(FlowGraph &flowgraph)
{
    bool changed = false;
    auto blocks = flowgraph.getBlocks();
    for (auto bb : blocks) {
        changed = load_store_elim_general(bb, flowgraph.fbase) || changed;
        changed = store_store_elim_general(bb, flowgraph.fbase) || changed;
    }
    return changed;
}
