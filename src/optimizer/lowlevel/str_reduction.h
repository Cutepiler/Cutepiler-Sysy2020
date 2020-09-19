#include "../../flowgraph/flowgraph.h"

bool strength_reduction(TacPtr inst);
bool strength_reduction(FlowGraph &flowgraph);
void schedule(FlowGraph &flowgraph);