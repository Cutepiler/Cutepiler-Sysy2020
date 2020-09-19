/* optimization driver file */
#include "../flowgraph/flowgraph.h"
#include "basic/cond_const_prop.h"
#include "basic/dead_code_elim.h"
#include "basic/work_list_easy.h"
#include "basic/peephole.h"
#include "basic/merge_basic_blocks.h"
#include "loop/loop.h"
#include "lowlevel/inst_merging.h"
#include "lowlevel/str_reduction.h"
#include "intraprocedure/intra_driver.h"
#include "adhoc/adhoc.h"
#include "opt_driver.h"
#include "../env/env.h"
#include <iostream>
#include <set>

using std::set;

void optimize(FlowGraph &flowgraph, TacProg &prog, const set<int> &pure_funcs, bool first, bool last)
{
    bool changed = false; 
    do {
        changed = false; 
        changed = peephole(flowgraph) || changed;
        changed = local_common_subexp_elim(flowgraph) || changed;
        changed = worklist_easy(flowgraph) || changed;
        changed = dead_code_elimination(flowgraph, pure_funcs) || changed;
        changed = cond_const_prop(flowgraph) || changed;
        if (first) changed = const_array_elim(flowgraph, prog) || changed; 
        changed = pure_func_merging(flowgraph, pure_funcs) || changed;
    } while (changed);
    if (first) {
        loop_optimization(flowgraph, pure_funcs, prog);
        debugger <<flowgraph << std::endl;
    }
    merge_basic_blocks(flowgraph);
    changed = false; 
    do {
        changed = false; 
        changed = peephole(flowgraph) || changed;
        changed = local_common_subexp_elim(flowgraph) || changed;
        changed = worklist_easy(flowgraph) || changed;
        changed = dead_code_elimination(flowgraph, pure_funcs) || changed;
        changed = cond_const_prop(flowgraph) || changed;
        if (first) changed = const_array_elim(flowgraph, prog) || changed; 
        changed = pure_func_merging(flowgraph, pure_funcs) || changed;
    } while (changed);
    take_whole_array_out(prog, flowgraph);
    changed = false; 
    do {
        changed = false; 
        changed = peephole(flowgraph) || changed;
        changed = local_common_subexp_elim(flowgraph) || changed;
        changed = worklist_easy(flowgraph) || changed;
        changed = dead_code_elimination(flowgraph, pure_funcs) || changed;
        changed = cond_const_prop(flowgraph) || changed;
        if (first) changed = const_array_elim(flowgraph, prog) || changed; 
        changed = pure_func_merging(flowgraph, pure_funcs) || changed;
    } while (changed);
    flowgraph.toSatisfyUniquePredOrSuccProp();
    flowgraph.toTac();
    if (last) {
        loop_acc_opt(flowgraph, pure_funcs, prog);
    }
    if (last) {
        loop_rotation(flowgraph);
        strength_reduction(flowgraph);
        logger << "Before Instr Merging" << std::endl;
        logger << flowgraph << std::endl;
        inst_merge(flowgraph, prog.fbase);
        schedule(flowgraph);
    }
}

void optimize_pred(TacProg &prog, set<int> &pure_funcs)
{
    pure_funcs.clear();
    compute_pure_func(prog);
    for (auto func : prog.funcs)
        if (func->is_pure)
            pure_funcs.insert(func->id);
    bool changed = false;
    do {
        changed = false;
        changed = auto_inline(prog, pure_funcs) || changed;
    } while (changed);
    pure_funcs.clear();
    compute_pure_func(prog);
    for (auto func : prog.funcs)
        if (func->is_pure)
            pure_funcs.insert(func->id);
}

void optimize_mid(TacProg &prog, set<int> &pure_funcs) {
    pure_funcs.clear();
    bool changed = false;
    do {
        changed = false;
        changed = auto_inline(prog, pure_funcs) || changed;
    } while (changed);
    compute_pure_func(prog);
    for (auto func : prog.funcs)
        if (func->is_pure)
            pure_funcs.insert(func->id);
}

void optimize_tac(FlowGraph &flowgraph)
{
    bool changed = false;
    do {
        changed = false;
        changed = elim_addr_expr(flowgraph) || changed;
    } while (changed);
}
