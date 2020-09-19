#include "../env/env.h"
#include "../util/setopr.h"
#include "flowgraph.h"
#include "../optimizer/loop/loop.h"

static const double INF_BASE = 1e50;
static const double INF_DEL = 1e10;

static int K;
static std::set<TacOpd> precolored, initial;
static std::set<std::pair<double, TacOpd>> spillWorklist;
static std::set<TacOpd> freezeWorklist, simplifyWorklist;
static std::set<TacOpd> spilledNodes, coalescedNodes, coloredNodes;
static std::set<TacOpd> toPreLoad;
static std::set<TacPtr> toRelocate;
static std::map<TacOpd, double> frequency;

class {
    std::vector<TacOpd> stack;

public:
    auto begin() { return stack.begin(); }
    auto end() { return stack.end(); }
    auto begin() const { return stack.begin(); }
    auto end() const { return stack.end(); }
    void push(const TacOpd &a) { stack.push_back(a); }
    auto pop() {
        auto result = stack.back();
        stack.pop_back();
        return result;
    }
    auto size() { return stack.size(); }
    auto empty() { return stack.empty(); }
} selectStack;

struct Move {
    BBPtr block;
    TacPtr inst;
    Move(BBPtr block, TacPtr inst) : block(block), inst(inst) {}
    bool operator<(const Move &other) const { return inst < other.inst; }
};

static std::set<Move> coalescedMoves, constrainedMoves;
static std::set<Move> frozenMoves, worklistMoves, activeMoves;

static std::set<std::pair<TacOpd, TacOpd>> adjSet;
static std::map<TacOpd, std::set<TacOpd>> adjList;
static std::map<TacOpd, int> degree;
static std::map<TacOpd, std::set<Move>> moveList;
static std::map<TacOpd, TacOpd> alias;
static std::map<TacOpd, int> color;

static std::map<TacOpd, int> addr;
static int stackTop, delta;

static auto createNew(const TacOpd &v) {
    assert(v.getType() == OpdType::Reg);
    static std::map<int, int> counter;
    int id = ++counter[v.getId()];
    auto newopd = TacOpd(v.getId() | (id << SPILL_SUB_BASE), OpdType::Reg);
    frequency[newopd] = INF_BASE + id * INF_DEL;
    return newopd;
}

static void clearGraph() {
    adjSet.clear();
    adjList.clear();
    degree.clear();
}

static void init() {
    delta = 0;
    toPreLoad.clear();
    toRelocate.clear();
    initial.clear();
    precolored.clear();
    spilledNodes.clear();
    coalescedNodes.clear();
    coloredNodes.clear();
    coalescedMoves.clear();
    constrainedMoves.clear();
    frozenMoves.clear();
    worklistMoves.clear();
    activeMoves.clear();
    moveList.clear();
    alias.clear();
    color.clear();
    addr.clear();
    clearGraph();
}

static auto adjacent(const TacOpd &n) {
    assert(n.getType() == OpdType::Reg);
    auto result = adjList[n];
    for (auto a : selectStack) result.erase(a);
    for (auto a : coalescedNodes) result.erase(a);
    return result;
}

static void alloc(const TacOpd &opd) {
    assert(opd.getType() == OpdType::Reg);
    addr[opd] = (stackTop++) * WORD_SIZE;
    ++delta;
}

static bool is_valid_offset(int offset) {
    return -4096 < offset && offset < 4096;
}

static auto nodeMoves(const TacOpd &n) {
    assert(n.getType() == OpdType::Reg);
    return intersection_set(moveList[n], activeMoves + worklistMoves);
}

static bool moveRelated(const TacOpd &n) {
    assert(n.getType() == OpdType::Reg);
    return !nodeMoves(n).empty();
}

static void addEdge(const TacOpd &a, const TacOpd &b) {
    assert(a.getType() == OpdType::Reg);
    assert(b.getType() == OpdType::Reg);
    if (a == b || adjSet.find(std::make_pair(a, b)) != adjSet.end()) return;
    adjSet.insert(std::make_pair(a, b));
    adjSet.insert(std::make_pair(b, a));
    if (precolored.find(a) == precolored.end()) {
        adjList[a].insert(b);
        ++degree[a];
    }
    if (precolored.find(b) == precolored.end()) {
        adjList[b].insert(a);
        ++degree[b];
    }
}

static void build(FlowGraph &g) {
    for (auto b : g.getBlocks()) {
        auto live = b->getLiveOut();
        for (auto inst = b->insts.tail->pred; inst != b->insts.head;
             inst = inst->pred) {
            if (inst->opr == TacOpr::Mov && inst->opd2.getType() == OpdType::Reg) {
                live.erase(inst->opd2);
                auto mv = Move(b, inst);
                moveList[inst->opd1].insert(mv);
                moveList[inst->opd2].insert(mv);
                worklistMoves.insert(mv);
            }
            for (int i = 1; i <= 4; ++i)
                if (inst->isDef(i)) live.insert(inst->getOpd(i));
            for (int i = 1; i <= 4; ++i) {
                if (inst->isDef(i)) {
                    for (auto l : live) {
                        assert(l.getType() == OpdType::Reg);
                        addEdge(inst->getOpd(i), l);
                    }
                }
            }
            for (int i = 1; i <= 4; ++i)
                if (inst->isDef(i)) live.erase(inst->getOpd(i));
            for (int i = 1; i <= 4; ++i)
                if (inst->isUse(i)) live.insert(inst->getOpd(i));
        }
        if (b == g.getStartBlock()) {
            for (auto l1 : live) {
                for (auto l2 : live) {
                    addEdge(l1, l2);
                }
            }
        }
    }
}

static void makeWorkList() {
    for (auto n : initial) {
        assert(n.getType() == OpdType::Reg);
        if (degree[n] >= K) {
            spillWorklist.insert(std::make_pair(frequency[n], n));
        } else if (moveRelated(n)) {
            freezeWorklist.insert(n);
        } else {
            simplifyWorklist.insert(n);
        }
    }
    initial.clear();
}

void enableMoves(const std::set<TacOpd> &nodes) {
    for (auto n : nodes) {
        assert(n.getType() == OpdType::Reg);
        for (auto m : nodeMoves(n)) {
            if (activeMoves.find(m) != activeMoves.end()) {
                activeMoves.erase(m);
                worklistMoves.insert(m);
            }
        }
    }
}

static void decrementDegree(const TacOpd &m) {
    assert(m.getType() == OpdType::Reg);
    auto d = degree[m]--;
    if (d == K) {
        auto temp = adjacent(m);
        temp.insert(m);
        enableMoves(temp);
        spillWorklist.erase(std::make_pair(frequency[m], m));
        if (moveRelated(m)) {
            freezeWorklist.insert(m);
        } else {
            simplifyWorklist.insert(m);
        }
    }
}

static void simplify() {
    auto n = *simplifyWorklist.begin();
    assert(n.getType() == OpdType::Reg);
    simplifyWorklist.erase(simplifyWorklist.begin());
    selectStack.push(n);
    for (auto m : adjacent(n)) decrementDegree(m);
}

static TacOpd getAlias(const TacOpd &n) {
    assert(n.getType() == OpdType::Reg);
    if (coalescedNodes.find(n) != coalescedNodes.end()) {
        return getAlias(alias[n]);
    }
    return n;
}

static void addWorkList(const TacOpd &u) {
    assert(u.getType() == OpdType::Reg);
    if (precolored.find(u) == precolored.end() && !moveRelated(u) && degree[u] < K) {
        freezeWorklist.erase(u);
        simplifyWorklist.insert(u);
    }
}

static bool Okay(const TacOpd &t, const TacOpd &r) {
    return degree[t] < K || precolored.find(t) != precolored.end() ||
           adjSet.find(std::make_pair(t, r)) != adjSet.end();
}

static bool conservative(const std::set<TacOpd> &nodes) {
    int k = 0;
    for (auto n : nodes) k += (degree[n] >= K);
    return k < K;
}

static void combine(const TacOpd &u, const TacOpd &v) {
    if (freezeWorklist.find(v) != freezeWorklist.end()) {
        freezeWorklist.erase(v);
    } else {
        spillWorklist.erase(std::make_pair(frequency[v], v));
    }
    coalescedNodes.insert(v);
    alias[v] = u;
    for (auto I : moveList[v]) moveList[u].insert(I);
    for (auto t : adjacent(v)) {
        addEdge(t, u);
        decrementDegree(t);
    }
    if (degree[u] >= K && freezeWorklist.find(u) != freezeWorklist.end()) {
        freezeWorklist.erase(u);
        spillWorklist.insert(std::make_pair(frequency[u], u));
    }
}

static void coalesce() {
    auto m = *worklistMoves.begin();
    worklistMoves.erase(worklistMoves.begin());
    auto x = getAlias(m.inst->opd1), y = getAlias(m.inst->opd2);
    auto pr = precolored.find(y) != precolored.end() ? std::make_pair(y, x)
                                                     : std::make_pair(x, y);
    auto [u, v] = pr;
    if (u == v) {
        coalescedMoves.insert(m);
        addWorkList(u);
        return;
    }
    if (precolored.find(v) != precolored.end() || adjSet.find(pr) != adjSet.end()) {
        constrainedMoves.insert(m);
        addWorkList(u);
        addWorkList(v);
        return;
    }
    bool cond;
    if (precolored.find(u) != precolored.end()) {
        cond = true;
        for (auto t : adjacent(v))
            if (!Okay(t, u)) {
                cond = false;
                break;
            }
    } else {
        cond = conservative(adjacent(u) + adjacent(v));
    }
    if (cond) {
        coalescedMoves.insert(m);
        combine(u, v);
        addWorkList(u);
    } else {
        activeMoves.insert(m);
    }
}

static void freezeMoves(const TacOpd &u) {
    for (auto m : nodeMoves(u)) {
        auto x = m.inst->opd1, y = m.inst->opd2;
        TacOpd v;
        if (getAlias(y) == getAlias(u)) {
            v = getAlias(x);
        } else {
            v = getAlias(y);
        }
        activeMoves.erase(m);
        frozenMoves.insert(m);
        if (precolored.find(v) == precolored.end() && nodeMoves(v).empty() && degree[v] < K) {
            freezeWorklist.erase(v);
            simplifyWorklist.insert(v);
        }
    }
}

static void freeze() {
    auto u = *freezeWorklist.begin();
    freezeWorklist.erase(u);
    simplifyWorklist.insert(u);
    freezeMoves(u);
}

static void selectSpill() {
    auto [freq, m] = *spillWorklist.begin();
    spillWorklist.erase(spillWorklist.begin());
    simplifyWorklist.insert(m);
    freezeMoves(m);
}

static int selectColor(const std::set<int> &colors) {
    const static int order[14] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    for (int i = 0; i < K; ++i) {
        if (colors.count(order[i])) return order[i];
    }
    assert(false);
}

static void assignColors() {
    while (!selectStack.empty()) {
        auto n = selectStack.pop();
        std::set<int> okColors;
        for (int i = 0; i < K; ++i) okColors.insert(i);
        for (auto w : adjList[n]) {
            auto aw = getAlias(w);
            if (coloredNodes.find(aw) != coloredNodes.end() ||
                precolored.find(aw) != precolored.end()) {
                okColors.erase(color[aw]);
            }
        }
        if (okColors.empty()) {
            spilledNodes.insert(n);
        } else {
            coloredNodes.insert(n);
            color[n] = selectColor(okColors);
        }
    }
    for (auto n : coalescedNodes) color[n] = color[getAlias(n)];
}

static void insertStackLoad(TacPtr pre, const TacOpd &opd, int offset) {
    if (is_valid_offset(offset)) {
        pre->insert(std::make_shared<Tac>(TacOpr::Load, opd, TacOpd::newImme(offset), TacOpd::newImme(STACK_SPACE_ID)));
    } else {
        pre->insert(std::make_shared<Tac>(TacOpr::Load, opd, opd, TacOpd::newImme(STACK_SPACE_ID)));
        pre->insert(std::make_shared<Tac>(TacOpr::Mov, opd, TacOpd::newImme(offset)));
    }
}

static TacOpd insertStackStore(TacPtr pre, const TacOpd &opd, int offset) {
    if (is_valid_offset(offset)) {
        pre->insert(std::make_shared<Tac>(TacOpr::Store, opd, TacOpd::newImme(offset), TacOpd::newImme(STACK_SPACE_ID)));
        return TacOpd();
    } else {
        auto temp = TacOpd::newReg();
        frequency[temp] = INF_BASE;
        pre->insert(std::make_shared<Tac>(TacOpr::Store, opd, temp, TacOpd::newImme(STACK_SPACE_ID)));
        pre->insert(std::make_shared<Tac>(TacOpr::Mov, temp, TacOpd::newImme(offset)));
        return temp;
    }
}

static void rewriteProgram(FlowGraph &g) {
    for (auto v : spilledNodes) alloc(v);
    std::set<TacOpd> newTemps;
    for (auto block : g.getBlocks()) {
        for (auto inst = block->insts.head->succ; inst != block->insts.tail;
             inst = inst->succ) {
            bool insertAfter = false;
            for (int i = 1; i <= 4; ++i) {
                if (inst->getOpd(i).getType() != OpdType::Reg) continue;
                bool appeared = false;
                for (int j = 1; j < i; ++j)
                    appeared = appeared || inst->getOpd(i) == inst->getOpd(j);
                if (appeared) continue;
                if (spilledNodes.find(inst->getOpd(i)) == spilledNodes.end())
                    continue;
                auto oldopd = inst->getOpd(i);
                auto newopd = createNew(oldopd);
                newTemps.insert(newopd);
                bool use = false, def = false;
                for (int j = 1; j <= 4; ++j) {
                    if (inst->getOpd(j) == oldopd) {
                        inst->getOpd(j) = newopd;
                        use = use || inst->isUse(j);
                        def = def || inst->isDef(j);
                    }
                }
                if (use) {
                    insertStackLoad(inst->pred, newopd, addr[oldopd]);
                }
                if (def) {
                    auto temp = insertStackStore(inst, newopd, addr[oldopd]);
                    if (!temp.empty()) newTemps.insert(temp);
                    insertAfter = true;
                }
            }
            if (insertAfter) inst = inst->succ;
        }
    }
    spilledNodes.clear();
    initial = coloredNodes + coalescedNodes + newTemps;
    coloredNodes.clear();
    coalescedNodes.clear();
    clearGraph();
}

void coloringMain(FlowGraph &g) {
    g.computeLiveness();
    build(g);
    makeWorkList();
    do {
        if (!simplifyWorklist.empty())
            simplify();
        else if (!worklistMoves.empty())
            coalesce();
        else if (!freezeWorklist.empty())
            freeze();
        else if (!spillWorklist.empty())
            selectSpill();
    } while (!simplifyWorklist.empty() || !worklistMoves.empty() ||
             !freezeWorklist.empty() || !spillWorklist.empty());
    assignColors();
    if (!spilledNodes.empty()) {
        rewriteProgram(g);
        coloringMain(g);
    }
}

void FlowGraph::graphColoring() {
    stackTop = func->length;
    init();
    K = 14;
/*    for (auto bb : blocks) {
        for (auto inst : bb->insts) {
            if (inst.opr == TacOpr::Call) {
                K = 13;
                break;
            }
        }
        if (K == 13) break;
    }*/
    for (int i = 4, pos = 0; i < func->paramId.size(); ++i, pos += WORD_SIZE) {
        auto parm = TacOpd(func->paramId[i], OpdType::Reg);
        auto offset = pos + (stackTop + CALLEE_REGISTERS) * WORD_SIZE;
        insertStackLoad(startBlock->insts.head, parm, offset);
        toRelocate.insert(startBlock->insts.head->succ);
    }
    frequency = get_frequency(*this);
    for (int i = 0; i < 4; ++i) {
        if (i >= func->paramId.size()) break;
        auto parm = TacOpd(func->paramId[i], OpdType::Reg);
        precolored.insert(parm);
        color[parm] = i;
    }
    calcVars();
    for (auto v : vars)
        if (color.find(v) == color.end()) initial.insert(v);
    coloringMain(*this);
    func->length = stackTop;
    func->reg = color;
    for (auto inst : toRelocate) {
        switch (inst->opr) {
            case TacOpr::Load:
            case TacOpr::Mov:
                inst->opd2 = TacOpd::newImme(inst->opd2.getVal() + delta * WORD_SIZE);
                break;
            default:
                assert(false);
        }
    }
}
