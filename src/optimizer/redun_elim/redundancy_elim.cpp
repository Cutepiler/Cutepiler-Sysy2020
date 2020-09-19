#include "redundancy_elim.h"
#include "../basic/work_list_easy.h"

DAG::DAG(std::set<BBPtr> nodes, const std::set<LoopPtr> &children, BBPtr header,
         FlowGraph &flowgraph, const std::set<int> &pure_funcs, const TacProg &prog)
    : flowgraph(flowgraph) {
    std::map<BBPtr, int> in_degree;

    for (auto subloop : children) {
        nodes.insert(subloop->pre_header);
    }

    for (auto node : nodes) alias[node] = node;

    auto addEdge = [&](BBPtr fr, BBPtr to) {
        if (!nodes.count(fr) || !nodes.count(to)) return;
        if (to == header) return;
        pred[to].insert(fr);
        succ[fr].insert(to);
    };

    for (auto node : nodes) {
        for (auto p : node->pred) addEdge(p, node);
        for (auto s : node->succ) addEdge(node, s);
    }

    for (auto subloop : children) {
        for (auto node : subloop->node_inside) {
            alias[node] = subloop->pre_header;
            for (auto s : node->succ) addEdge(subloop->pre_header, s);
        }
    }

    for (auto node : nodes) {
        in_degree[node] = pred[node].size();
        if (!in_degree[node]) bbInOrder.push_back(node);
    }

    int id = 0;
    while (id < bbInOrder.size()) {
        for (auto s : succ[bbInOrder[id]]) {
            --in_degree[s];
            if (!in_degree[s]) bbInOrder.push_back(s);
        }
        ++id;
    }
    assert(bbInOrder.size() == nodes.size());
    auto liveLoad = flowgraph.getLiveLoad(prog);
    for (auto bb : bbInOrder) lreAndGetComps(bb, pure_funcs, liveLoad);
    copy_propagation(flowgraph);
}

void DAG::lreAndGetComps(BBPtr bb, const std::set<int> &pure_funcs,
                         const std::map<TacPtr, std::set<TacPtr>> &liveLoad) {
    comps[bb].clear();
    for (auto inst = bb->insts.head->succ; inst != bb->insts.tail;
         inst = inst->succ) {
        auto c = Computation(inst, pure_funcs);
        if (!c.empty()) {
            auto pos = findComp(comps[bb], c, liveLoad);
            if (pos) {
                convert2Move(*this, inst, getDes(pos));
            } else {
                comps[bb].insert(c);
            }
        }
    }
}

Computation::Computation() : opr(TacOpr::_Head) {}

Computation::Computation(TacPtr inst, const std::set<int> &pure_funcs) {
    pos = inst;
    is_empty = true;
    switch (inst->opr) {
        case TacOpr::Neg:
        case TacOpr::Add:
        case TacOpr::Sub:
        case TacOpr::Mul:
        case TacOpr::Div:
        case TacOpr::Mod:
            is_empty = false;
            opr = inst->opr;
            if (!inst->opd2.empty()) opds.push_back(inst->opd2);
            if (!inst->opd3.empty()) opds.push_back(inst->opd3);
            break;
        case TacOpr::Call:
            if (!pure_funcs.count(inst->opd1.getVal())) break;
            is_empty = false;
            opr = TacOpr::Call;
            opds.push_back(inst->opd1);
            inst = inst->pred;
            while (inst->opr == TacOpr::Param) {
                opds.push_back(inst->opd1);
                inst = inst->pred;
            }
            break;
        case TacOpr::Load:
            opr = TacOpr::Load;
            is_empty = false;
            opds.push_back(inst->opd2);
            opds.push_back(inst->opd3);
            break;
        default:
            is_empty = true;
    }
}

bool Computation::operator<(const Computation &other) const {
    if (opr != other.opr) return opr < other.opr;
    if (opds.size() != other.opds.size()) return opds.size() < other.opds.size();
    for (int i = 0; i < opds.size(); ++i) {
        if (opds[i] != other.opds[i]) return opds[i] < other.opds[i];
    }
    return pos < other.pos;
}

bool Computation::same(const Computation &other) const {
    if (opr != other.opr) return false;
    if (opds.size() != other.opds.size()) return false;
    for (int i = 0; i < opds.size(); ++i) {
        if (opds[i] != other.opds[i]) return false;
    }
    return true;
}

bool Computation::empty() const { return is_empty; }

Computation Computation::rename(const std::map<TacOpd, std::map<BBPtr, TacOpd>> &phi,
                                BBPtr p) const {
    auto res = *this;
    for (auto &opd : res.opds) {
        if (phi.count(opd)) opd = phi.at(opd).at(p);
    }
    return res;
}

TacPtr findComp(const std::set<Computation> &comps, Computation comp,
                const std::map<TacPtr, std::set<TacPtr>> &liveLoad) {
/*    for (auto c : comps) {
        if (c.same(comp)) {
            if (c.opr == TacOpr::Load) {
                assert(liveLoad.count(comp.pos));
                if (!liveLoad.at(comp.pos).count(c.pos)) return nullptr;
            }
            return c.pos;
        }
    }
    return nullptr;*/
    auto pos = comp.pos;
    comp.pos = nullptr;
    auto it = comps.lower_bound(comp);
    if (it == comps.end()) return nullptr;
    if (!it->same(comp)) return nullptr;
    if (it->opr == TacOpr::Load) {
        assert(liveLoad.count(pos));
        if (!liveLoad.at(pos).count(it->pos)) return nullptr;
    }
    return it->pos;
}

TacOpd getDes(TacPtr inst) {
    for (int i = 1; i <= 4; ++i) {
        if (inst->isDef(i)) return inst->getOpd(i);
    }
    return TacOpd();
}

void convert2Move(DAG &g, TacPtr inst, const TacOpd &val) {
    auto des = getDes(inst);
    assert(!des.empty());
    inst->opr = TacOpr::Mov;
    inst->opd1 = des;
    inst->opd2 = val;
    inst->opd3 = inst->opd4 = TacOpd();
    while (inst->pred->opr == TacOpr::Param) {
        inst->pred->remove();
    }
}

std::ostream &operator<<(std::ostream &os, const DAG &g) {
    os << "Nodes in topsort order: ";
    for (auto bb : g.bbInOrder) {
        os << bb->getId() << " ";
    }
    os << std::endl;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Computation &c) {
    os << c.pos->to_string() << std::endl;
    os << "\t";
    for (auto opd : c.opds) os << opd.name() << " ";
    return os << std::endl;
}