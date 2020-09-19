#include "loop.h"
#include "../../util/cuteprint.h"
#include <cassert>
#include <map>
#include <set>
#include <vector>

using std::map;
using std::set;
using std::vector;
using std::make_shared;
using std::make_pair;

LoopInfo Loop::computeLoopInfo(const std::set<int> &pure_funcs)
{
    if (!children.empty())
        return LoopInfo();
    for (auto inst : header->insts)
        if (inst.opr == TacOpr::Call)
            return LoopInfo();
    for (auto [opd, info] : index_table) {
        auto [start, step] = info;
        // std::cerr << opd.name() << " " << start.name() << " " << step.name() << std::endl;
    }
    node_inside.erase(pre_header);
    computeDefs(pure_funcs);
    node_inside.insert(pre_header);
    auto last = header->insts.tail->pred;
    auto compare = last->pred;
    bool inside;
    if (last->opr == TacOpr::Beqz) {
        if (node_inside.count(header->getFalseBranch()))
            inside = true;              // inside the loop when compare is true
        else 
            inside = false;             // inside the loop when compare is not true
    } else if (last->opr == TacOpr::Bnez) {
        if (node_inside.count(header->getFalseBranch()))
            inside = false;             // inside the loop when compare is not true
        else 
            inside = true;              // inside the loop when compare is true        
    } else {
        return LoopInfo();
    }
    switch (compare->opr) {
        case TacOpr::Lt:
            if (index_table.count(compare->opd2) && is_invariant(compare->opd3)) {
                auto [start, step] = index_table[compare->opd2];
                return LoopInfo(TacOpr::Lt, start, compare->opd3, step);
            } else if (index_table.count(compare->opd3) && is_invariant(compare->opd2)) {
                auto [start, step] = index_table[compare->opd3];
                return LoopInfo(TacOpr::Gt, start, compare->opd2, step);
            }
        case TacOpr::Le:
        case TacOpr::Gt:
        case TacOpr::Ge:
        case TacOpr::Eq:
        case TacOpr::Ne:
        default:
            return LoopInfo();
    }
}

bool Loop::loopUnrolling(const std::set<int> &pure_funcs)
{
    for (auto chl : children)
        chl->loopUnrolling(pure_funcs);
    for (auto [opd, step] : induction_set) {
        if (header->phi.count(opd)) {
            index_table[opd] = make_pair(header->phi[opd][pre_header], step);
        }
    }
    auto info = computeLoopInfo(pure_funcs);
    // std::cerr << info.simple << " " << info.start.name() << " " << info.end.name() << " " << info.step.name() << " " << nodes.size() << std::endl;
    if (!info.simple)
        return false;
    assert(!nodes.count(pre_header));
    int line = 0;
    for (auto block : nodes) {
        if (block == header) continue;
        if (block->succ.size() > 1)
            return false;
        for (auto inst : block->insts) {
            line++;
            if (inst.opr == TacOpr::Call)
                return false;
        }
    }
    if (nodes.size() >= 5)
        return false;
    if (line >= 10) 
        return false;
    // case 1, total unrolling
    if (info.start.getType() == OpdType::Imme 
        && info.end.getType() == OpdType::Imme
        && info.step.getType() == OpdType::Imme) {
        int times = -1;
        int start = info.start.getVal();
        int end = info.end.getVal();
        int step = info.step.getVal();
        switch (info.type) {
            case TacOpr::Lt:
                if (start >= end)
                    times = 0;
                else if (start < end && step > 0) 
                    times = (end - start - 1) / step + 1; 
                break;
            case TacOpr::Gt:
                if (start <= end)
                    times = 0;
                else if (start > end && step < 0)
                    times = (start - end - 1) / (-step) + 1;
                break;
            case TacOpr::Le:
                if (start > end)
                    times = 0;
                else if (start <= end && step > 0) 
                    times = (end - start) / step + 1;
                break;
            case TacOpr::Ge:
                if (start < end)
                    times = 0;
                else if (start <= end && step < 0)
                    times = (start - end) / (-step) + 1;
                break;
            case TacOpr::Eq:
                break;
            case TacOpr::Ne:
                if (start == end)
                    times = 0;
                else if (start < end && step > 0 && (end-step) % step == 0)
                    times = (end - start) / step; 
                else if (start > end && step < 0 && (start-end) % (-step) == 0)
                    times = (start - end) / (-step);
                break;
            default:
                break;
        }
        // std::cerr << "Loop: " << start << " " << end << " " << step << " #" << times << std::endl; 
        assert(times >= -1);
        if (times <= 24) {
            // std::cerr << "Unrolling" << std::endl;
            total_unrolling(times);
            return true;
        }
    }
    return false;
}

void Loop::total_unrolling(int times)
{
    /* step 1: deal with phi from outside of the loop */ 
    vector<TacPtr> insts;
    for (auto &[base, phi_src] : header->phi) 
        insts.push_back(make_shared<Tac>(TacOpr::Mov, base, phi_src[pre_header]));
    /* step 2: copy the codes*/
    for (int i = 0; i < times; i++) {
        BBPtr fst = nullptr;
        for (auto block : header->succ)
            if (node_inside.count(block)) 
                fst = block;
        assert(fst != nullptr);
        for (auto block = fst; block != header; 
                block = *(block->succ.begin())) {
            assert (block != header);
            for (auto inst : block->insts)
                insts.push_back(make_shared<Tac>(inst));
            assert(block->succ.size() == 1);
        }
        for (auto &[base, phi_src] : header->phi) {
            int cnt = 0;
            for (auto [pre, value] : phi_src) {
                if (!nodes.count(pre)) continue;
                insts.push_back(make_shared<Tac>(TacOpr::Mov, base, value));
                cnt++;
            }
            assert(cnt == 1);
        }
    }
    /* step 3: put back to header, and change the flowgraph */
    header->phi.clear();
    auto clean_node = [](BBPtr block) {
        while (block->insts.head->succ != block->insts.tail)
            block->insts.head->succ->remove();
    };
    for (auto block : header->succ)
        if (nodes.count(block)) {
            header->succ.erase(block);
            block->pred.erase(header);
            while (block != header) {
                clean_node(block);
                block = *(block->succ.begin());
            }
            break;
        }
    for (auto block : header->pred)
        if (nodes.count(block)) {
            header->pred.erase(block);
            block->succ.erase(header);
            while (block->insts.head->succ != block->insts.tail)
                block->insts.head->succ->remove();
            break;
        }
    while (header->insts.head->succ != header->insts.tail)
        header->insts.head->succ->remove();
    for (auto inst : insts)
        header->insts.tail->pred->insert(inst);
    // std::cerr << *header << std::endl;
    renaming(header);
    // std::cerr << *header << std::endl;
}

void Loop::renaming(BBPtr bb)
{
    /* step 1: find the last definition points of the variables */ 
    map<TacOpd, TacPtr> last;
    for (auto inst = bb->insts.head->succ; 
            inst != bb->insts.tail; inst = inst->succ) {
        auto defs = inst->getDefs();
        for (auto opd : defs) 
            last[opd] = inst;
    }
    map<TacOpd, TacOpd> cur_name;
    for (auto inst = bb->insts.head->succ;
            inst != bb->insts.tail; inst = inst->succ) {
        for (int i = 1; i <= 4; i++) {
            if (inst->isUse(i)) {
                auto &opd = inst->getOpd(i);
                if (cur_name.count(opd))
                    opd = cur_name[opd];
            } 
            if (inst->isDef(i)) {
                auto &opd = inst->getOpd(i);
                if (last[opd] != inst) {
                    cur_name[opd] = TacOpd::newReg();
                    opd = cur_name[opd];
                } else {
                    cur_name[opd] = opd;
                }
            }
        }
    }
}
