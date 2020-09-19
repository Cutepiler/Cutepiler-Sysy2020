/* peephole optimization*/
#include "../../tac/tac.h"
#include "../../flowgraph/flowgraph.h"

bool algebraic_optimization(TacPtr ptr, std::function<SpaceType(int)> fbase);
bool algebraic_optimization(FlowGraph &flowgraph);
bool load_store_elim(BBPtr &block);
bool load_store_elim(FlowGraph &flowgraph);
bool peephole(FlowGraph &flowgraph);
