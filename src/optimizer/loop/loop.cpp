#include "loop.h"
#include "../../util/cuteprint.h"
#include "../basic/work_list_easy.h"
#include "../basic/dead_code_elim.h"
#include "../basic/cond_const_prop.h"
#include "../basic/peephole.h"
#include "../../env/env.h"
#include "../redun_elim/redundancy_elim.h"
#include <algorithm>
#include <map>
#include <queue> 
#include <iostream>

using std::set; 
using std::map; 
using std::make_shared; 
using std::set_union; 
using std::pair;
using std::make_pair;
using std::vector; 
using std::queue; 
using std::endl; 
using std::cerr;

void Loop::add_pre(TacPtr tac)
{
    pre_header->insts.push_back(tac); 
}

Loop::Loop()
{
    header = nullptr;
    pre_header = nullptr; 
    parent = nullptr; 
}

Loop::Loop(FlowGraph &flowgraph, set<BBPtr> nodes, BBPtr header)
{
    this->nodes = nodes;
    this->header = header; 
    space_type = flowgraph.fbase;
    parent = nullptr;
    pre_header = make_shared<BasicBlock>(); 
    flowgraph.addBlock(pre_header);
}

void Loop::setNodes(const set<BBPtr> &nodes)
{ this->nodes = nodes; }

void Loop::addNode(BBPtr bb)
{ nodes.insert(bb); }

set<BBPtr> Loop::getNodes() const
{ return nodes; }

void Loop::eraseNode(BBPtr bb) 
{ nodes.erase(bb); }

LoopPtr Loop::getParent() const
{ return parent; }

void Loop::setParent(LoopPtr parent) 
{ this->parent = parent; }

set<LoopPtr> Loop::getChildren() const
{ return children; }

void Loop::setChildren(const set<LoopPtr> &children) 
{ this->children = children; }

void Loop::addChild(LoopPtr child)
{ children.insert(child); }

BBPtr Loop::getHeader() const
{ return header; }

template<typename T>
static void quick_merge(set<T> &merge_tar, set<T> &merge_in)
{
    if (merge_tar.size() < merge_in.size()) merge_tar.swap(merge_in); 
    for (auto ele : merge_in) {
        merge_tar.insert(ele); 
    }
}

set<BBPtr> Loop::flatten()
{
    set<BBPtr> all_nodes = nodes;
    for (auto chl : children) {
        auto chl_nodes = chl->flatten(); 
        quick_merge(all_nodes, chl_nodes); 
    }
    return all_nodes; 
}

pair<set<BBPtr>, set<LoopPtr>> Loop::findLoops(FlowGraph &flowgraph, const set<BBPtr> &nodes)
{
    set<BBPtr> remaining = nodes;
    set<LoopPtr> loops;
    vector<BBPtr> node_list;
    for (auto node : nodes)
        node_list.push_back(node); 
    assert(flowgraph.getStartBlock()->idom == nullptr);
    flowgraph.calcDominator();
    flowgraph.getStartBlock()->computeDepth();
    sort(node_list.begin(), node_list.end(), 
        [](BBPtr x, BBPtr y) {
            if (x->depth != y->depth)
                return x->depth < y->depth;
            return x->getId() < y->getId(); 
        });
    for (auto node : node_list) {
        if (!remaining.count(node))
            continue;
        queue<BBPtr> que;
        set<BBPtr> edges;
        for (auto last : node->pred) 
            if (nodes.count(last) && BasicBlock::dominate(node, last))
                que.push(last); 
        if (que.empty()) 
            continue;
        edges.insert(node);
        while (!que.empty()) {
            auto ele = que.front(); 
            que.pop();
            edges.insert(ele);
            for (auto pre_node : ele->pred) { 
                if (!nodes.count(pre_node) || edges.count(pre_node))
                    continue;
                if (!BasicBlock::dominate(node, pre_node))
                    continue;
                que.push(pre_node);
            }
        }
        for (auto loop_node : edges) 
            remaining.erase(loop_node); 
        LoopPtr this_loop = make_shared<Loop>(flowgraph, edges, node);
        loops.insert(this_loop); 
    }
    return make_pair(remaining, loops); 
}

void Loop::build(FlowGraph &flowgraph, LoopPtr that)
{
    that->eraseNode(that->getHeader()); 
    auto [remaining, smaller_loops] = findLoops(flowgraph, that->getNodes());
    remaining.insert(that->getHeader()); 
    that->setChildren(smaller_loops); 
    that->setNodes(remaining);
    for (auto inside_loop : smaller_loops) {
        inside_loop->setParent(that); 
        build(flowgraph, inside_loop); 
    }  
}

pair<set<BBPtr>, set<LoopPtr>> Loop::constructTrees(FlowGraph &flowgraph, const std::set<BBPtr> &nodes)
{
    flowgraph.cleanBB();
    auto [remaining, loops] = findLoops(flowgraph, nodes);
    for (auto loop : loops)
        build(flowgraph, loop); 
    return make_pair(remaining, loops);
}

void Loop::display(int tab_size)
{
    for (int i = 0; i < tab_size; i++)
        cerr << "\t";
    cerr << "Loop with header : " << header->getId() << "; containing ";
    for (auto node : nodes) {
        cerr << node->getId() << " ";
    }
    cerr << endl; 
    for (auto chl : children) {
        chl->display(tab_size + 1); 
    }
}

void Loop::loopRotation(FlowGraph &flowgraph)
{
    node_inside = nodes;
    assert(!nodes.count(pre_header));
    for (auto loop : children) {
        loop->loopRotation(flowgraph);
        auto nds = loop->node_inside;
        for (auto node : nds)
            node_inside.insert(node); 
    }
    flowgraph.calcDominator();
    BBPtr header_tail = make_shared<BasicBlock>();
    for (auto inst : header->insts)
        header_tail->insts.push_back(make_shared<Tac>(inst));
    vector<BBPtr> delist;
    for (auto pred : header->pred)
        if (node_inside.count(pred)) {
            pred->replaceSucc(pred, header, header_tail);
            header_tail->pred.insert(pred);
            delist.push_back(pred);
        }
    for (auto de : delist)
        header->pred.erase(de);
    delist.clear();
    if (header->succ.size() == 1) {
        header_tail->succ = header->succ;
    } else {
        header_tail->setTrueBranch(header_tail, header->getFalseBranch());
        header_tail->setFalseBranch(header_tail, header->getTrueBranch());
        auto inst = header_tail->insts.tail->pred;
        switch (inst->opr) {
            case TacOpr::Beqz: inst->opr = TacOpr::Bnez; break;
            case TacOpr::Bnez: inst->opr = TacOpr::Beqz; break;
            default: assert(false);
        }
    }
    for (auto succ : header_tail->succ)
        succ->pred.insert(header_tail);
    flowgraph.addBlock(header_tail);
    node_inside.insert(header_tail);
}

void Loop::loopTransform(FlowGraph &flowgraph) 
{
    // recursively transform, and insert all the nodes 
    node_inside = nodes;
    assert(!nodes.count(pre_header));
    for (auto loop : children) {
        loop->loopTransform(flowgraph);
        auto nds = loop->node_inside;
        for (auto node : nds)
            node_inside.insert(node); 
    }
    node_inside.insert(pre_header);

    flowgraph.calcDominator();

    vector<BBPtr> delist;
    for (auto bb : header->pred)
        if (!node_inside.count(bb)) 
            delist.push_back(bb);
    for (auto bb : delist) {
        bb->replaceSucc(bb, header, pre_header);
        pre_header->pred.insert(bb);
        header->pred.erase(bb);
    }

    for (auto &[var, phi_src] : header->phi) {
        auto var_init = TacOpd::newReg();
        vector<BBPtr> delist;
        for (auto &[bb, val] : phi_src) 
            if (!node_inside.count(bb)) {
                pre_header->phi[var_init][bb] = val;
                delist.push_back(bb);
            }
        for (auto bb : delist) {
            phi_src.erase(bb);
        }   
        phi_src[pre_header] = var_init;
    }

    pre_header->succ.insert(header);
    header->pred.insert(pre_header);

    if (flowgraph.getStartBlock() == header) {
        flowgraph.setStartBlock(pre_header);
    }
}


void Loop::loopOpt(const set<int> &pure_funcs, FlowGraph &flowgraph, const TacProg &prog)
{
    for (auto chl : children)
        chl->loopOpt(pure_funcs, flowgraph, prog);
    std::set<BBPtr> ns = nodes;
    ns.erase(pre_header);
    bool changed;
    do {
        auto dag = DAG(ns, children, header, flowgraph, pure_funcs, prog);
        changed = global_redundancy_elimintaion(dag, flowgraph, pure_funcs, prog);
    } while (changed);
    invariantOpt(flowgraph, pure_funcs, prog);
    inductionVar(pure_funcs);
}

void Loop::getFrequency(map<TacOpd, double> &des)
{
    for (auto chl : children) 
        chl->getFrequency(des);
    map<TacOpd, int> cnt_map;
    for (auto block : nodes) {
        for (auto inst : block->insts) {
            auto uses = inst.getUses();
            for (auto var : uses) {
                cnt_map[var]++;
                if (!des.count(var))
                    des[var] = 0;
            }
        }
    }
    for (auto &[var, fre] : des) {
        fre = fre * 16 + cnt_map[var];
    }
}

void loop_rotation(FlowGraph &flowgraph)
{
    auto [remaining, loops] = Loop::constructTrees(flowgraph, flowgraph.getBlockSet());
    for (auto loop : loops) {
        loop->loopRotation(flowgraph);
    }
}

void loop_acc_opt(FlowGraph &flowgraph, const std::set<int> &pure_funcs, const TacProg &prog)
{
    auto [remaining, loops] = Loop::constructTrees(flowgraph, flowgraph.getBlockSet());
    for (auto loop : loops) {
        loop->loopTransform(flowgraph);
        loop->accLoop(flowgraph, pure_funcs, prog);
    }
}

void loop_optimization(FlowGraph &flowgraph, const set<int> &pure_funcs, const TacProg &prog)
{
    logger << "-- Loop Optimization --" << endl; 

    
    auto [remaining, loops] = Loop::constructTrees(flowgraph, flowgraph.getBlockSet());

    if (loops.empty()) flowgraph.func->hasLoop = false;

    for (auto loop : loops)
        loop->loopTransform(flowgraph);

    bool changedgre;
    do {
        auto dag = DAG(remaining, loops, flowgraph.getStartBlock(), flowgraph, pure_funcs, prog);
        changedgre = global_redundancy_elimintaion(dag, flowgraph, pure_funcs, prog);
    } while (changedgre);

    auto blocks = flowgraph.getBlocks();
    for (auto block : blocks)
        for (auto &[var, phi_src] : block->phi)
            assert(block->pred.size() == phi_src.size());

    for (auto loop : loops) {
        loop->loopOpt(pure_funcs, flowgraph, prog);
    }

    flowgraph.calcVars();
    flowgraph.calcDefUses(); 
    
    bool changed;
    do {
        changed = false; 
        changed = peephole(flowgraph) || changed;
        changed = worklist_easy(flowgraph) || changed;
        changed = dead_code_elimination(flowgraph, pure_funcs) || changed;
        changed = cond_const_prop(flowgraph) || changed;
    } while (changed);

//    debugger << flowgraph << endl;

    int loop_count = loops.size();

    for (auto loop : loops) {
        if (loop->loopUnrolling(pure_funcs)) --loop_count;
    }

    if (!loop_count) flowgraph.func->hasLoop = false;

    flowgraph.calcVars();
    flowgraph.calcDefUses(); 
    
} 

map<TacOpd, double> get_frequency(FlowGraph &flowgraph)
{
    flowgraph.calcDominator();
    auto [remaining, loops] = Loop::constructTrees(flowgraph, flowgraph.getBlockSet());
    map<TacOpd, double> fre_map;
    for (auto loop : loops) {
        // loop->display();
        loop->getFrequency(fre_map); 
    }
    for (auto block : remaining) {
        for (auto inst : block->insts) {
            auto uses = inst.getUses();
            for (auto var : uses)
                fre_map[var]++;
        }
    }
    return fre_map;
}
