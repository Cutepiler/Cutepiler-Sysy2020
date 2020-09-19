#include "asm.h"
#include "../env/env.h"
#include "gen.h"
#include "help.h"
#include "opt.h"

const int MAX_DIS = 4095;

bool before(LinePtr s, LinePtr t) {
    while (!s->content.empty() && s != t) s = s->succ;
    return !s->content.empty();
}

int distance(LinePtr s, LinePtr t) {
    if (!before(s, t)) std::swap(s, t);
    assert(before(s, t));
    int dis = 0;
    while (s != t) {
        dis += s->length;
        s = s->succ;
    }
    return dis;
}

void fillLabel(LinePtr inst, const string &label) { inst->content += label; }

void insertLabel(LinePtr posLabel, LinePtr posNeed, int array_id) {
    auto temp = new_label();
    posLabel->pred->insert(gen_label(temp));
    posLabel->pred->insert(
        std::make_shared<Line>(".word " + array_name(array_id), 32));
    fillLabel(posNeed, temp);
}

void addAccess2array(Lines &prog, std::vector<std::pair<LinePtr, int>> positions) {
    LinePtr last[3] = {};
    for (auto [pos, type] : positions) {
        if (last[type] != nullptr && distance(last[type], pos) < MAX_DIS) {
            auto label = last[type]->content;
            label.pop_back();
            fillLabel(pos, label);
            continue;
        }
        int dis = pos->length;
        auto toIns = pos->succ;
        while (dis < MAX_DIS) {
            if (is_uncond_branch(toIns->pred->content)) {
                if (toIns == last[3 - type]) {
                    toIns = toIns->succ->succ;
                    assert(distance(pos, toIns) < MAX_DIS);
                }
                insertLabel(toIns, pos, type);
                last[type] = toIns->pred->pred;
                break;
            }
            dis += toIns->length;
            toIns = toIns->succ;
        }
        if (dis < MAX_DIS) continue;
        toIns = toIns->pred;
        auto temp = new_label();
        toIns->pred->insert(gen_b("", temp));
        toIns->pred->insert(gen_label(temp));
        toIns = toIns->pred;
        insertLabel(toIns, pos, type);
        // if (RUN_TEST) assert(false);
    }
}

bool isSpecialProg(const TacProg &tacProg) {
    for (auto func : tacProg.funcs) {
        if (func->paramId.size() > 4) return false;
        for (auto inst = func->insts.head->succ; inst != func->insts.tail;
             inst = inst->succ) {
            if (inst->opr != TacOpr::Param) continue;
            auto opr = inst->succ->opr;
            if (opr != TacOpr::Param && opr != TacOpr::Call) return false;
        }
    }
    return true;
}

static std::set<int> used_register(std::shared_ptr<TacFunc> func)
{
    std::set<int> used;
    used.insert(4);
    for (auto inst : func->insts)
        for (int i = 1; i <= 4; i++)
            if (inst.getOpd(i).getType() == OpdType::Reg)
                used.insert(func->reg[inst.getOpd(i)]);
    return used;
}

AsmProg::AsmProg(const TacProg &tacProg) {
    std::vector<std::pair<LinePtr, int>> needLabel;
    genProgramStart(prog);
    std::map<int, std::string> id2funcName;
    for (auto func : tacProg.funcs) id2funcName[func->id] = func->name;
    id2funcName[1] = "getint";
    id2funcName[2] = "getch";
    id2funcName[3] = "getarray";
    id2funcName[4] = "putint";
    id2funcName[5] = "putch";
    id2funcName[6] = "putarray";
    id2funcName[7] = "printf";
    id2funcName[8] = "_sysy_starttime";
    id2funcName[9] = "_sysy_stoptime";
    id2funcName[10] = "memset";
    bool special = false; 
    // isSpecialProg(tacProg); TODO: trade-off with callee-save registers
    std::vector<TacPtr> params;
    for (auto func : tacProg.funcs) {
        int spOffset = 0, temp;
        int param_count = 0;
        auto used = used_register(func);
        genFuncStart(prog, func->name, func->length, used);  // Callee saved registers 9
        for (auto inst = func->insts.head->succ; inst != func->insts.tail;
             inst = inst->succ) {
            switch (inst->opr) {
                case TacOpr::And:
                case TacOpr::Or:
                case TacOpr::Not:
                    assert(false);
                    break;
                case TacOpr::Mov:
                    genMov(prog, *inst, func->reg);
                    break;
                case TacOpr::Neg:
                    genNeg(prog, *inst, func->reg);
                    break;
                case TacOpr::Add:
                    genAdd(prog, *inst, func->reg);
                    break;
                case TacOpr::Sub:
                    genSub(prog, *inst, func->reg);
                    break;
                case TacOpr::Mul:
                    genMul(prog, *inst, func->reg);
                    break;
                case TacOpr::AddLS:
                    genAddLS(prog, *inst, func->reg);
                    break;
                case TacOpr::AddLSR:
                    genAddLSR(prog, *inst, func->reg);
                    break;
                case TacOpr::ASL:
                    genASL(prog, *inst, func->reg);
                    break;
                case TacOpr::ASR:
                    genASR(prog, *inst, func->reg);
                    break;
                case TacOpr::BIC:
                    genBIC(prog, *inst, func->reg);
                    break;
                case TacOpr::LSR:
                    genLSR(prog, *inst, func->reg);
                    break;
                case TacOpr::MLA:
                    genMLA(prog, *inst, func->reg);
                    break;
                case TacOpr::MLS:
                    genMLS(prog, *inst, func->reg);
                    break;
                case TacOpr::RsbASR:
                    genRsbASR(prog, *inst, func->reg);
                    break;
                case TacOpr::RsbLS:
                    genRsbLS(prog, *inst, func->reg);
                    break;
                case TacOpr::Smmul:
                    genSmmul(prog, *inst, func->reg);
                    break;
                case TacOpr::SubLS:
                    genSubLS(prog, *inst, func->reg);
                    break;
                case TacOpr::Load:
                    genLoad(prog, *inst, func->reg, tacProg.fbase, spOffset);
                    break;
                case TacOpr::LoadSpASL:
                    assert(inst->opd3.getType() == OpdType::Imme);
                    genLoadSpShift(prog, *inst, func->reg, tacProg.fbase, spOffset, "lsl #" + to_string(inst->opd3.getVal()));
                    break;
                case TacOpr::LoadSpASR:
                    assert(inst->opd3.getType() == OpdType::Imme);
                    genLoadSpShift(prog, *inst, func->reg, tacProg.fbase, spOffset, "asr #" + to_string(inst->opd3.getVal()));
                    break;
                case TacOpr::LoadSpLSR:
                    assert(inst->opd3.getType() == OpdType::Imme);
                    genLoadSpShift(prog, *inst, func->reg, tacProg.fbase, spOffset, "lsr #" + to_string(inst->opd3.getVal()));
                    break;
                case TacOpr::LoadAdd:
                    genLoadAdd(prog, *inst, func->reg, tacProg.fbase, spOffset, "");
                    break;
                case TacOpr::LoadAddLSR:
                    assert(inst->opd4.getType() == OpdType::Imme);
                    genLoadAdd(prog, *inst, func->reg, tacProg.fbase, spOffset, "lsr #" + to_string(inst->opd4.getVal()));
                    break;
                case TacOpr::LoadAddASR:
                    assert(inst->opd4.getType() == OpdType::Imme);
                    genLoadAdd(prog, *inst, func->reg, tacProg.fbase, spOffset, "asr #" + to_string(inst->opd4.getVal()));
                    break;
                case TacOpr::LoadAddASL:
                    assert(inst->opd4.getType() == OpdType::Imme);
                    genLoadAdd(prog, *inst, func->reg, tacProg.fbase, spOffset, "lsl #" + to_string(inst->opd4.getVal()));
                    break;
                case TacOpr::Store:
                    genStore(prog, *inst, func->reg, tacProg.fbase, spOffset);
                    break;
                case TacOpr::Lt:
                case TacOpr::Gt:
                case TacOpr::Le:
                case TacOpr::Ge:
                case TacOpr::Ne:
                case TacOpr::Eq:
                    assert(inst->succ != func->insts.tail);
                    assert(inst->succ->opr == TacOpr::Beqz ||
                           inst->succ->opr == TacOpr::Bnez);
                    assert(inst->opd1 == inst->succ->opd1);
                    break;
                case TacOpr::Beqz:
                case TacOpr::Bnez:
                    genCondBranch(prog, *(inst->pred), *inst, func->reg);
                    break;
                case TacOpr::Branch:
                    genBranch(prog, *inst, func->reg);
                    break;
                case TacOpr::Labl:
                    genLabel(prog, *inst);
                    break;
                case TacOpr::Addr:
                    temp = genAddr(prog, *inst, func->reg, tacProg.fbase);
                    if (temp)
                        needLabel.push_back(std::make_pair(prog.tail->pred, temp));
                    break;
                case TacOpr::Param:
                    if (!special) {
                        genParam(prog, *inst, func->reg, param_count);
                        param_count++;
                    } else {
                        params.push_back(inst);
                    }
                    break;
                case TacOpr::Call:
                    if (!special) {
                        genCall(
                            prog, *inst, func->reg, id2funcName[inst->opd1.getVal()],
                            param_count,
                            (inst->opd1.getVal() == 8 || inst->opd1.getVal() == 9));
                        param_count = 0;
                    } else {
                        speFuncCall(
                            prog, params, *inst, id2funcName[inst->opd1.getVal()],
                            func->reg,
                            (inst->opd1.getVal() == 8 || inst->opd1.getVal() == 9));
                        params.clear();
                    }
                    break;
                case TacOpr::Ret:
                    genRet(prog, *inst, func->reg, func->length, used);
                    break;
                case TacOpr::Div:
                    genDiv(prog, *inst, func->reg);
                    break;
                case TacOpr::Mod:
                    genMod(prog, *inst, func->reg);
                    break;
                default:
                    break;
            }
        }
    }
    addAccess2array(prog, needLabel);
//    opt_ldr_str_simple(prog);
    genBss(prog, tacProg.bss.length);
    genData(prog, tacProg.data.store);
}

std::ostream &operator<<(std::ostream &os, const AsmProg &prog) {
    for (auto line = prog.prog.head->succ; line != prog.prog.tail;
         line = line->succ) {
        os << line->content << std::endl;
    }
    return os;
}
