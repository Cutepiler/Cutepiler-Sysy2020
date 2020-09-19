#include "inst_merging.h"
#include "../../flowgraph/flowgraph.h"
#include <algorithm>
#include <map>

using std::vector;
using std::list;
using std::make_shared;
using std::make_pair;
using std::map;
using std::set;
using std::pair;

vector<MatcherPtr> matchers;

static TreePtr get_tree(TacOpd opd, map<TacOpd, pair<TacPtr, TreePtr>> &inst_map, const set<TacOpd> &tmp_var)
{
    TreePtr tree;
    if (!inst_map.count(opd) || !tmp_var.count(opd) 
        || (inst_map[opd].first != nullptr && inst_map[opd].first->opr == TacOpr::Load)) {
        tree = make_shared<TreeNode>(opd, Single);
    } else {
        tree = inst_map[opd].second;
        inst_map[opd].second = nullptr;
        assert(tree != nullptr);
    }
    return tree;
}

static TreePtr build_tree(TacPtr inst, map<TacOpd, pair<TacPtr, TreePtr>> &inst_map, const set<TacOpd> &tmp_var)
{
    TreePtr res;
    switch (inst->opr) {
        case TacOpr::Add:
        case TacOpr::Sub:
        case TacOpr::Mul:
        case TacOpr::ASL:
        case TacOpr::ASR:
        case TacOpr::LSR:
        case TacOpr::Load:
            if (inst_map.count(inst->opd1))
                return nullptr;
            res = make_shared<TreeNode>(inst->opd1, inst->opr);
            res->lc = get_tree(inst->opd2, inst_map, tmp_var);
            res->rc = get_tree(inst->opd3, inst_map, tmp_var);
            return res;
        default: 
            return nullptr;
    }
}

void inst_merge(FlowGraph &flowgraph, std::function<SpaceType(int)> fbase)
{
    register_matchers(fbase);
    /* step 1: compute temporary variables, i.e. def[v] = use[v] = 1 */ 
    auto blocks = flowgraph.getBlocks();
    map<TacOpd, int> def_map, use_map;
    set<TacOpd> tmp_var;
    for (auto block : blocks) {
        for (auto inst = block->insts.head->succ; 
                inst != block->insts.tail; inst = inst->succ) {
            auto defs = inst->getDefs(), uses = inst->getUses();
            for (auto def : defs)
                def_map[def]++;
            for (auto use : uses)
                use_map[use]++;
        }
    }
    for (auto [var, times] : def_map) {
        if (times == 1 && use_map[var] == 1)
            tmp_var.insert(var);
    }

    for (auto block : blocks) {
        /* step 2: construct tree inside basicblock, for temporary vars */
        map<TacOpd, pair<TacPtr, TreePtr>> inst_map;
        for (auto inst = block->insts.head->succ; 
                inst != block->insts.tail; inst = inst->succ) {
            auto res = build_tree(inst, inst_map, tmp_var);
            if (res != nullptr) {
                inst_map[res->des] = make_pair(inst, res);
            }
        }
        /* step 3: generate code using dynamic programming algorithm */
        for (auto [opd, val] : inst_map) {
            auto [inst_origin, tree] = val;
            auto insts = gen_code(tree);
            auto cur = inst_origin;
            for (auto inst : insts) {
                cur->insert(inst);
                cur = cur->succ;
            }
            inst_origin->remove();
        }
    } 
}

void register_matchers(std::function<SpaceType(int)> fbase)
{ 
    matchers.clear();
    matchers.push_back(make_shared<SingleMatcher>());
    matchers.push_back(make_shared<BinaryMatcher>(TacOpr::Add, 1));
    matchers.push_back(make_shared<BinaryMatcher>(TacOpr::Sub, 1));
    matchers.push_back(make_shared<BinaryMatcher>(TacOpr::Mul, 3));
    matchers.push_back(make_shared<BinaryMatcher>(TacOpr::ASL, 1));
    matchers.push_back(make_shared<BinaryMatcher>(TacOpr::ASR, 1));
    matchers.push_back(make_shared<BinaryMatcher>(TacOpr::LSR, 1));
    matchers.push_back(make_shared<BinaryMatcher>(TacOpr::Load, 4));
    matchers.push_back(make_shared<MLAMatcher>(true));
    matchers.push_back(make_shared<MLAMatcher>(false));
    matchers.push_back(make_shared<MLSMatcher>());
    matchers.push_back(make_shared<AddLSMatcher>(true));
    matchers.push_back(make_shared<AddLSMatcher>(false));
    matchers.push_back(make_shared<LoadSpMatcher>(fbase));
    matchers.push_back(make_shared<LoadAddMatcher>(fbase));
    matchers.push_back(make_shared<LoadAddShiftMatcher>(fbase, true));
    matchers.push_back(make_shared<LoadAddShiftMatcher>(fbase, false));
}

void dfs_compute_cost(TreePtr tree)
{
    tree->cost = -1;
    tree->matcher = nullptr;
    if (tree->lc) dfs_compute_cost(tree->lc);
    if (tree->rc) dfs_compute_cost(tree->rc);
    for (auto matcher : matchers) {
        auto [chl, inst, cost] = matcher->match(tree);
        if (cost == -1) continue;
        for (auto subtree : chl)
            cost = cost + subtree->cost;
        if (tree->cost == -1 || cost < tree->cost) {
            tree->cost = cost;
            tree->matcher = matcher;
        }
    }
    assert(tree->cost != -1);
    assert(tree->matcher != nullptr);
}

void dfs_gen_code(TreePtr tree, vector<TacPtr> &inst_list)
{
    assert(tree != nullptr);
    assert(tree->matcher != nullptr);
    auto [chl, inst, cost] = tree->matcher->match(tree);
    for (auto subtree : chl) {
        dfs_gen_code(subtree, inst_list);
    }
    if (inst != nullptr)
        inst_list.push_back(inst);
}

vector<TacPtr> gen_code(TreePtr tree)
{
    vector<TacPtr> insts;
    if (tree == nullptr)
        return insts;
    dfs_compute_cost(tree);
    dfs_gen_code(tree, insts);
    return insts;
}
