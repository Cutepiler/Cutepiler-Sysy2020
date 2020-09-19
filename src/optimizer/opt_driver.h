#pragma once
#include "../flowgraph/flowgraph.h"

void optimize(FlowGraph &flowgraph, TacProg &pg, const std::set<int> &pure_funcs, bool first, bool last);
void optimize_pred(TacProg &prog, std::set<int> &pure_funcs);
void optimize_mid(TacProg &prog, std::set<int> &pure_funcs);
void optimize_tac(FlowGraph &flowgraph);
