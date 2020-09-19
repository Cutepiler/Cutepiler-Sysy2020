#include "flowgraph.h"
#include "../util/cuteprint.h"
#include <set>
#include <algorithm>
#include <vector>
#include <iostream>

using std::set;
using std::vector; 
using std::reverse; 
using std::endl;

void BasicBlock::computeUseDef()
{
    uses.clear();
    defs.clear(); 
    for (auto inst : insts) {
        set<TacOpd> use_inst = inst.getUses(); 
        set<TacOpd> def_inst = inst.getDefs(); 
        for (auto opd : use_inst)
            if (!defs.count(opd)) {
                uses.insert(opd);
            }
        for (auto opd : def_inst) {
            defs.insert(opd); 
        }
    }
}

set<TacOpd> BasicBlock::getUsesFast() const 
{
    return uses; 
}

set<TacOpd> BasicBlock::getDefsFast() const
{
    return defs; 
}

set<TacOpd> BasicBlock::getUses() 
{
    computeUseDef();
    return uses; 
}

set<TacOpd> BasicBlock::getDefs()
{
    computeUseDef();
    return defs; 
}

bool BasicBlock::computeLiveness(const std::set<TacOpd> &liveOutGiven)
{
    computeUseDef(); 
    liveOut = liveOutGiven; 
    int siz = liveIn.size(); 
    liveIn = liveOut;
    for (auto opd : defs) 
        liveIn.erase(opd);
    for (auto opd : uses)
        liveIn.insert(opd); 
    assert(liveIn.size() >= siz); 
    return liveIn.size() > siz; 
}

void BasicBlock::mergeLiveIn(set<TacOpd> &des)
{
    for (auto var : liveIn)
        des.insert(var); 
}

void BasicBlock::mergeLiveOut(set<TacOpd> &des)
{
    for (auto var : liveOut)
        des.insert(var); 
}

void BasicBlock::cleanLive()
{
    liveIn.clear();
    liveOut.clear();
    for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ)
        inst->cleanLive();
}

void FlowGraph::computeLiveness()
{
    vector<BBPtr> bblist; 
    set<BBPtr> ex_bb; 
    set<TacOpd> nothing; 
    dfs_blocks(startBlock, bblist, ex_bb);
    reverse(bblist.begin(), bblist.end());

    for (auto bb : bblist)
        bb->cleanLive();

    for (auto bb : bblist) {
        if (bb->succ.size() == 0) 
            bb->computeLiveness(nothing);
    }
    
    // block-level analysis
    bool changed; 
    do {
        changed = false; 
        for (auto bb : bblist) {
            set<TacOpd> liveOut = nothing; 
            for (auto nxt : bb->succ) {
                nxt->mergeLiveIn(liveOut);
            }
            changed = bb->computeLiveness(liveOut) || changed; 
        }
    } while (changed);
    
    // inst-level analysis
    for (auto bb : bblist) {
        set<TacOpd> last;
        bb->mergeLiveOut(last); 
        for (auto inst = bb->insts.tail->pred; 
                  inst != bb->insts.head; inst = inst->pred) {
            inst->computeLiveness(last); 
            last = inst->getLiveIn(); 
        }
    }
}

void FlowGraph::dfs_blocks(BBPtr bb, vector<BBPtr> &bblist, set<BBPtr> &ex_bb)
{
    bblist.push_back(bb);
    ex_bb.insert(bb);
    for (auto nxt : bb->succ) {
        if (ex_bb.count(nxt)) continue;
        dfs_blocks(nxt, bblist, ex_bb);
    }
}

const set<TacOpd> &BasicBlock::getLiveIn() const {
    return liveIn;
}

const set<TacOpd> &BasicBlock::getLiveOut() const {
    return liveOut;
}
