#include "../tac/tac.h"
#include "flowgraph.h"
#include <set>

using std::set;

bool is_pointer(const TacOpd &opd);
set<int> get_space(const TacOpd &opd);
TacOpd get_source(const TacOpd &opd);
void work_iteration(const TacProg &pg,const FlowGraph &flowgraph);
void work_iteration(const TacProg &pg);
void display_pointer(const FlowGraph &flowgraph);
void display_pointer(const TacProg &pg);