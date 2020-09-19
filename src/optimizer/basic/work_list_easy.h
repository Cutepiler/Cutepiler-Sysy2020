/* work list algorithm */

#include "../../tac/tac.h"
#include "../../flowgraph/flowgraph.h"

bool constant_folding(TacPtr ptr);
bool constant_folding(FlowGraph &flowgraph);
bool local_common_subexp_elim(BBPtr &block);
bool local_common_subexp_elim(FlowGraph &flowgraph);
bool copy_propagation(FlowGraph &flowgraph);
bool constant_conditions(FlowGraph &flowgraph);
bool worklist_easy(FlowGraph &flowgraph);
bool temp_addr_elim(FlowGraph &flowgraph);
bool elim_addr_expr(FlowGraph &flowgraph);
bool const_array_elim(FlowGraph &flowgraph, const TacProg &pg);
bool load_store_elim_general(FlowGraph &flowgraph);
