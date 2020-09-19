/* finding loops and build loop trees */
#pragma once
#include "../../tac/tac.h"
#include "../../flowgraph/flowgraph.h"
#include "../../flowgraph/pointer_analysis.h"
#include <set>
#include <vector>
#include <algorithm>
#include <stack>
#include <functional>

class Loop;
typedef std::shared_ptr<Loop> LoopPtr; 

struct DAG;

void loop_optimization(FlowGraph &flowgraph, const std::set<int> &pure_funcs, const TacProg &prog);
void loop_rotation(FlowGraph &flowgraph);
void loop_acc_opt(FlowGraph &flowgraph, const std::set<int> &pure_funcs, const TacProg &prog);

std::map<TacOpd, double> get_frequency(FlowGraph &flow);

class DefInfo {
private:
    std::set<TacOpd> defs;
    std::set<int> dirty_space;
    bool all_dirty;
public:
    void insert(const TacOpd &opd);
    void erase(const TacOpd &opd);
    void set_all(bool new_val);
    void set_dirty(int space_id);
    bool get_all() const;
    void set_clean(int space_id);
    bool is_dirty(int space_id);
    bool has_def(const TacOpd &opd);
    void merge(const DefInfo &from);
    void clear();
    const std::set<TacOpd>& get_defs() const;
};

struct AffineVar {
    TacOpd a, b;
};

struct LoopInfo {
    bool simple;
    
    // for (int i = start; i (type) end; i += step)
    TacOpr type; // i.e. Lt, Le, Gt, Ge, Eq, Ne
    TacOpd start;
    TacOpd end;
    TacOpd step;

    LoopInfo() : simple(false) {}
    LoopInfo(TacOpr type, TacOpd start, TacOpd end, TacOpd step)
        : simple(true), type(type), start(start), end(end), step(step) {}
};

bool operator == (const AffineVar &a, const AffineVar &b); 

class Loop {
private:
    /* At the beginning, it contains all the basicblocks that are 
     *  DIRECTLY inside this loop (i.e. not inside a inner loop) 
     * The optimization algorithm is a Tree DP algorithm, which 
     *  converts the whole subtree into a new tree. 
     */
    std::set<BBPtr> nodes, node_inside;
    BBPtr header; 
    BBPtr pre_header;
    BBPtr ind_pre;
    std::set<LoopPtr> children; 
    LoopPtr parent; 
    BBPtr in_header;

    std::function<SpaceType(int)> space_type;

    /* opd |-> [init, step] */
    std::map<TacOpd, std::pair<TacOpd, TacOpd>> index_table;
    std::map<TacOpd, TacOpd> induction_set;

    /* insert an instruction @tac into the pre_header */
    void add_pre(TacPtr tac);
    DefInfo defs;
    std::shared_ptr<PointerAnalyzer> ptz;
    std::vector<TacPtr> store_insts; 
    // must be called after defs have been computed
    bool is_invariant(TacPtr tac);
    bool is_invariant(const TacOpd &opd);
    void remove_update_defs(TacPtr tac);

    TacOpd opd_add(const TacOpd &a, const TacOpd &b);
    TacOpd opd_sub(const TacOpd &a, const TacOpd &b);
    TacOpd opd_neg(const TacOpd &a);
    TacOpd opd_mul(const TacOpd &a, const TacOpd &b);

    AffineVar affine_add(const AffineVar &var, const TacOpd &opd);
    AffineVar affine_sub(const AffineVar &var, const TacOpd &opd);
    AffineVar affine_sub(const TacOpd &opd, const AffineVar &var);
    AffineVar affine_mul(const AffineVar &var, const TacOpd &opd);

    LoopInfo computeLoopInfo(const std::set<int> &pure_funcs);

    void total_unrolling(int times);

    /* translate TAC into SSA, in a single block */
    void renaming(BBPtr bb);

public:

    Loop(); 
    Loop(FlowGraph &flowgraph, std::set<BBPtr> nodes, BBPtr header); 
    std::set<BBPtr> getNodes() const; 
    void setNodes(const std::set<BBPtr> &nodes);
    void addNode(BBPtr bb); 
    BBPtr getHeader() const;
    void eraseNode(BBPtr bb);
    LoopPtr getParent() const; 
    void setParent(LoopPtr parent);
    std::set<LoopPtr> getChildren() const; 
    void setChildren(const std::set<LoopPtr> &children);
    void addChild(LoopPtr child); 

    void getFrequency(std::map<TacOpd, double> &des);

    // compute defs inside the loop
    void computeDefs(const std::set<int> &pure_funcs);
    void loopOpt(const std::set<int> &pure_funcs, FlowGraph &flowgraph, const TacProg &prog);
    const DefInfo& getDefs() const;
    // after defs have been computed, run loop-invariant optimization 
    void invariantOpt(FlowGraph &flowgraph, const std::set<int> &pure_funcs, const TacProg &prog);
    void inductionVar(const std::set<int> &pure_funcs);
    bool loopUnrolling(const std::set<int> &pure_funcs);
    void accLoop(FlowGraph &flowgraph, const std::set<int> &pure_funcs, const TacProg &prog);

    BBPtr getPreHeader() const; 

    void display(int tab_size = 0); 

    /* find (outer) loops and return thoses basicblocks 
     *  that are not inside any loop 
     */
    static std::pair<std::set<BBPtr>, std::set<LoopPtr>> 
        findLoops(FlowGraph &flowgraph, const std::set<BBPtr> &nodes); 
    // find all (inside) loops and recursively build the tree
    static void build(FlowGraph &flowgraph, LoopPtr that);
    static std::pair<std::set<BBPtr>, std::set<LoopPtr>> 
        constructTrees(FlowGraph &flowgraph, const std::set<BBPtr> &nodes); 

    // flatten the loop to get all the nodes inside it
    std::set<BBPtr> flatten();

    // optimization algorithm entry
    void optimize();

    void loopTransform(FlowGraph &flowgraph); 
    void loopRotation(FlowGraph &flowgraph);

    friend struct DAG;
};
