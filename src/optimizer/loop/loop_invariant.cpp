#include "loop.h"
#include "../../env/env.h"
#include <set> 
#include <iostream>
#include <vector>
#include <cassert>

using std::cerr;
using std::endl;
using std::set;
using std::vector;

void DefInfo::insert(const TacOpd &opd)
{ defs.insert(opd); }
void DefInfo::erase(const TacOpd &opd)
{ defs.erase(opd); }
void DefInfo::set_all(bool new_val) 
{ all_dirty = new_val; }
void DefInfo::set_dirty(int space_id)
{ dirty_space.insert(space_id); }
bool DefInfo::get_all() const
{ return all_dirty; }
void DefInfo::set_clean(int space_id) 
{ dirty_space.erase(space_id); }
bool DefInfo::is_dirty(int space_id)
{ return all_dirty || dirty_space.count(space_id); }
bool DefInfo::has_def(const TacOpd &opd)
{ return defs.count(opd); }
void DefInfo::clear()
{
    all_dirty = false;
    dirty_space.clear();
    defs.clear();
}
void DefInfo::merge(const DefInfo &from)
{
    all_dirty = all_dirty || from.all_dirty; 
    for (auto space : from.dirty_space)
        dirty_space.insert(space);
    for (auto opd : from.defs)
        defs.insert(opd);
}
const set<TacOpd> & DefInfo::get_defs() const
{ return defs; }

const DefInfo& Loop::getDefs() const
{ return defs; }

BBPtr Loop::getPreHeader() const 
{ return pre_header; }

#define NAIVE_SPACE 1

void Loop::computeDefs(const std::set<int> &pure_funcs) 
{
    defs.clear();
    store_insts.clear();
    for (auto block : node_inside) {
        assert (block != pre_header);
        for (auto inst : block->insts) {
            for (int i = 1; i <= 4; i++)
                if (inst.isDef(i))
                    defs.insert(inst.getOpd(i)); 
        }
        for (auto &[var, phi_src] : block->phi) {
            defs.insert(var);
        }

        // original 

        for (const auto &inst : block->insts) {
            switch (inst.opr) {
                case TacOpr::Store:
                    // std::cerr << "Dirty: " << inst.opd3.getVal() << std::endl;
                    if (inst.opd3.getVal() == 0) {
                        // TODO: Open this when closing PA
                        // defs.set_all(true);
                    } else {
                        defs.set_dirty(inst.opd3.getVal());
                        defs.set_dirty(0);
                    }
                    break;
                case TacOpr::Call:
                    if (!pure_funcs.count(inst.opd1.getVal()))
                        defs.set_all(true);
                    break;
                default:
                    break;
            }
        }

        for (auto inst = block->insts.head->succ; 
                inst != block->insts.tail; inst = inst->succ) {
            if (inst->opr == TacOpr::Store) {
                store_insts.push_back(inst);
            }
        }
    }
}

bool Loop::is_invariant(TacPtr tac)
{
    // cerr << "Asking " << tac->to_string() << endl;
    switch (tac->opr) {
        case TacOpr::Not:
        case TacOpr::Neg:
        case TacOpr::Mov:
            return !defs.has_def(tac->opd2);
        case TacOpr::Add:
        case TacOpr::Sub:
        case TacOpr::Mul:
        case TacOpr::Div:
        case TacOpr::Mod:
            return !defs.has_def(tac->opd2) && !defs.has_def(tac->opd3);
        case TacOpr::Gt:
        case TacOpr::Lt:
        case TacOpr::Ge:
        case TacOpr::Le:
        case TacOpr::Eq:
        case TacOpr::Ne:
        case TacOpr::And:
        case TacOpr::Or:
            return false;
        case TacOpr::Load:
            // std::cerr << tac->to_string() << " " << !defs.is_dirty(tac->opd3.getVal()) << " " << !defs.has_def(tac->opd2) << std::endl;
            // return !defs.is_dirty(tac->opd3.getVal()) && !defs.has_def(tac->opd2);
            // TODO: use new pointer analyzer
            // std::cerr << "Here " << tac->to_string() << std::endl;
            if (defs.get_all()) return false;
            return !defs.has_def(tac->opd2) && ptz->independent(store_insts, tac);
        case TacOpr::Addr:
            if (RUN_TEST) {
                assert(space_type(tac->opd3.getVal()) != SpaceType::Stack);
                assert(space_type(tac->opd3.getVal()) != SpaceType::Abs);
                return false;
            } 
            assert(tac->opd3.getType() == OpdType::Imme);
            return !defs.has_def(tac->opd2);
        default:
            return false;
    }
}

void Loop::remove_update_defs(TacPtr tac)
{
    auto def = tac->getDefs();
    for (auto opd : def) {
        // cerr << "Del Def " << opd.name() << endl; 
        defs.erase(opd);
    }
}

void Loop::invariantOpt(FlowGraph &flowgraph, const set<int> &pure_funcs, const TacProg &prog) 
{
    node_inside.erase(pre_header);
    bool changed;
    assert(pre_header != nullptr);
    ptz = std::make_shared<PointerAnalyzer>(prog, flowgraph);
    do {
        computeDefs(pure_funcs);
        changed = false;
        vector<TacPtr> delist;
        for (auto block : node_inside) {
            for (auto inst = block->insts.head->succ; 
                    inst != block->insts.tail; inst = inst->succ) 
                if (is_invariant(inst)) {
                    // std::cerr << "Is invariant: " << inst->to_string() << std::endl;
                    if (inst->succ != block->insts.tail) {
                        bool flag = true;
                        switch (inst->succ->opr) {
                            case TacOpr::Bnez:
                            case TacOpr::Beqz:
                                flag = false;
                                break;
                            default:
                                break;
                        }
                        if (flag == false)
                            continue;
                    }
                    delist.push_back(inst);
                    changed = true;
                }
        }
        for (auto tac : delist) {
            tac->remove();
            add_pre(tac);
        }
    } while (changed);
    node_inside.insert(pre_header);
}
