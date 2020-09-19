#include <memory>
#include "gen.h"
#include "help.h"
#include "gen_unit/gen_unit.h"

TacOpr revOpr(TacOpr opr) {
    switch (opr) {
        case TacOpr::Ge: return TacOpr::Le;
        case TacOpr::Le: return TacOpr::Ge;
        case TacOpr::Lt: return TacOpr::Gt;
        case TacOpr::Gt: return TacOpr::Lt;
        case TacOpr::Ne: return TacOpr::Ne;
        case TacOpr::Eq: return TacOpr::Eq;
        default: assert(false);
    }
}

static TacOpr negOpr(TacOpr opr) {
    switch (opr) {
        case TacOpr::Ge: return TacOpr::Lt;
        case TacOpr::Le: return TacOpr::Gt;
        case TacOpr::Gt: return TacOpr::Le;
        case TacOpr::Lt: return TacOpr::Ge;
        case TacOpr::Ne: return TacOpr::Eq;
        case TacOpr::Eq: return TacOpr::Ne;
        default: assert(false);
    }
}

string nameOpr(TacOpr opr) {
    switch (opr) {
        case TacOpr::Ge: return "ge";
        case TacOpr::Le: return "le";
        case TacOpr::Gt: return "gt";
        case TacOpr::Lt: return "lt";
        case TacOpr::Ne: return "ne";
        case TacOpr::Eq: return "eq";
        default: assert(false);
    }
}

void genCondBranch(Lines &prog, const Tac &condition, const Tac &branch, const std::map<TacOpd, int> &reg) {
    auto opr = condition.opr;
    auto lhs = condition.opd2, rhs = condition.opd3;
    if (lhs.getType() == OpdType::Imme) {
        std::swap(lhs, rhs);
        opr = revOpr(opr);
    }
    assert(lhs.getType() == OpdType::Reg);
    if (branch.opr == TacOpr::Beqz) opr = negOpr(opr);
    prog.push_back(gen_cmp("", toOperand2(lhs, reg), toOperand2(rhs, reg)));
    prog.push_back(gen_b(nameOpr(opr), label(branch.opd2.getId())));
}

void genBranch(Lines &prog, const Tac &branch, const std::map<TacOpd, int> &reg) {
    assert(branch.opr == TacOpr::Branch);
    prog.push_back(gen_b("", label(branch.opd1.getId())));
}

void genLabel(Lines &prog, const Tac &tac) {
    assert(tac.opr == TacOpr::Labl);
    prog.push_back(gen_label(label(tac.opd1.getId())));
}

void genCMP(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
    assert(tac.opr == TacOpr::CMP);
    assert(tac.opd1.getType() == OpdType::Reg);
    auto Rn = toOperand2(tac.opd1, reg);
    auto op2 = toOperand2(tac.opd2, reg);
    prog.push_back(gen_cmp("", Rn, op2));
}