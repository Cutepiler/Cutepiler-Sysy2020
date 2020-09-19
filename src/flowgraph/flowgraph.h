#pragma once

#include <functional>
#include <memory>
#include <set>
#include <stack>
#include <utility>
#include <vector>

#include "../tac/tac.h"
#include "../util/cuteprint.h"
#include "graph.hpp"

struct BasicBlock;
using BBPtr = std::shared_ptr<BasicBlock>;

class Position {
    bool _isPhi;

    BBPtr bb;
    TacOpd phiName;
    TacPtr tac;

public:
    Position(BBPtr, TacOpd);
    Position(BBPtr, TacPtr);

    bool isPhi() const;
    BBPtr getBB() const;
    TacOpd getPhiName() const;
    void setPhiName(TacOpd);
    TacPtr getTac() const;
    std::string name() const; 
};

using PosPtr = std::shared_ptr<Position>;

class FlowGraph {
private:
    BBPtr startBlock;
    std::set<BBPtr> blocks;

    void dfs_blocks(BBPtr bb, std::vector<BBPtr> &bblist, std::set<BBPtr> &ex_bb);

public:
    void addEdge(BBPtr fromEnd, BBPtr toEnd);
    void delEdge(BBPtr fromEnd, BBPtr toEnd);

    void cleanBB();

    void computeLiveness();
    void graphColoring();

    std::set<TacOpd> vars;
    std::map<TacOpd, PosPtr> def;
    std::map<TacOpd, std::set<PosPtr>> uses;
    std::shared_ptr<TacFunc> func;
    std::function<SpaceType(int)> fbase;

    FlowGraph(const Insts &insts, std::shared_ptr<TacFunc> func, std::function<SpaceType(int)> fbase);

    BBPtr getStartBlock() const;
    void setStartBlock(BBPtr block);
    std::vector<BBPtr> getBlocks() const;
    std::set<BBPtr> getBlockSet() const; 

    void toSatisfyUniquePredOrSuccProp();
    void addBlock(BBPtr block);

    void calcDominator();
    void toSSA();

    void calcVars();
    void calcDefUses();

    std::map<TacPtr, std::set<TacPtr>> getLiveLoad(const TacProg &prog);

    void remove(BBPtr block);
    void removeSingle(BBPtr block);

    void removeMov();

    void toTac();
    Insts toInsts();

    void clearPrint(std::ostream &os) const;

    template <class TV, class TE>
    Graph<TV, TE> *getStruct(std::function<TV(BBPtr)>,
                            std::function<TE(BBPtr, BBPtr)>) const;
};

struct BasicBlock {
    Insts insts;
    std::set<BBPtr> pred, succ;
    std::set<BBPtr> doms;

    int depth;

    BBPtr idom;
    std::set<BBPtr> child;

    std::map<TacOpd, std::map<BBPtr, TacOpd>>
        phi;  // var to def -> basicblock -> var to use

    void computeDepth();
    
    void addPred(BBPtr pred);
    void addSucc(BBPtr succ);
    void delPred(BBPtr pred);
    void delSucc(BBPtr succ);

    void setTrueBranch(BBPtr self, BBPtr tb);
    void setFalseBranch(BBPtr self, BBPtr tf);
    
    // for liveness analysis
    std::set<TacOpd> getDefs();
    std::set<TacOpd> getUses(); 
    std::set<TacOpd> getDefsFast() const;
    std::set<TacOpd> getUsesFast() const; 
    // compute liveness backward, and return true if is changed 
    //  i.e. more live-variables are detected 
    bool computeLiveness(const std::set<TacOpd> &liveOut);
    void cleanLive();
    void mergeLiveIn(std::set<TacOpd> &des); 
    void mergeLiveOut(std::set<TacOpd> &des);

    const std::set<TacOpd> &getLiveIn() const;
    const std::set<TacOpd> &getLiveOut() const;

    void replaceSucc(BBPtr self, BBPtr pre, BBPtr now);
    
    BBPtr getTrueBranch() const;
    BBPtr getFalseBranch() const;

    BBPtr getPred() const; // error when predecessor is not unique
    BBPtr getSucc() const; // error when successor is not unique

    bool isConditional() const; 

    void swapBranches();

    static bool dominate(BBPtr a, BBPtr b);
    static bool strictDominate(BBPtr a, BBPtr b);

    int getId() const;   

    void clearPrint(std::ostream &os) const;

    BasicBlock(); 

private:
    int id;
    friend FlowGraph::FlowGraph(const Insts &insts, std::shared_ptr<TacFunc> func, std::function<SpaceType(int)>);
    friend void FlowGraph::toSatisfyUniquePredOrSuccProp();
    BBPtr trueBranch, falseBranch; // only for conditional jump
    bool reachedBySource; // only used when constructing the CFG
    void visitFromSource();
    std::set<TacOpd> liveIn;
    std::set<TacOpd> liveOut;
    std::set<TacOpd> defs;
    std::set<TacOpd> uses;
    void computeUseDef();
};

std::ostream &operator<<(std::ostream &os, const BasicBlock &block);
std::ostream &operator<<(std::ostream &os, const FlowGraph &flowgraph);
