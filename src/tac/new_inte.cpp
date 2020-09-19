/* This is an intepreter for the TAC. */

#include <ctime>
#include <iostream>
#include <list>
#include <map>
#include <stack>
#include <vector>

#include "../env/env.h"
#include "../util/cuteprint.h"
#include "tac.h"

namespace NInterpreter {
/*
    TAC oprand type code
*/
const unsigned int NONE = 1;
const unsigned int REG = NONE << 1;
const unsigned int IMME = REG << 1;
const unsigned int LABEL = IMME << 1;

struct FuncInfo {
    int startLine;
    int id;
    int length;
    std::vector<int> params;
    std::string name;
    std::map<TacOpd, int> reg;

    FuncInfo(int startLine, int id, int length, std::vector<int> params,
             const std::string name, const std::map<TacOpd, int> &reg)
        : startLine(startLine),
          id(id),
          length(length),
          params(params),
          name(name),
          reg(reg) {}
};

std::vector<Tac> insts;
std::map<int, int> regs;    // (register, content)
std::map<int, int> labels;  // (label, index)
std::map<int, FuncInfo *> funcs;
std::stack<int> stk;
std::map<int, int> mem;

int sp, dataBase, bssBase;

int getID(const TacOpd &opd, int subscript, const std::map<TacOpd, int> &reg) {
    debugger << opd.name() << std::endl;
    return reg.at(opd) | (subscript << 10);
}

unsigned int getTypeCode(OpdType type) {
    if (type == OpdType::Null) return NONE;
    if (type == OpdType::Reg) return REG;
    if (type == OpdType::Imme) return IMME;
    if (type == OpdType::Label) return LABEL;
    return -1;
}

int typeCheck(const TacOpd &opd, unsigned int type, const Tac &inst) {
    if ((getTypeCode(opd.getType()) | type) != type) {
        std::cerr << "(TAC interpreter) Oprand error in " << inst.to_string() << "."
                  << std::endl;
        return OPR_ERROR;
    }
    return OK;
}

int getTop(int &result, const std::string &name) {
    if (stk.empty()) {
        std::cerr << "(TAC interpreter) Stack error in " << name << "." << std::endl;
        return STACK_ERROR;
    }
    result = stk.top();
    stk.pop();
    return OK;
}

int getValue(const TacOpd &opd, int nestCounter, const std::map<TacOpd, int> &reg) {
    return opd.getType() == OpdType::Reg ? regs[getID(opd, nestCounter, reg)]
                                         : opd.getVal();
}

int getBase(const SpaceType &st) {
    switch (st) {
        case SpaceType::Abs:
            return 0;
        case SpaceType::BSS:
            return bssBase * WORD_SIZE;
        case SpaceType::Data:
            return dataBase * WORD_SIZE;
        case SpaceType::Stack:
            return sp * WORD_SIZE;
    }
}

int runFunc(int id, const TBase &fbase, std::istream &is, std::ostream &os,
            int nestCounter) {
    int base, n, value, size, result;
    char ch;
    switch (id) {
        case GETINT:
            if (is >> n)
                stk.push(n);
            else
                stk.push(0);
            return OK;

        case GETCH:
            ch = is.get();
            if (is)
                stk.push(ch);
            else
                stk.push(0);
            return OK;

        case GETARRAY:
            if (getTop(base, "getarray")) return STACK_ERROR;
            base /= WORD_SIZE;
            if (is >> n)
                stk.push(n);
            else
                stk.push(0);
            for (int index = 0; index < n; ++index) {
                if (!(is >> mem[base + index])) mem[base + index] = 0;
            }
            return OK;

        case PUTINT:
            if (getTop(value, "putint")) return STACK_ERROR;
            os << value;
            return OK;

        case PUTCH:
            if (getTop(value, "putch")) return STACK_ERROR;
            os << static_cast<char>(value);
            return OK;

        case PUTARRAY:
            if (getTop(n, "putarray")) return STACK_ERROR;
            if (getTop(base, "putarray")) return STACK_ERROR;
            os << n << ":";
            for (int index = 0; index < n; ++index) {
                os << " " << mem[base + index];
            }
            return OK;

        case PUTF:
        case STARTTIME:
        case STOPTIME:
            return OK;

        case MEMSET:
            if (getTop(base, "memset")) return STACK_ERROR;
            if (getTop(value, "memset")) return STACK_ERROR;
            if (getTop(size, "memset")) return STACK_ERROR;
            size /= WORD_SIZE;
            base /= WORD_SIZE;
            for (int j = 0; j < WORD_SIZE; ++j) value |= value << 8;
            for (int index = 0; index < size; ++index) mem[base + index] = value;
            stk.push(base * WORD_SIZE);
            return OK;

        default:
            break;
    }
    auto func = funcs[id];
    if (func == nullptr) return MISS_FUNCTION;
    n = -4;
    for (auto it = func->params.begin(); it != func->params.end(); ++it, ++n) {
        auto param = *it;
        int value;
        if (getTop(value, func->name)) return STACK_ERROR;
        auto opd = TacOpd(param, OpdType::Reg);
        if (func->reg.find(opd) != func->reg.end()) {
            regs[getID(opd, nestCounter, func->reg)] = value;
        }
        if (n >= 0) mem[sp + func->length + n] = value;
    }
    for (int pc = func->startLine;; ++pc) {
        auto inst = insts[pc];
        auto opd1 = inst.opd1;
        auto opd2 = inst.opd2;
        auto opd3 = inst.opd3;
        auto opd4 = inst.opd4;
        switch (inst.opr) {
            case TacOpr::Neg:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, NONE, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    -getValue(opd2, nestCounter, func->reg);
                break;
            case TacOpr::Add:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) +
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::Sub:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) -
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::Mul:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) *
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::Smmul:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    ((long long)getValue(opd2, nestCounter, func->reg) *
                     getValue(opd3, nestCounter, func->reg)) >>
                    32;
                break;
            case TacOpr::BIC:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) &
                    (~getValue(opd3, nestCounter, func->reg));
                break;
            case TacOpr::Div:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) /
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::Mod:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) %
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::ASL:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg)
                    << getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::ASR:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) >>
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::LSR:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    (unsigned int)(getValue(opd2, nestCounter, func->reg)) >>
                    ((unsigned int)getValue(opd3, nestCounter, func->reg));
                break;
            case TacOpr::MLA:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd4, REG, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) *
                        getValue(opd3, nestCounter, func->reg) +
                    getValue(opd4, nestCounter, func->reg);
                break;
            case TacOpr::MLS:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd4, REG, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) *
                        getValue(opd3, nestCounter, func->reg) -
                    getValue(opd4, nestCounter, func->reg);
                break;
            case TacOpr::AddLS:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd4, IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) +
                    (getValue(opd3, nestCounter, func->reg)
                     << getValue(opd4, nestCounter, func->reg));
                break;
            case TacOpr::SubLS:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd4, IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) -
                    (getValue(opd3, nestCounter, func->reg)
                     << getValue(opd4, nestCounter, func->reg));
                break;
            case TacOpr::RsbLS:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd4, IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    (getValue(opd3, nestCounter, func->reg)
                     << getValue(opd4, nestCounter, func->reg)) -
                    getValue(opd2, nestCounter, func->reg);
                break;
            case TacOpr::AddLSR:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd4, IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) +
                    ((unsigned)getValue(opd3, nestCounter, func->reg) >>
                     (unsigned)getValue(opd4, nestCounter, func->reg));
                break;
            case TacOpr::RsbASR:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd4, IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    (getValue(opd3, nestCounter, func->reg) >>
                     getValue(opd4, nestCounter, func->reg)) -
                    getValue(opd2, nestCounter, func->reg);
                break;
            case TacOpr::Gt:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) >
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::Lt:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) <
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::Ge:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) >=
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::Le:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) <=
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::Eq:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) ==
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::Ne:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) !=
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::Not:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, NONE, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    !getValue(opd2, nestCounter, func->reg);
                break;
            case TacOpr::And:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) &&
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::Or:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, REG | IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg) ||
                    getValue(opd3, nestCounter, func->reg);
                break;
            case TacOpr::Branch:
                if (typeCheck(opd1, LABEL, inst)) return OPR_ERROR;
                if (typeCheck(opd2, NONE, inst)) return OPR_ERROR;
                if (typeCheck(opd3, NONE, inst)) return OPR_ERROR;
                pc = labels[opd1.getId()];
                break;
            case TacOpr::Beqz:
                if (typeCheck(opd1, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd2, LABEL, inst)) return OPR_ERROR;
                if (typeCheck(opd3, NONE, inst)) return OPR_ERROR;
                if (!getValue(opd1, nestCounter, func->reg))
                    pc = labels[opd2.getId()];
                break;
            case TacOpr::Bnez:
                if (typeCheck(opd1, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd2, LABEL, inst)) return OPR_ERROR;
                if (typeCheck(opd3, NONE, inst)) return OPR_ERROR;
                if (getValue(opd1, nestCounter, func->reg)) pc = labels[opd2.getId()];
                break;
            case TacOpr::Ret:
                if (typeCheck(opd1, REG | IMME | NONE, inst)) return OPR_ERROR;
                if (typeCheck(opd2, NONE, inst)) return OPR_ERROR;
                if (typeCheck(opd3, NONE, inst)) return OPR_ERROR;
                if (opd1.getType() == OpdType::Null) return OK;
                stk.push(getValue(opd1, nestCounter, func->reg));
                return OK;
            case TacOpr::Param:
                if (typeCheck(opd1, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd2, IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, NONE, inst)) return OPR_ERROR;
                stk.push(getValue(opd1, nestCounter, func->reg));
                break;
            case TacOpr::Call:
                if (typeCheck(opd1, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | NONE, inst)) return OPR_ERROR;
                if (typeCheck(opd3, NONE, inst)) return OPR_ERROR;
                n = getValue(opd1, nestCounter, func->reg);
                sp += func->length + func->params.size();
                result = runFunc(n, fbase, is, os, nestCounter + 1);
                sp -= func->length + func->params.size();
                if (result != OK) return result;
                if (opd2.getType() != OpdType::Null) {
                    if (getTop(regs[getID(opd2, nestCounter, func->reg)], func->name))
                        return STACK_ERROR;
                }
                break;
            case TacOpr::Mov:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, NONE, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getValue(opd2, nestCounter, func->reg);
                break;
            case TacOpr::Load:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    mem[(getBase(fbase(opd3.getVal())) +
                         getValue(opd2, nestCounter, func->reg)) /
                        WORD_SIZE];
                break;
            case TacOpr::Store:
                if (typeCheck(opd1, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, IMME, inst)) return OPR_ERROR;
                mem[(getBase(fbase(opd3.getVal())) +
                     getValue(opd2, nestCounter, func->reg)) /
                    WORD_SIZE] = getValue(opd1, nestCounter, func->reg);
                break;
            case TacOpr::Addr:
                if (typeCheck(opd1, REG, inst)) return OPR_ERROR;
                if (typeCheck(opd2, REG | IMME, inst)) return OPR_ERROR;
                if (typeCheck(opd3, IMME, inst)) return OPR_ERROR;
                regs[getID(opd1, nestCounter, func->reg)] =
                    getBase(fbase(opd3.getVal())) +
                    getValue(opd2, nestCounter, func->reg);
                break;
            default:
                break;
        }
    }
    return OK;
}
}  // namespace NInterpreter

int new_inte(const TacProg &prog, std::istream &is, std::ostream &os) {
    using namespace NInterpreter;
    insts.clear();
    regs.clear();
    labels.clear();
    funcs.clear();

    dataBase = 0;
    bssBase = dataBase + prog.data.length;
    sp = bssBase + prog.bss.length;

    for (int i = 0; i < prog.data.length; ++i) {
        mem[dataBase + i] = prog.data.store[i];
    }

    for (auto p : prog.data.initRegs) {
        regs[p.first] = p.second;
    }

    for (auto func : prog.funcs) {
        funcs[func->id] = new FuncInfo(insts.size(), func->id, func->length,
                                       func->paramId, func->name, func->reg);
        for (auto inst : func->insts) {
            insts.push_back(inst);
        }
    }
    for (int i = 0; i < insts.size(); ++i) {
        if (insts[i].opr == TacOpr::Labl) {
            if (typeCheck(insts[i].opd1, LABEL, insts[i])) return OPR_ERROR;
            labels[insts[i].opd1.getId()] = i;
        }
    }

    int result = runFunc(MAIN, prog.fbase, is, os, 0);
    if (result != OK) return result;
    os << stk.top();
    return OK;
}
