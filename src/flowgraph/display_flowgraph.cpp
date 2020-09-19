#include "flowgraph.h"

using std::ostream;
using std::endl; 

ostream& operator << (ostream &os, const BasicBlock &block)
{
    os << "-- Start of BasicBlock " << block.getId() << " --" << endl;
    os << "[Pred]: ";
    for (auto pp : block.pred) 
        os << pp->getId() << " ";
    os << endl; 
    os << "[Succ]: ";
    for (auto ss : block.succ) 
        os << ss->getId() << " ";
    os << endl; 
    os << "[Children(Dom)]: ";
    for (auto chl : block.child) 
        os << chl->getId() << " "; 
    os << endl; 
    os << "[Doms]: ";
    for (auto dom : block.doms) {
        if (block.idom != nullptr && dom->getId() == block.idom->getId()) 
            os << dom->getId() << "(idom) ";
        else 
            os << dom->getId() << " ";
    }
    os << endl; 
    os << "[Phi Functions]" << endl;
    for (auto [opd, phi] : block.phi) {
        assert(opd.getType() == OpdType::Reg); 
        os << opd.name() << ": ";
        for (auto [from, reg] : phi) {
            os << from->getId() << ":" << reg.name() << " ";
        }
        os << endl; 
    }

    int id = 0;
    for (auto inst : block.insts) {
        os << id << "\t::\t" << inst.to_string() << endl;
        os << "\t\t LiveIn = ";
        auto livein = inst.getLiveIn(), liveout = inst.getLiveOut();
        for (auto var : livein)
            os << var.name() << " ";
        os << "; LiveOut = ";
        for (auto var : liveout)
            os << var.name() << " ";
        os << ";" << endl;
        id++; 
    }
    os << "-- End of BasicBlock --" << endl;
    return os;
}

ostream& operator << (ostream &os, const FlowGraph &flowgraph)
{
    os << "== Start of FlowGraph ==" << endl; 
    os << "[Number of blocks]: " << flowgraph.getBlocks().size() << endl;
    os << "[Start Block]: " << flowgraph.getStartBlock()->getId() << endl; 
    os << "[Vars]: ";
    for (auto var : flowgraph.vars) 
        os << var.name() << " ";
    os << endl; 
    os << "[Defs]:" << endl;
    for (auto [opt, pos] : flowgraph.def) 
        os << "\t" << opt.name() << "@" << pos->name() << endl;
    os << "[Uses]:" << endl; 
    for (auto [opt, poset] : flowgraph.uses) {
        os << "\t" << opt.name() << endl; 
        for (auto pos : poset) {
            os << "\t\t" << pos->name() << endl;
        }
    }
    for (auto bb : flowgraph.getBlocks()) {
        os << *bb; 
        os << endl;
    }
    os << "== End of FlowGraph ==" << endl;
    return os; 
}

void BasicBlock::clearPrint(ostream &os) const {
    os << "-- Start of BasicBlock " << getId() << " --" << endl;
    os << "[Pred]: ";
    for (auto pp : pred) 
        os << pp->getId() << " ";
    os << endl; 
    os << "[Succ]: ";
    for (auto ss : succ) 
        os << ss->getId() << " ";
    os << endl; 

    int id = 0;
    for (auto inst : insts) {
        os << id << "\t::\t" << inst.to_string() << endl;
        id++; 
    }
    os << "-- End of BasicBlock --" << endl;
}

void FlowGraph::clearPrint(ostream &os) const {
    os << "== Start of FlowGraph ==" << endl; 
    os << "[Number of blocks]: " << getBlocks().size() << endl;
    os << "[Start Block]: " << getStartBlock()->getId() << endl;
    os << endl; 
    for (auto bb : getBlocks()) {
        bb->clearPrint(os);
        os << endl;
    }
    os << "== End of FlowGraph ==" << endl;
}