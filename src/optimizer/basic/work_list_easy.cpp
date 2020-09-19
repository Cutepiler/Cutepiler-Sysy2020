/* easy optimization */

#include "work_list_easy.h"
#include "../../util/cuteprint.h"
#include <queue> 
#include <functional>
#include <map>
#include <vector>
#include <iostream>  
#include <set> 

using std::queue; 
using std::function;
using std::map; 
using std::make_shared;
using std::vector;
using std::cerr;
using std::endl; 
using std::set; 
using std::pair;

// @return: true if modify, false otherwise 
bool constant_folding(TacPtr ptr) 
{
    function<int(int)> func1;
    function<int(int,int)> func2; 
    enum {UNARY, BINARY, NONE};
    int type = NONE; 
    switch (ptr->opr) {
        case TacOpr::Not: func1 = [](int x) { return int(!x); }; break; 
        case TacOpr::Neg: func1 = [](int x) { return -x; }; break; 
        case TacOpr::Add: func2 = [](int x, int y) { return x + y; }; break; 
        case TacOpr::Sub: func2 = [](int x, int y) { return x - y; }; break; 
        case TacOpr::Mul: func2 = [](int x, int y) { return x * y; }; break; 
        case TacOpr::Div: func2 = [](int x, int y) { return x / y; }; break; 
        case TacOpr::Mod: func2 = [](int x, int y) { return x % y; }; break;
        case TacOpr::Gt:  func2 = [](int x, int y) { return x > y; }; break; 
        case TacOpr::Lt:  func2 = [](int x, int y) { return x < y; }; break; 
        case TacOpr::Ge:  func2 = [](int x, int y) { return x >= y; }; break;
        case TacOpr::Le:  func2 = [](int x, int y) { return x <= y; }; break; 
        case TacOpr::Eq:  func2 = [](int x, int y) { return x == y; }; break; 
        case TacOpr::Ne:  func2 = [](int x, int y) { return x != y; }; break;   
        case TacOpr::And: func2 = [](int x, int y) { return x && y; }; break; 
        case TacOpr::Or:  func2 = [](int x, int y) { return x || y; }; break; 
        default: break;
    }

    switch(ptr->opr) {
        case TacOpr::Not: 
        case TacOpr::Neg:
            if (ptr->opd2.getType() == OpdType::Imme)
                type = UNARY;  
            break; 
        case TacOpr::Add: 
        case TacOpr::Sub: 
        case TacOpr::Mul: 
        case TacOpr::Div: 
        case TacOpr::Mod: 
        case TacOpr::Gt:  
        case TacOpr::Lt:
        case TacOpr::Ge:
        case TacOpr::Le:
        case TacOpr::Eq:
        case TacOpr::Ne:
        case TacOpr::And:
        case TacOpr::Or:
            if (ptr->opd2.getType() == OpdType::Imme && 
                ptr->opd3.getType() == OpdType::Imme) {
                type = BINARY; 
            }
            break; 
        default: break;   
    }
    
    switch (type) {
        case UNARY: 
            ptr->opr = TacOpr::Mov;
            ptr->opd2 = TacOpd::newImme(func1(ptr->opd2.getVal()));
            return true; 
        case BINARY: 
            ptr->opr = TacOpr::Mov; 
            ptr->opd2 = TacOpd::newImme(func2(ptr->opd2.getVal(), ptr->opd3.getVal())); 
            ptr->opd3 = TacOpd(); 
            return true; 
        default: 
            return false; 
    }
}

bool constant_folding(FlowGraph &flowgraph)
{
    bool changed = false;
    auto blocks = flowgraph.getBlocks();
    for (auto bb : blocks) {
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ) {
            changed = constant_folding(inst) || changed; 
        }
    }
    return changed; 
}

bool copy_propagation(FlowGraph &flowgraph) 
{
    // debugger << "Before" << endl;
    // debugger << flowgraph << endl; 

    map<TacOpd, vector<TacOpd>> graph;   // the graph of copy
    map<TacOpd, TacOpd> rep;     // representation 
    map<TacOpd, int> in_degree;  // in degree of a node
    set<TacOpd> graph_v; 
    auto push_graph = [&graph,&graph_v,&in_degree] (TacOpd from, TacOpd tar) {
        graph[from].push_back(tar); 
        in_degree[tar]++; 
        graph_v.insert(from);
        graph_v.insert(tar); 
    };

    auto compute_rep = [&rep,&graph_v,flowgraph,&graph,&in_degree] () {
        queue<TacOpd> que; 
        for (auto var : graph_v) {
            if (in_degree[var] == 0) {
                que.push(var); 
                rep[var] = var;
                // cerr << "rep " << var.name() << endl;
            }
        }
        while (!que.empty()) {
            auto var = que.front();
            que.pop();
            auto res = rep[var];
            for (auto nxt : graph[var]) {
                if (!rep.count(nxt)) {
                    rep[nxt] = res;
                    // cerr << "rep: " << nxt.name() << "<--" << res.name() << endl; 
                    que.push(nxt);
                }
            }
        }
    };
    
    auto blocks = flowgraph.getBlocks();
    for (auto bb : blocks) {
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ) {
            if (inst->opr == TacOpr::Mov) {
                push_graph(inst->opd2, inst->opd1);
            } 
        }
        vector<TacOpd> phi_erase; 
        for (auto [var, phi_src] : bb->phi) {
            assert(phi_src.size() == bb->pred.size());
            const auto &[bb_ptr, src] = *phi_src.begin();
            bool is_single = true;
            for (const auto &[bb_ptr_1, src_2] : phi_src) {
                if (src != src_2)
                    is_single = false;
            }
            if (is_single) {
                push_graph(src, var);
                phi_erase.push_back(var);
            }
        }
        for (auto var : phi_erase) {
            bb->phi.erase(var); 
        }
    }

    bool changed = false;
    compute_rep(); 

    vector<TacPtr> delist; 
    blocks = flowgraph.getBlocks();
    for (auto bb : blocks) {
        auto insts = bb->insts; 
        for (auto &[var, phi_src] : bb->phi) {
            for (auto &[id, src] : phi_src) {
                if (rep.count(src) && rep[src] != src) {
                    src = rep[src];
                    changed = true; 
                }
            }
        }
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ) {
            for (int i = 1; i <= 3; i++) {
                auto &opd = inst->getOpd(i);  
                if (inst->isUse(i) && rep.count(opd)) {
                    changed = opd != rep[opd] || changed; 
                    opd = rep[opd];
                }
            }
        }
    }
    
    blocks = flowgraph.getBlocks();
    for (auto bb : blocks) {
        auto insts = bb->insts; 
        for (auto &[var, phi_src] : bb->phi) {
            assert(phi_src.size() == bb->pred.size());
            for (auto &[id, src] : phi_src) {
                assert(src.getType() != OpdType::Null);
                assert(src.getType() != OpdType::Label);
            }
        }
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ) {
            if (inst->opr == TacOpr::Mov) {
                // cerr << "inst: " << inst->to_string() << " " << rep.count(inst->opd1) << endl; 
                assert(rep.count(inst->opd1) && rep[inst->opd1] != inst->opd1); 
                assert(inst->opd2.getType() != OpdType::Null);
                delist.push_back(inst);
            }
        }
    }

    for (auto inst : delist) {
        inst->remove(); 
    }

    flowgraph.calcVars(); 

    // debugger << "After" << endl; 
    // debugger << flowgraph << endl; 

    return changed; 
}

bool constant_conditions(FlowGraph &flowgraph)
{
    bool changed = false; 
    auto blocks = flowgraph.getBlocks();
    for (auto bb : blocks) {
        auto btac = bb->insts.tail->pred;
        if (btac->opr != TacOpr::Beqz && btac->opr != TacOpr::Bnez)
            continue;
        if (btac->opd1.getType() != OpdType::Imme)
            continue; 
        changed = true; 
        auto true_branch = bb->getTrueBranch();
        auto false_branch = bb->getFalseBranch();
        auto tar = btac->opd2; 
        assert(false_branch != nullptr);
        assert(tar.getType() == OpdType::Label); 
        bool remain; 
        switch (btac->opr) {
            case TacOpr::Beqz:
                remain = btac->opd1.getVal() == 0; 
                break; 
            case TacOpr::Bnez: 
                remain = btac->opd1.getVal() != 0; 
                break; 
            default: assert(false); 
        }
        btac->remove(); 
        if (remain == true) {
            flowgraph.delEdge(bb, false_branch);
        } else {
            flowgraph.delEdge(bb, true_branch); 
        }
    }
    return changed; 
}

bool local_common_subexp_elim(BBPtr &block){
    map<pair<TacOpr, vector<TacOpd>>, TacOpd> subexp;
    vector<TacOpd> opdvec;
    pair<TacOpr, vector<TacOpd>> p;
    subexp.clear();
    bool changed = false;
    Insts insts = block->insts;
    for (auto inst = insts.head->succ;inst != insts.tail;inst = inst->succ){
        switch(inst->opr) {
            case TacOpr::Not: 
            case TacOpr::Neg:
                if (inst->opd2.getType() == OpdType::Reg){
                    opdvec.clear();
                    opdvec.push_back(inst->opd2);
                    p = make_pair(inst->opr, opdvec);
                    if (subexp.find(p) != subexp.end()){
                        changed = true;
                        inst->opd2 = subexp[p];
                        inst->opr = TacOpr::Mov;
                    }
                    else subexp[p] = inst->opd1;
                }
                break; 
            case TacOpr::Add: 
            case TacOpr::Mul:
            case TacOpr::Eq:
            case TacOpr::Ne:
            case TacOpr::And:
            case TacOpr::Or:
                if (inst->opd2.getType() == OpdType::Reg || inst->opd3.getType() == OpdType::Reg){
                    opdvec.clear();
                    opdvec.push_back(inst->opd2);
                    opdvec.push_back(inst->opd3);
                    sort(opdvec.begin(), opdvec.end());
                    p = make_pair(inst->opr, opdvec);
                    if (subexp.find(p) != subexp.end()){
                        changed = true;
                        inst->opd2 = subexp[p];
                        inst->opd3 = TacOpd();
                        inst->opr = TacOpr::Mov;
                    }
                    else subexp[p] = inst->opd1;
                }
                break;
            case TacOpr::Sub: 
            case TacOpr::Div: 
            case TacOpr::Mod: 
            case TacOpr::Gt:  
            case TacOpr::Lt:
            case TacOpr::Ge:
            case TacOpr::Le:
                if (inst->opd2.getType() == OpdType::Reg || inst->opd3.getType() == OpdType::Reg){
                    opdvec.clear();
                    opdvec.push_back(inst->opd2);
                    opdvec.push_back(inst->opd3);
                    p = make_pair(inst->opr, opdvec);
                    if (subexp.find(p) != subexp.end()){
                        changed = true;
                        inst->opd2 = subexp[p];
                        inst->opd3 = TacOpd();
                        inst->opr = TacOpr::Mov;
                    }
                    else subexp[p] = inst->opd1;
                }
                break; 
            default: break;   
        }
    }
    return changed;
}

bool local_common_subexp_elim(FlowGraph &flowgraph){
    bool changed = false;
    auto blocks = flowgraph.getBlocks();
    for (auto bb : blocks){
        changed = local_common_subexp_elim(bb) || changed;
    }
    return changed;
}

bool worklist_easy(FlowGraph &flowgraph) 
{
    bool changed, this_changed = false;
    do {
        changed = false;
        changed = constant_folding(flowgraph) || changed;
        changed = copy_propagation(flowgraph) || changed;
        changed = constant_conditions(flowgraph) || changed; 
        changed = temp_addr_elim(flowgraph) || changed;
        changed = elim_addr_expr(flowgraph) || changed;
        changed = load_store_elim_general(flowgraph) || changed;
        this_changed = this_changed || changed;
    } while (changed);
    return this_changed; 
}
