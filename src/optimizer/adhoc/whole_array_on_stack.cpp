#include "adhoc.h"
#include "../../env/env.h"
#include <set>
#include <vector>
#include <functional>
#include <map>

using std::set;
using std::vector;
using std::pair;
using std::make_pair;
using std::function;
using std::map;

struct NewSpaceMap {
    function<SpaceType(int)> func;
    map<int, SpaceType> new_maps;
    NewSpaceMap(function<SpaceType(int)> func) : func(func) {}
    SpaceType operator () (int space_id) {
        if (new_maps.count(space_id))
            return new_maps.at(space_id);
        return func(space_id);
    }
};

// mainly for bitset
void take_whole_array_out(TacProg &prog, FlowGraph &flowgraph)
{
    set<BBPtr> func_header;
    vector<BBPtr> func_vec;
    set<int> dirty_space;
    /* step 1: collect program header */ 
    auto block = flowgraph.getStartBlock();
    auto blocks = flowgraph.getBlocks();
    while (1) {
        func_header.insert(block);
        func_vec.push_back(block);
        if (block->insts.head->succ != block->insts.tail)
            break;
        if (block->succ.size() == 1) {
            auto nxt = *block->succ.begin();
            if (nxt == block) break;
            block = nxt;
        } else break;
    } 
    /* step 2 : find all dirty spaces */
    for (auto block : blocks) {
        if (func_header.count(block))
            continue;
        for (auto inst : block->insts) {
            switch (inst.opr) {
                case TacOpr::Store:
                case TacOpr::Addr:
                    assert(inst.opd3.getType() == OpdType::Imme);
                    dirty_space.insert(inst.opd3.getVal());
                    break;
                default: break;
            }
        }
    }
    for (auto block : func_header) {
        for (auto inst : block->insts) {
            switch (inst.opr) {
                case TacOpr::Addr:
                case TacOpr::Load:
                    assert(inst.opd3.getType() == OpdType::Imme);
                    dirty_space.insert(inst.opd3.getVal());
                    break;
                case TacOpr::Store:
                    if (inst.opd1.getType() != OpdType::Imme || inst.opd2.getType() != OpdType::Imme) {
                        assert(inst.opd3.getType() == OpdType::Imme);
                        dirty_space.insert(inst.opd3.getVal());
                    }
                    break;
                default:
                    break;
            }
        }
    }
    /* step 3 : collect clean spaces, [id, length] */
    set<pair<int,int>> clean_spaces;
    for (auto block : func_vec) {
        for (auto inst : block->insts)
            if (inst.opr == TacOpr::Store) {
                int space_id = inst.opd3.getVal();
                if (dirty_space.count(space_id)) continue;
                if (prog.fbase(space_id) != SpaceType::Stack) continue;
                if (!prog.space_length.count(space_id)) continue;
                clean_spaces.insert(make_pair(space_id, prog.space_length[space_id]));
            }
    }
    /* step 4 : allocate in data section */
    auto new_space = NewSpaceMap(prog.fbase);
    vector<TacPtr> delist;
    int length_sum = 0;
    for (auto [space, length] : clean_spaces) {
        new_space.new_maps[space] = SpaceType::Data;
        int base = prog.data.length;
        prog.data.length += length;
        length_sum += length;
        prog.data.store.resize(prog.data.length);
        // flowgraph.func->length -= length;
        for (auto block : func_vec) {
            for (auto inst = block->insts.head->succ; inst != block->insts.tail; inst = inst->succ)
                if (inst->opr == TacOpr::Store) {
                    int space_id = inst->opd3.getVal();
                    if (space_id != space) continue;
                    assert(inst->opd1.getType() == OpdType::Imme);
                    assert(inst->opd2.getType() == OpdType::Imme);
                    int val = inst->opd1.getVal();
                    int pos = inst->opd2.getVal() / WORD_SIZE - prog.space_base[space];
                    prog.data.store.at(base + pos) = val;
                    delist.push_back(inst);
                }
        }
        for (auto block : blocks) {
            if (func_header.count(block)) continue;
            for (auto inst = block->insts.head->succ; 
                    inst != block->insts.tail; inst = inst->succ)
                if (inst->opr == TacOpr::Load) {
                    int space_id = inst->opd3.getVal();
                    if (space_id != space) continue;
                    auto reg = TacOpd::newReg();
                    auto new_inst = std::make_shared<Tac>(
                        TacOpr::Add, reg, inst->opd2, TacOpd::newImme(base*WORD_SIZE - prog.space_base[space]*WORD_SIZE)
                    );
                    inst->opd2 = reg;
                    inst->pred->insert(new_inst);
                }
        }
    } 
    if (length_sum == flowgraph.func->length)
        flowgraph.func->length = 0;
    for (auto de : delist)
         de->remove();
    prog.fbase = new_space;
    flowgraph.fbase = new_space;
}
