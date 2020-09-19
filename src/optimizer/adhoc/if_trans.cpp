#include "adhoc.h"

static TacOpr negOpr(TacOpr opr) {
    switch (opr) {
        case TacOpr::Ge:
            return TacOpr::Lt;
        case TacOpr::Le:
            return TacOpr::Gt;
        case TacOpr::Gt:
            return TacOpr::Le;
        case TacOpr::Lt:
            return TacOpr::Ge;
        case TacOpr::Ne:
            return TacOpr::Eq;
        case TacOpr::Eq:
            return TacOpr::Ne;
        default:
            assert(false);
    }
}

static TacOpr getFalseBranchCond(TacOpr cmp, TacOpr bcond) {
    if (bcond == TacOpr::Bnez) return negOpr(cmp);
    assert(bcond == TacOpr::Beqz);
    return cmp;
}

static bool validFalseBranch(BBPtr block) {
    int counter = 0;
    for (auto inst : block->insts) {
        ++counter;
        switch (inst.opr) {
            case TacOpr::Call:
            case TacOpr::Div:
            case TacOpr::Mod:
            case TacOpr::Ret:
                return false;
            default:
                break;
        }
    }
    return counter < 7;
}

bool simple_if_trans(FlowGraph &flowgraph) {
    bool changed = false;
    for (auto bb : flowgraph.getBlocks()) {
        if (bb->succ.size() < 2) continue;
        auto tb = bb->getTrueBranch();
        auto fb = bb->getFalseBranch();
        if (fb->succ.size() > 1) continue;
        if (fb->pred.size() > 1) continue;
        if (fb->succ.size() == 1 && fb->getSucc() != tb) continue;
        if (!validFalseBranch(fb)) continue;
        changed = true;
        flowgraph.delEdge(bb, tb);
        auto cond = getFalseBranchCond(bb->insts.tail->pred->pred->opr,
                                       bb->insts.tail->pred->opr);
        auto opd1 = bb->insts.tail->pred->pred->opd2,
             opd2 = bb->insts.tail->pred->pred->opd3;
        bb->insts.tail->pred->remove();
        bb->insts.tail->pred->remove();
        bb->insts.push_back(std::make_shared<Tac>(TacOpr::CMP, opd1, opd2));
        for (auto inst = fb->insts.head->succ; inst != fb->insts.tail;
             inst = inst->succ) {
            switch (inst->opr) {
                case TacOpr::Not:
                case TacOpr::Neg:
                case TacOpr::Add:
                case TacOpr::Sub:
                case TacOpr::Mul:
                case TacOpr::Mov:
                case TacOpr::And:
                case TacOpr::Or:
                case TacOpr::Load:
                case TacOpr::LoadSpASL:
                case TacOpr::LoadSpASR:
                case TacOpr::LoadSpLSR:
                case TacOpr::LoadAdd:
                case TacOpr::LoadAddLSR:
                case TacOpr::LoadAddASL:
                case TacOpr::LoadAddASR:
                case TacOpr::Store:
                case TacOpr::Addr:
                case TacOpr::ASL:
                case TacOpr::ASR:
                case TacOpr::LSR:
                case TacOpr::MLA:
                case TacOpr::MLS:
                case TacOpr::AddLS:
                case TacOpr::SubLS:
                case TacOpr::RsbLS:
                case TacOpr::AddLSR:
                case TacOpr::RsbASR:
                case TacOpr::Smmul:
                case TacOpr::BIC:
                    inst->cond = cond;
                    break;
                case TacOpr::Gt:
                case TacOpr::Lt:
                case TacOpr::Ge:
                case TacOpr::Le:
                case TacOpr::Eq:
                case TacOpr::Ne:
                case TacOpr::Beqz:
                case TacOpr::Bnez:
                case TacOpr::Param:
                case TacOpr::Call:
                case TacOpr::Labl:
                case TacOpr::CMP:
                case TacOpr::Div:
                case TacOpr::Mod:
                case TacOpr::Ret:
                    assert(false);
                    break;
                case TacOpr::Branch:
                    break;
                default:
                    assert(false);
            }
        }
    }
    return changed;
}