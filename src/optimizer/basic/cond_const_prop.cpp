#include <queue>
#include <stdexcept>

#include "cond_const_prop.h"

namespace CondConstProp {
struct Value {
    enum {
        OVER_DEF,
        DEF_AS,
        NEVER_DEF,
    };
    int type;
    int val;

    Value() : type(NEVER_DEF) {}

    bool setOverDef() {
        if (type == OVER_DEF) return false;
        type = OVER_DEF;
        return true;
    }

    bool addValue(const Value &other) {
        switch (other.type) {
            case NEVER_DEF:
                return false;
            case DEF_AS:
                switch (type) {
                    case NEVER_DEF:
                        type = DEF_AS;
                        val = other.val;
                        return true;
                    case DEF_AS:
                        if (val != other.val) {
                            type = OVER_DEF;
                            return true;
                        }
                        return false;
                    case OVER_DEF:
                        return false;
                }
            case OVER_DEF:
                return setOverDef();
            default:
                assert(false);
        }
    }
};

}  // namespace CondConstProp

bool cond_const_prop(FlowGraph &g) {
    using namespace CondConstProp;

    std::map<BBPtr, bool> exec;
    std::map<TacOpd, Value> V;

    std::queue<BBPtr> workListBlock;
    std::queue<TacOpd> workListVar;

    auto setExec = [&](BBPtr b) -> void {
        if (exec[b] == true) return;
        exec[b] = true;
        workListBlock.push(b);
    };

    auto handlePhi = [&](BBPtr bb, const TacOpd &var) {
        auto val = V[var];
        bool raise = false;
        for (auto p : bb->phi[var]) {
            auto x = p.second;
            raise |= val.addValue(V[x]);
        }
        if (raise) {
            V[var] = val;
            workListVar.push(var);
        }
    };

    auto handleInst = [&](const Tac &inst) {
        switch (inst.opr) {
            case TacOpr::Not:
            case TacOpr::Neg:
            case TacOpr::Mov:
            case TacOpr::Addr:
                if (V[inst.opd1].addValue(V[inst.opd2]))
                    workListVar.push(inst.opd1);
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
                if (V[inst.opd1].addValue(V[inst.opd2]) ||
                    V[inst.opd1].addValue(V[inst.opd3])) {
                    workListVar.push(inst.opd1);
                }
                break;
            case TacOpr::Call:
                if (inst.opd2.getType() == OpdType::Reg) {
                    if (V[inst.opd2].setOverDef()) workListVar.push(inst.opd2);
                }
            case TacOpr::Load:
                if (V[inst.opd1].setOverDef()) workListVar.push(inst.opd1);
            default:
                break;
        }
    };

    auto handlePos = [&](PosPtr pos) {
        if (pos->isPhi()) {
            handlePhi(pos->getBB(), pos->getPhiName());
        } else {
            handleInst(*(pos->getTac()));
        }
    };

    for (auto v : g.vars) {
        try {
            g.def.at(v);
        } catch (std::out_of_range) {
            V[v].setOverDef();
            workListVar.push(v);
        }
    }
    exec[g.getStartBlock()] = true;
    workListBlock.push(g.getStartBlock());

    while (!workListBlock.empty() || !workListVar.empty()) {
        while (!workListBlock.empty()) {
            auto b = workListBlock.front();
            workListBlock.pop();
            for (auto p : b->phi) handlePhi(b, p.first);
            for (auto inst : b->insts) handleInst(inst);
            if (b->succ.size() == 0) continue;
            if (b->succ.size() == 1) {
                setExec(*(b->succ.begin()));
                continue;
            }
            assert(b->succ.size() == 2);
            assert(b->getTrueBranch() != nullptr);
            auto cond = b->insts.tail->pred;
            assert(cond->opr == TacOpr::Beqz || cond->opr == TacOpr::Bnez);
            assert(cond->opd1.getType() == OpdType::Reg);
            auto val = V[cond->opd1];
            switch (val.type) {
                case Value::OVER_DEF:
                    setExec(b->getTrueBranch());
                    setExec(b->getFalseBranch());
                    break;
                case Value::DEF_AS:
                    if (cond->opr == TacOpr::Beqz && val.val == 0 ||
                        cond->opr == TacOpr::Bnez && val.val != 0) {
                        setExec(b->getTrueBranch());
                    } else {
                        setExec(b->getFalseBranch());
                    }
                    break;
                case Value::NEVER_DEF:
                    assert(false);
            }
        }
        while (!workListVar.empty()) {
            auto v = workListVar.front();
            workListVar.pop();
            for (auto pos : g.uses[v]) {
                if (exec[pos->getBB()]) handlePos(pos);
            }
        }
    }

    std::vector<TacOpd> toChange;

    for (auto v : g.vars) {
        if (V[v].type == Value::DEF_AS) toChange.push_back(v);
    }
    for (auto v : toChange) g.vars.erase(v);
    for (auto v : toChange) {
        int val = V[v].val;
        for (auto pos : g.uses[v]) {
            if (pos->isPhi()) {
                for (auto &p : pos->getBB()->phi[pos->getPhiName()]) {
                    if (p.second == v) p.second = TacOpd::newImme(val);
                }
            } else {
                auto inst = pos->getTac();
                for (int i = 0; i < 3; ++i) {
                    auto &opd = inst->getOpd(i);
                    if (opd == v) opd = TacOpd::newImme(val);
                }
            }
        }
        g.uses.erase(v);
    }
    for (auto v : toChange) {
        auto pos = g.def[v];
        if (pos->isPhi()) {
            pos->getBB()->phi.erase(v);
        } else {
            pos->getTac()->remove();
        }
        g.def.erase(v);
    }

    bool changed = false;
    auto blocks = g.getBlocks();
    for (auto b : blocks) {
        if (!exec[b]) {
            changed = true;
            g.removeSingle(b);
        }
    }

    return toChange.size() > 0 || changed;
}
