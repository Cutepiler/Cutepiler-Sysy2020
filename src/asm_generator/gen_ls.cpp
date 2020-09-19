#include <memory>
#include "gen.h"
#include "help.h"

/*
Restraint:
    Abs: opd2 must be register
    Stack: if opd2 is immediate, it must be in [-4095, 4095]
    Other space types are not permitted
*/
void genLoad(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg, TBase fbase, int spOffset) {
    assert(tac.opr == TacOpr::Load);
    auto space = fbase(tac.opd3.getVal());
    auto Rt = toOperand2(tac.opd1, reg);
    auto offset = toOperand2(tac.opd2, reg);
    auto cond = get_cond_code(tac.cond);
    switch (space) {
        case SpaceType::Abs:
            assert(tac.opd2.getType() == OpdType::Reg);
            prog.push_back(gen_ldr(cond, Rt, offset, 0));
            break;
        case SpaceType::Stack:
            if (tac.opd2.getType() == OpdType::Reg) {
                assert(spOffset == 0);
                prog.push_back(gen_ldr(cond, Rt, stack_pointer, offset));
            } else {
                prog.push_back(gen_ldr(cond, Rt, stack_pointer, tac.opd2.getVal() + spOffset));
            }
            break;
        default:
            assert(false);
    }
}

void genLoadSpShift(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg, TBase fbase, int spOffset, const string &shift)
{
    assert(tac.opr == TacOpr::LoadSpASL || 
           tac.opr == TacOpr::LoadSpLSR || 
           tac.opr == TacOpr::LoadSpASR);
    assert(tac.opd2.getType() == OpdType::Reg);
    assert(spOffset == 0);
    auto Rt = toOperand2(tac.opd1, reg);
    auto offset = toOperand2(tac.opd2, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_ldr(cond, Rt, stack_pointer, offset, shift));
}

void genLoadAdd(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg, 
                TBase fbase, int spOffset, const string &shift)
{
    assert(tac.opr == TacOpr::LoadAdd || 
           tac.opr == TacOpr::LoadAddLSR || 
           tac.opr == TacOpr::LoadAddASL ||
           tac.opr == TacOpr::LoadAddASR);
    assert(tac.opd2.getType() == OpdType::Reg);
    auto Rt = toOperand2(tac.opd1, reg);
    auto Rn = toOperand2(tac.opd2, reg);
    auto Rm = toOperand2(tac.opd3, reg);
    auto cond = get_cond_code(tac.cond);
    if (shift.empty())
        prog.push_back(gen_ldr(cond, Rt, Rn, Rm));
    else 
        prog.push_back(gen_ldr(cond, Rt, Rn, Rm, shift));
}
/*
Restraint:
    opd1 must be register
    Abs: opd2 must be register
    Stack: if opd2 is immediate, it must be in [-4095, 4095]
    Other space types are not permitted
*/
void genStore(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg, TBase fbase, int spOffset) {
    assert(tac.opr == TacOpr::Store);
    assert(tac.opd1.getType() == OpdType::Reg);
    auto space = fbase(tac.opd3.getVal());
    auto Rt = toOperand2(tac.opd1, reg);
    auto offset = toOperand2(tac.opd2, reg);
    auto cond = get_cond_code(tac.cond);
    switch (space) {
        case SpaceType::Abs:
            assert(tac.opd2.getType() == OpdType::Reg);
            prog.push_back(gen_str(cond, Rt, offset, 0));
            break;
        case SpaceType::Stack:
            if (tac.opd2.getType() == OpdType::Reg) {
                assert(spOffset == 0);
                prog.push_back(gen_str(cond, Rt, stack_pointer, offset));
            } else {
                prog.push_back(gen_str(cond, Rt, stack_pointer, tac.opd2.getVal() + spOffset));
            }
            break;
        default:
            assert(false);
    }
}

int genAddr(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg, TBase fbase) {
    assert(tac.opr == TacOpr::Addr);
    auto space = fbase(tac.opd3.getVal());
    auto Rt = toOperand2(tac.opd1, reg);
    auto cond = get_cond_code(tac.cond);
    switch (space) {
        case SpaceType::Abs:
            genMov(prog, Tac(TacOpr::Mov, tac.opd1, tac.opd2), reg);
            return 0;
        case SpaceType::BSS:
            assert(tac.opd2.getType() == OpdType::Imme);
            assert(tac.opd2.getVal() == 0);
            prog.push_back(gen_ldr(cond, Rt, ""));
//            prog.push_back(gen_add(false, "", Rt, Rt, toOperand2(tac.opd2, reg)));
            return BSS_SEC_ID;
        case SpaceType::Data:
            assert(tac.opd2.getType() == OpdType::Imme);
            assert(tac.opd2.getVal() == 0);
            prog.push_back(gen_ldr(cond, Rt, ""));
//            prog.push_back(gen_add(false, "", Rt, Rt, toOperand2(tac.opd2, reg)));
            return DATA_SEC_ID;
        case SpaceType::Stack:
            prog.push_back(gen_add(false, cond, Rt, stack_pointer, toOperand2(tac.opd2, reg)));
            return 0;
    }
}