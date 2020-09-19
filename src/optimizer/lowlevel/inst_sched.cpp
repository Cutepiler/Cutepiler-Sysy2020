#include "../../tac/tac.h"
#include "../../flowgraph/flowgraph.h"
#include "str_reduction.h"
#include <set>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <iostream>

using std::cerr;
using std::endl;
using std::set;
using std::pair;
using std::map;
using std::vector;
using std::queue;
using std::function;
using std::max;

using uint = unsigned int;

struct res_t {
    uint integer : 2; 
    uint load : 1;
    uint store : 1;
    uint branch : 1;
    uint multi : 1;
    uint delay : 12;
};

static res_t get_res(TacPtr tac)
{
    res_t res = {0, 0, 0, 0, 0, 0};
    switch (tac->opr) {
        case TacOpr::Add:
        case TacOpr::Sub:
        case TacOpr::BIC:
        case TacOpr::Neg:
        case TacOpr::Eq:
        case TacOpr::Ne:
        case TacOpr::Le:
        case TacOpr::Lt:
        case TacOpr::Ge:
        case TacOpr::Gt:
            res.integer = 1;
            res.delay = 1;
            break;
        case TacOpr::AddLS:
        case TacOpr::AddLSR:
        case TacOpr::RsbLS:
        case TacOpr::RsbASR:
        case TacOpr::SubLS:
            res.multi = 1;
            res.delay = 2;
            break;
        case TacOpr::MLA:
        case TacOpr::Smmul:
        case TacOpr::MLS:
        case TacOpr::Mul:
            res.multi = 1;
            res.delay = 3;
            break;
        case TacOpr::ASL:
        case TacOpr::ASR:
        case TacOpr::LSR:
        case TacOpr::Mov:
            res.integer = 1;
            res.delay = 1;
            break;
        case TacOpr::Div:
        case TacOpr::Mod:
        case TacOpr::Call:
            res.branch = 1;
            res.integer = 2;
            res.delay = 1024;
            break;
        case TacOpr::Beqz:
        case TacOpr::Bnez:
        case TacOpr::Branch:
        case TacOpr::Ret:
            res.branch = 1;
            res.delay = 1;
            break;
        case TacOpr::Load:
        case TacOpr::Addr:
        case TacOpr::LoadSpASL:
        case TacOpr::LoadSpASR:
        case TacOpr::LoadSpLSR:
        case TacOpr::LoadAdd:
        case TacOpr::LoadAddLSR:
        case TacOpr::LoadAddASR:
        case TacOpr::LoadAddASL:
            res.load = 1;
            res.delay = 4;
            break;
        case TacOpr::Store:
        case TacOpr::Param:
            res.store = 1;
            res.delay = 1;
            break;
        case TacOpr::Labl: break;
        default:
            cerr << tac->to_string() << endl;
            assert(false);
    }
    return res;
}

static int delay_race(TacPtr pta, TacPtr ptb)
{
    auto a = get_res(pta), b = get_res(ptb);
    int race = 0;
    if (a.integer + b.integer >= 2)
        race = a.delay;
    if (a.load + b.load >= 1 || a.store + b.store >= 1)
        race = a.delay;
    if (a.branch + b.branch >= 1)
        race = a.delay;
    if (a.multi + b.multi >= 1)
        race = a.delay;
    auto defa = pta->getDefs(), useb = ptb->getUses();
    for (auto u : defa)
        if (useb.count(u)) {
            race = a.delay + 1;
            break;
        }
    return race;
}

static bool interference(TacPtr inst1, TacPtr inst2, function<SpaceType(int)> fbase)
{
    auto def1 = inst1->getDefs(), use1 = inst1->getUses();
    auto def2 = inst2->getDefs(), use2 = inst2->getUses();
    /* writer / reader */
    for (auto d : def1)
        if (use2.count(d))
            return true; 
    /* writer / writer */
    for (auto d : def1)
        if (def2.count(d)) 
            return true;
    /* reader / writer */
    for (auto u : use1)
        if (def2.count(u))
            return true;
    if (inst1->opr == TacOpr::Store) {
        if (inst2->opr == TacOpr::Load || inst2->opr == TacOpr::Store)
            return fbase(inst1->opd3.getVal()) == SpaceType::Abs 
                    || fbase(inst2->opd3.getVal()) == SpaceType::Abs 
                    || inst1->opd3.getVal() == inst2->opd3.getVal();
    } else if (inst1->opr == TacOpr::Load) {
        if (inst2->opr == TacOpr::Store)
            return fbase(inst1->opd3.getVal()) == SpaceType::Abs 
                    || fbase(inst2->opd3.getVal()) == SpaceType::Abs 
                    || inst1->opd3.getVal() == inst2->opd3.getVal();
    }
    if (inst1->opr == TacOpr::Store) {
        switch (inst2->opr) {
            case TacOpr::LoadSpASL:
            case TacOpr::LoadSpASR:
            case TacOpr::LoadSpLSR:
            case TacOpr::LoadAdd:
            case TacOpr::LoadAddASL:
            case TacOpr::LoadAddASR:
            case TacOpr::LoadAddLSR:
                return true;
            default: break;
        }
    }
    if (inst2->opr == TacOpr::Store) {
        switch (inst1->opr) {
            case TacOpr::LoadSpASL:
            case TacOpr::LoadSpASR:
            case TacOpr::LoadSpLSR:
            case TacOpr::LoadAdd:
            case TacOpr::LoadAddASL:
            case TacOpr::LoadAddASR:
            case TacOpr::LoadAddLSR:
                return true;
            default: break;
        }
    }
    /* function call */
    if (inst1->opr == TacOpr::Param || inst2->opr == TacOpr::Param)
        return true;
    if (inst1->opr == TacOpr::Call || inst2->opr == TacOpr::Call)
        return true;
    /* function return */
    if (inst1->opr == TacOpr::Ret || inst2->opr == TacOpr::Ret)
        return true;
    return false;
}

static bool is_computation(TacPtr inst) {
    switch (inst->opr) {
        case TacOpr::Branch:
        case TacOpr::Bnez:
        case TacOpr::Beqz:
        case TacOpr::Labl:
        case TacOpr::Le:
        case TacOpr::Lt:
        case TacOpr::Ge:
        case TacOpr::Gt:
        case TacOpr::Eq:
        case TacOpr::Ne:
            return false;
        default:
            return true;
    }
}

void schedule(BBPtr bb, function<SpaceType(int)> fbase)
{
    /* pre-processing */
    vector<TacPtr> inst_vec;
    vector<TacPtr> result;
    TacPtr fst = nullptr;
    map<TacPtr, int> place;
    int k = 0;
    for (auto inst = bb->insts.head->succ;
            inst != bb->insts.tail; inst = inst->succ) {
        if (is_computation(inst)) {
            if (fst == nullptr)
                fst = inst->pred;
            inst_vec.push_back(inst);
            place[inst] = k++;
        }
    }

    if (inst_vec.size() > 40) // pian men
        return;
    
    for (auto inst : inst_vec) 
        inst->remove();

    /* scheduling algorithm */
    map<TacPtr, vector<TacPtr>> graph;
    map<TacPtr, int> degree;
    for (auto u : inst_vec)
        for (auto v : inst_vec)
            if (place[u] < place[v] && interference(u, v, fbase)) {
                graph[u].push_back(v);
                degree[v]++;
            }
    map<TacPtr, int> candidate;
    for (auto inst : inst_vec)
        if (degree[inst] == 0)
            candidate[inst] = 0;
    while (!candidate.empty()) {
        int min_delay = 2048;
        TacPtr ptr = nullptr;
        /* select next instruction with minimum delay */
        for (auto [inst, delay] : candidate) {
            if (delay < min_delay) {
                ptr = inst;
                min_delay = delay;
            }
        }
        assert(ptr != nullptr);
        /* update delay */ 
        result.push_back(ptr);
        candidate.erase(ptr);
        for (auto &[inst, delay] : candidate) {
            delay = max(delay - min_delay, delay_race(ptr, inst));
        }
        /* deal with dependency */ 
        for (auto nxt : graph[ptr]) {
            degree[nxt]--;
            if (degree[nxt] == 0) {
                candidate[nxt] = max(candidate[nxt], min_delay);
            }
        }
    }
    for (auto inst : result) {
        fst->insert(inst);
        fst = inst;
    }
}

void schedule(FlowGraph &flowgraph)
{
    auto blocks = flowgraph.getBlocks();
    for (auto block : blocks) {
        schedule(block, flowgraph.fbase);
    }
}
