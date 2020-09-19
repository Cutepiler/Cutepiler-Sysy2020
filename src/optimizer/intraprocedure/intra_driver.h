#include "../../flowgraph/flowgraph.h"
#include "../../tac/tac.h"

bool auto_inline(TacProg &prog, const std::set<int> &pure_funcs);
bool pure_func_merging(FlowGraph &flowgraph, const std::set<int> &pure_funcs);
void compute_pure_func(TacProg &pg);
