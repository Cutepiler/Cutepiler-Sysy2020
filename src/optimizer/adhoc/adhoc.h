#include "../../flowgraph/flowgraph.h"

void take_whole_array_out(TacProg &prog, FlowGraph &flowgraph);
void branch_merging(Insts &insts);
bool simple_if_else_trans(FlowGraph &flowgraph);
bool simple_if_trans(FlowGraph &flowgraph);