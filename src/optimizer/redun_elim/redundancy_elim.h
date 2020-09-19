#include "../loop/loop.h"

static const int RANK_INF = 1e9;

class Computation {
    bool is_empty;

public:
    Computation();
    Computation(TacPtr inst, const std::set<int> &pure_funcs);
    bool operator<(const Computation &other) const;
    bool same(const Computation &other) const;
    bool empty() const;

    TacOpr opr;
    std::vector<TacOpd> opds;
    TacPtr pos;
    Computation rename(const std::map<TacOpd, std::map<BBPtr, TacOpd>> &phi,
                       BBPtr p) const;
};

struct DAG {
    //    DAG(const Loop &loop, FlowGraph &flowgraph, const std::set<int>
    //    &pure_funcs);
    DAG(std::set<BBPtr> nodes, const std::set<LoopPtr> &children, BBPtr header,
        FlowGraph &flowgraph, const std::set<int> &pure_funcs, const TacProg &prog);

    std::vector<BBPtr> bbInOrder;
    std::map<BBPtr, std::set<BBPtr>> pred, succ;
    std::map<BBPtr, std::set<Computation>> comps;
    std::map<BBPtr, BBPtr> alias;
    FlowGraph &flowgraph;

    void lreAndGetComps(BBPtr bb, const std::set<int> &pure_funcs,
                        const std::map<TacPtr, std::set<TacPtr>> &liveLoad);
};

void partial_redundancy_elimination(DAG &g, FlowGraph &flowgraph,
                                    const std::set<int> &pure_funcs);
bool global_redundancy_elimintaion(DAG &g, FlowGraph &flowgraph,
                                   const std::set<int> &pure_funcs,
                                   const TacProg &prog);

TacPtr findComp(const std::set<Computation> &comps, Computation comp,
                const std::map<TacPtr, std::set<TacPtr>> &liveLoad);
TacOpd getDes(TacPtr inst);
void convert2Move(DAG &g, TacPtr inst, const TacOpd &val);

std::ostream &operator<<(std::ostream &os, const DAG &g);
std::ostream &operator<<(std::ostream &os, const Computation &c);