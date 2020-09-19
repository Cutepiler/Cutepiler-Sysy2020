#include <string>
#include "../asm.h"
#include "gen_unit.h"

using std::string;

LinePtr gen_mov(bool S, const string &cond, const string &Rd,
                const string &Operand2) {
    return std::make_shared<Line>(
        (S ? "movs" : "mov") + cond + " " + Rd + ", " + Operand2, 32);
}

LinePtr gen_mvn(bool S, const string &cond, const string &Rd,
                const string &Operand2) {
    return std::make_shared<Line>(
        (S ? "mvns" : "mvn") + cond + " " + Rd + ", " + Operand2, 32);
}

LinePtr gen_mov(const string &cond, const string &Rd, int imm16) {
    return std::make_shared<Line>(
        "mov" + cond + " " + Rd + ", #" + std::to_string(imm16), 32);
}

LinePtr gen_movw(const string &cond, const string &Rd, int imm16) {
    return std::make_shared<Line>(
        "movw" + cond + " " + Rd + ", #" + std::to_string(imm16), 32);
}
LinePtr gen_movt(const string &cond, const string &Rd, int imm16) {
    return std::make_shared<Line>(
        "movt" + cond + " " + Rd + ", #" + std::to_string(imm16), 32);
}

void gen_mov32(Lines &prog, const string &Rd, int imm32) {
    auto op2 = constant(imm32);
    if (op2 != "") {
        prog.push_back(gen_mov(false, "", Rd, op2));
        return;
    }
    auto nop2 = constant(~imm32);
    if (nop2 != "") {
        prog.push_back(gen_mvn(false, "", Rd, nop2));
        return;
    }
    prog.push_back(gen_movw("", Rd, imm32 & 0xffff));
    prog.push_back(gen_movt("", Rd, (imm32 >> 16) & 0xffff));
}

void gen_mov32(Lines &prog, const string &cond, const string &Rd, int imm32) {
    auto op2 = constant(imm32);
    if (op2 != "") {
        prog.push_back(gen_mov(false, cond, Rd, op2));
        return;
    }
    auto nop2 = constant(~imm32);
    if (nop2 != "") {
        prog.push_back(gen_mvn(false, cond, Rd, nop2));
        return;
    }
    prog.push_back(gen_movw(cond, Rd, imm32 & 0xffff));
    prog.push_back(gen_movt(cond, Rd, (imm32 >> 16) & 0xffff));
}

LinePtr gen_lsl(bool S, const string &cond, const string &Rd, const string &Rm,
                const string &Rs) {
    return std::make_shared<Line>(
        (S ? "lsls" : "lsl") + cond + " " + Rd + ", " + Rm + ", " + Rs, 32);
}
LinePtr gen_lsl(bool S, const string &cond, const string &Rd, const string &Rm,
                int sh) {  // sh in [0, 31]
    return std::make_shared<Line>((S ? "lsls" : "lsl") + cond + " " + Rd + ", " + Rm +
                                      ", #" + std::to_string(sh),
                                  32);
}
LinePtr gen_lsr(bool S, const string &cond, const string &Rd, const string &Rm,
                const string &Rs) {
    return std::make_shared<Line>(
        (S ? "lsrs" : "lsr") + cond + " " + Rd + ", " + Rm + ", " + Rs, 32);
}
LinePtr gen_lsr(bool S, const string &cond, const string &Rd, const string &Rm,
                int sh) {  // sh in [1, 32]
    return std::make_shared<Line>((S ? "lsrs" : "lsr") + cond + " " + Rd + ", " + Rm +
                                      ", #" + std::to_string(sh),
                                  32);
}

LinePtr gen_adc(bool S, const string &cond, const string &Rd, const string &Rn,
                const string &Operand2) {
    return std::make_shared<Line>(
        (S ? "adcs" : "adc") + cond + " " + Rd + ", " + Rn + ", " + Operand2, 32);
}

LinePtr gen_add(bool S, const string &cond, const string &Rd, const string &Rn,
                const string &Operand2) {
    return std::make_shared<Line>(
        (S ? "adds" : "add") + cond + " " + Rd + ", " + Rn + "," + Operand2, 32);
}

LinePtr gen_sub(bool S, const string &cond, const string &Rd, const string &Rn,
                const string &Operand2) {
    return std::make_shared<Line>(
        (S ? "subs" : "sub") + cond + " " + Rd + ", " + Rn + "," + Operand2, 32);
}

LinePtr gen_rsb(bool S, const string &cond, const string &Rd, const string &Rn,
                const string &Operand2) {
    return std::make_shared<Line>(
        (S ? "rsbs" : "rsb") + cond + " " + Rd + ", " + Rn + "," + Operand2, 32);
}

LinePtr gen_bic(bool S, const string &cond, const string &Rd, const string &Rn,
                const string &Operand2) {
    return std::make_shared<Line>(
        (S ? "bics" : "bic") + cond + " " + Rd + ", " + Rn + ", " + Operand2, 32);
}

LinePtr gen_asl(bool S, const string &cond, const string &Rd, const string &Rn,
                const string &Rm) {
    return std::make_shared<Line>(
        (S ? "lsls" : "lsl") + cond + " " + Rd + ", " + Rn + "," + Rm, 32);
}

LinePtr gen_asr(bool S, const string &cond, const string &Rd, const string &Rn,
                const string &Rm) {
    return std::make_shared<Line>(
        (S ? "asrs" : "asr") + cond + " " + Rd + ", " + Rn + "," + Rm, 32);
}

LinePtr gen_mul(bool S, const string &cond, const string &Rd, const string &Rn,
                const string &Rm) {
    return std::make_shared<Line>(
        (S ? "muls" : "mul") + cond + " " + Rd + ", " + Rn + ", " + Rm, 32);
}

LinePtr gen_smmul(bool S, const string &cond, const string &Rd, const string &Rn,
                  const string &Rm) {
    return std::make_shared<Line>(
        (S ? "smmuls" : "smmul") + cond + " " + Rd + ", " + Rn + ", " + Rm, 32);
}

LinePtr gen_mla(bool S, const string &cond, const string &Rd, const string &Rn,
                const string &Rm, const string &Ra) {
    return std::make_shared<Line>(
        (S ? "mlas" : "mla") + cond + " " + Rd + ", " + Rn + ", " + Rm + ", " + Ra,
        32);
}

LinePtr gen_mls(bool S, const string &cond, const string &Rd, const string &Rn,
                const string &Rm, const string &Ra) {
    return std::make_shared<Line>(
        (S ? "mlss" : "mls") + cond + " " + Rd + ", " + Rn + ", " + Rm + ", " + Ra,
        32);
}

LinePtr gen_cmp(const string &cond, const string &Rn, const string &Operand2) {
    return std::make_shared<Line>("cmp" + cond + " " + Rn + ", " + Operand2, 32);
}
LinePtr gen_neg(const string &cond, const string &Rd, const string &Rm) {
    return std::make_shared<Line>("neg" + cond + " " + Rd + ", " + Rm, 32);
}