#pragma once

#include <memory>

#include "../tac/tac.h"
#include "../util/cuteprint.h"
#include "asm.h"
#include "gen_unit/gen_unit.h"

using std::string;
using std::to_string;

void genNeg(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genMov(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genAdd(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genSub(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genMul(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genASL(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genASR(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genLSR(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genMLA(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genMLS(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genAddLS(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genSubLS(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genRsbLS(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genAddLSR(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genRsbASR(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genBIC(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genSmmul(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);

void genCondBranch(Lines &prog, const Tac &condition, const Tac &branch,
                   const std::map<TacOpd, int> &reg);
void genBranch(Lines &prog, const Tac &branch, const std::map<TacOpd, int> &reg);
void genLabel(Lines &prog, const Tac &tac);

void genLoad(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg,
             TBase fbase, int spOffset);
void genLoadSpShift(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg, 
             TBase fbase, int spOffset, const string &shift);
void genLoadAdd(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg, 
                TBase fbase, int spOffset, const string &shift);

void genStore(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg,
              TBase fbase, int spOffset);
int genAddr(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg,
            TBase fbase);

void genProgramStart(Lines &prog);
void genBss(Lines &prog, int bss_length);
void genData(Lines &prog, const std::vector<int> &Data);

void genParam(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg,
              int param_count);
void genCall(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg,
             std::string func_name, int param_count, bool lineno);
void genFuncStart(Lines &prog, std::string func_name, int stack_length_in_words, const std::set<int> &used_registers);
void genRet(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg,
            int stack_length_in_words, const std::set<int> &used_registers);
void genDiv(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);
void genMod(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg);

std::set<int> liveRegs(const std::set<TacOpd> &live, const std::map<TacOpd, int> reg);
void speFuncCall(Lines &prog, const std::vector<TacPtr> &params, const Tac &tac,
                 const string &funcName, const std::map<TacOpd, int> reg,
                 bool lineno);