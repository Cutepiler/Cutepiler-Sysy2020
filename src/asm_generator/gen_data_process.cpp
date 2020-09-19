#include "gen.h"
#include "help.h"
#include <tuple>

using std::string;
using std::to_string;
using std::tuple;
using std::make_tuple;

void genMov(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
    assert(tac.opr == TacOpr::Mov);
    auto Rd = toOperand2(tac.opd1, reg);
    auto cond = get_cond_code(tac.cond);
    if (tac.opd2.getType() == OpdType::Reg) {
        auto Rn = toOperand2(tac.opd2, reg);
        if (Rd != Rn) prog.push_back(gen_mov(false, cond, Rd, Rn));
        return;
    }
    assert(tac.opd2.getType() == OpdType::Imme);
    int value = tac.opd2.getVal();
    gen_mov32(prog, cond, Rd, value);
}

void genAdd(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
    assert(tac.opr == TacOpr::Add);
    assert(!(tac.opd2.getType() == OpdType::Imme &&
             tac.opd3.getType() == OpdType::Imme));
    auto Rd = toOperand2(tac.opd1, reg);
    auto Rn = toOperand2(tac.opd2, reg);
    auto op2 = toOperand2(tac.opd3, reg);
    auto cond = get_cond_code(tac.cond);
    if (tac.opd2.getType() == OpdType::Imme) std::swap(Rn, op2);
    prog.push_back(gen_add(false, cond, Rd, Rn, op2));
}

void genNeg(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
    assert(tac.opr == TacOpr::Neg);
    assert(tac.opd2.getType() == OpdType::Reg);
    auto Rd = toOperand2(tac.opd1, reg);
    auto Rm = toOperand2(tac.opd2, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_neg(cond, Rd, Rm));
}

static void check_range(TacOpd opd, int l, int r)
{
    if (opd.getType() == OpdType::Imme) 
        assert(l <= opd.getVal() && opd.getVal() <= r); 
}

tuple<string, string, string> reg2arb(const Tac &tac, const std::map<TacOpd, int> &reg)
{
    assert(tac.opd1.getType() == OpdType::Reg);
    assert(tac.opd2.getType() == OpdType::Reg);
    assert(tac.opd3.getType() == OpdType::Imme 
        || tac.opd3.getType() == OpdType::Reg);
    auto Rd = toOperand2(tac.opd1, reg);
    auto Rn = toOperand2(tac.opd2, reg);
    auto Rm = toOperand2(tac.opd3, reg);
    return make_tuple(Rd, Rn, Rm);
}

void genASL(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
    assert(tac.opr == TacOpr::ASL);
    check_range(tac.opd3, 0, 31);
    auto [Rd, Rn, Rm] = reg2arb(tac, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_asl(false, cond, Rd, Rn, Rm));
}

void genASR(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
    assert(tac.opr == TacOpr::ASR);
    check_range(tac.opd3, 1, 32);
    auto [Rd, Rn, Rm] = reg2arb(tac, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_asr(false, cond, Rd, Rn, Rm));
}

void genLSR(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
    assert(tac.opr == TacOpr::LSR);
    check_range(tac.opd3, 1, 32);
    auto [Rd, Rn, Rm] = reg2arb(tac, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_lsr(false, cond, Rd, Rn, Rm));
}

void genBIC(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg)
{
    assert(tac.opr == TacOpr::BIC);
    auto [Rd, Rn, Rm] = reg2arb(tac, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_bic(false, cond, Rd, Rn, Rm));
}

void genSub(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
    assert(tac.opr == TacOpr::Sub);
    assert(!(tac.opd2.getType() == OpdType::Imme &&
             tac.opd3.getType() == OpdType::Imme));
    auto Rd = toOperand2(tac.opd1, reg);
    auto op1 = toOperand2(tac.opd2, reg);
    auto op2 = toOperand2(tac.opd3, reg);
    auto cond = get_cond_code(tac.cond);
    if (tac.opd2.getType() == OpdType::Imme) {
        prog.push_back(gen_rsb(false, cond, Rd, op2, op1));
    } else {
        prog.push_back(gen_sub(false, cond, Rd, op1, op2));
    }
}

void genMul(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
    assert(tac.opr == TacOpr::Mul);
    assert(tac.opd2.getType() == OpdType::Reg);
    assert(tac.opd3.getType() == OpdType::Reg);
    auto Rd = toOperand2(tac.opd1, reg);
    auto Rn = toOperand2(tac.opd2, reg);
    auto Rm = toOperand2(tac.opd3, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_mul(false, cond, Rd, Rn, Rm));
}

tuple<string, string, string> reg3(const Tac &tac, const std::map<TacOpd, int> &reg)
{
    assert(tac.opd1.getType() == OpdType::Reg);
    assert(tac.opd2.getType() == OpdType::Reg);
    assert(tac.opd3.getType() == OpdType::Reg);
    auto Rd = toOperand2(tac.opd1, reg);
    auto Rn = toOperand2(tac.opd2, reg);
    auto Rm = toOperand2(tac.opd3, reg);
    return make_tuple(Rd, Rn, Rm);
}

void genSmmul(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
    assert(tac.opr == TacOpr::Smmul);
    auto [Rd, Rn, Rm] = reg3(tac, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_smmul(false, cond, Rd, Rn, Rm));
}

tuple<string, string, string, string> reg4(const Tac &tac, const std::map<TacOpd, int> &reg)
{
    assert(tac.opd1.getType() == OpdType::Reg);
    assert(tac.opd2.getType() == OpdType::Reg);
    assert(tac.opd3.getType() == OpdType::Reg);
    assert(tac.opd4.getType() == OpdType::Reg);
    auto Rd = toOperand2(tac.opd1, reg);
    auto Rn = toOperand2(tac.opd2, reg);
    auto Rm = toOperand2(tac.opd3, reg);
    auto Ra = toOperand2(tac.opd4, reg);
    return make_tuple(Rd, Rn, Rm, Ra);
}

void genMLA(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg)
{
    assert(tac.opr == TacOpr::MLA);
    auto [Rd, Rn, Rm, Ra] = reg4(tac, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_mla(false, cond, Rd, Rn, Rm, Ra));
}

void genMLS(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg)
{
    assert(tac.opr == TacOpr::MLS);
    auto [Rd, Rn, Rm, Ra] = reg4(tac, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_mls(false, cond, Rd, Rn, Rm, Ra));
}

tuple<string, string, string, string> reg3imm(const Tac &tac, const std::map<TacOpd, int> &reg)
{
    assert(tac.opd1.getType() == OpdType::Reg);
    assert(tac.opd2.getType() == OpdType::Reg);
    assert(tac.opd3.getType() == OpdType::Reg);
    assert(tac.opd4.getType() == OpdType::Imme);
    auto Rd = toOperand2(tac.opd1, reg);
    auto Rn = toOperand2(tac.opd2, reg);
    auto Rm = toOperand2(tac.opd3, reg);
    auto sh = toOperand2(tac.opd4, reg);
    return make_tuple(Rd, Rn, Rm, sh);
}

void genAddLS(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg)
{
    assert(tac.opr == TacOpr::AddLS);
    auto [Rd, Rn, Rm, sh] = reg3imm(tac, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_add(false, cond, Rd, Rn, reg_with_shift(Rm, "lsl", sh)));
}

void genSubLS(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg)
{
    assert(tac.opr == TacOpr::SubLS);
    auto [Rd, Rn, Rm, sh] = reg3imm(tac, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_sub(false, cond, Rd, Rn, reg_with_shift(Rm, "lsl", sh)));
}

void genRsbLS(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg)
{
    assert(tac.opr == TacOpr::RsbLS);
    auto [Rd, Rn, Rm, sh] = reg3imm(tac, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_rsb(false, cond, Rd, Rn, reg_with_shift(Rm, "lsl", sh)));
}

void genAddLSR(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg)
{
    assert(tac.opr == TacOpr::AddLSR);
    auto [Rd, Rn, Rm, sh] = reg3imm(tac, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_add(false, cond, Rd, Rn, reg_with_shift(Rm, "lsr", sh)));
}

void genRsbASR(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg)
{
    assert(tac.opr == TacOpr::RsbASR);
    auto [Rd, Rn, Rm, sh] = reg3imm(tac, reg);
    auto cond = get_cond_code(tac.cond);
    prog.push_back(gen_rsb(false, cond, Rd, Rn, reg_with_shift(Rm, "asr", sh)));
}

