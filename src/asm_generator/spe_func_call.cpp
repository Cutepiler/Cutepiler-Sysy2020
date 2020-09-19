/*
    A function call generator for special testcases:
    - No function with number of parameters over 4;
    - No instructions within a function call except 'Param' and 'Call';
*/

#include "asm.h"
#include "gen.h"
#include "gen_unit/gen_unit.h"
#include "help.h"

std::set<int> liveRegs(const std::set<TacOpd> &live, const std::map<TacOpd, int> reg) {
    std::set<int> result;
    for (auto opd : live) {
        result.insert(reg.at(opd));
    }
    return result;
}

static std::vector<std::pair<int, int>> findCircle(std::set<std::pair<int, int>> &assign) {
    for (auto p : assign) {
        std::vector<std::pair<int, int>> result;
        result.push_back(p);
        bool findNext;
        do {
            findNext = false;
            for (auto p : assign) {
                if (p.first == result.back().second) {
                    result.push_back(p);
                    findNext = true;
                    break;
                }
            }
        } while (findNext && result.front().first != result.back().second);
        if (findNext) {
            for (auto p : result) assign.erase(p);
            return result;
        }
    }
    return std::vector<std::pair<int, int>>();
}

static void genMoves(Lines &prog, std::set<std::pair<int, int>> &assign, std::pair<int, int> toGen) {
    assign.erase(toGen);
    for (auto p : assign) {
        auto [des, ori] = p;
        if (ori == toGen.first) {
            genMoves(prog, assign, p);
            break;
        }
    }
    prog.push_back(gen_mov(false, "", gpr(toGen.first), gpr(toGen.second)));
}

void speFuncCall(Lines &prog, const std::vector<TacPtr> &params, const Tac &tac,
                 const string &funcName, const std::map<TacOpd, int> reg, bool lineno) {
    std::set<int> save2stack;
    int temp = -1;
    for (auto id : liveRegs(tac.getLiveIn(), reg)) {
        if (is_caller_save(id)) {
            save2stack.insert(id);
            if (id >= params.size()) temp = id;
        }
    }

    auto live = liveRegs(params.empty() ? tac.getLiveIn() : params.front()->getLiveIn(), reg);
    std::set<int> notlive;
    for (int id = params.size(); id < 13; ++id) {
        if (!is_caller_save(id) && !live.count(id)) notlive.insert(id);
    }
    if (!tac.opd2.empty()) notlive.erase(reg.at(tac.opd2));
    if (temp == -1) {
        temp = *notlive.begin(); // Dangerous!
        notlive.erase(temp);
    }

    std::set<std::pair<int, int>> save2reg;
    while (!save2stack.empty() && !notlive.empty()) {
        int des = *notlive.begin();
        int ori = *save2stack.begin();
        save2reg.insert(std::make_pair(des, ori));
        notlive.erase(des);
        save2stack.erase(ori);
        prog.push_back(gen_mov(false, "", gpr(des), gpr(ori)));
    }

    int offset = 0;
    for (auto id : save2stack) {
        offset += 4;
        prog.push_back(gen_str("", gpr(id), stack_pointer, -offset));
    }
    if (offset) prog.push_back(gen_sub(false, "", stack_pointer, stack_pointer, constant(offset)));
    std::set<std::pair<int, int>> assign;
    for (auto inst : params) {
        auto desId = inst->opd2.getVal() - 1;
        if (inst->opd1.getType() == OpdType::Imme) {
            gen_mov32(prog, gpr(desId), inst->opd1.getVal());
        } else {
            auto oriId = reg.at(inst->opd1);
            if (desId == oriId) continue;
            assign.insert(std::make_pair(desId, oriId));
        }
    }
    while (true) {
        auto c = findCircle(assign);
        if (c.empty()) break;
        assert(temp != -1); // Dangerous!
        prog.push_back(gen_mov(false, "", gpr(temp), gpr(c.front().first)));
        c.pop_back();
        for (auto [des, ori] : c) {
            prog.push_back(gen_mov(false, "", gpr(des), gpr(ori)));
        }
        prog.push_back(gen_mov(false, "", gpr(c.back().second), gpr(temp)));
    }
    while (!assign.empty()) {
        genMoves(prog, assign, *assign.begin());
    }
    if (lineno) {
        assert(params.size() == 0);
        prog.push_back(gen_mov("", "r0", 0));
    }
    prog.push_back(gen_bl(funcName));
    if (offset) prog.push_back(gen_add(false, "", stack_pointer, stack_pointer, constant(offset)));
    if (!tac.opd2.empty()) {
        prog.push_back(gen_mov(false, "", toOperand2(tac.opd2, reg), gpr(0)));
    }
    offset = 0;
    for (auto id : save2stack) {
        offset += 4;
        prog.push_back(gen_ldr("", gpr(id), stack_pointer, -offset));
    }
    for (auto [des, ori] : save2reg) {
        prog.push_back(gen_mov(false, "", gpr(ori), gpr(des)));
    }
}

void genDivOrMod(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg, const string &funcName) {
    auto liveIn = tac.getLiveOut();
    liveIn.erase(tac.opd1);
    std::set<int> save2stack;
    int temp = -1;
    for (auto id : liveRegs(liveIn, reg)) {
        if (is_caller_save(id)) {
            save2stack.insert(id);
            if (id >= 2) temp = id;
        }
    }

    auto live = liveRegs(tac.getLiveIn(), reg);
    std::set<int> notlive;
    for (int id = 2; id < 13; ++id) {
        if (!is_caller_save(id) && !live.count(id)) notlive.insert(id);
    }
    notlive.erase(reg.at(tac.opd1));
    if (temp == -1) {
        temp = *notlive.begin(); // Dangerous!
        notlive.erase(temp);
    }

    std::set<std::pair<int, int>> save2reg;
    while (!save2stack.empty() && !notlive.empty()) {
        int des = *notlive.begin();
        int ori = *save2stack.begin();
        save2reg.insert(std::make_pair(des, ori));
        notlive.erase(des);
        save2stack.erase(ori);
        prog.push_back(gen_mov(false, "", gpr(des), gpr(ori)));
    }

    int offset = 0;
    for (auto id : save2stack) {
        offset += 4;
        prog.push_back(gen_str("", gpr(id), stack_pointer, -offset));
    }
    if (offset) prog.push_back(gen_sub(false, "", stack_pointer, stack_pointer, constant(offset)));
    std::set<std::pair<int, int>> assign;

    if (tac.opd2.getType() == OpdType::Imme) {
        gen_mov32(prog, gpr(0), tac.opd2.getVal());
    } else {
        auto id = reg.at(tac.opd2);
        if (id != 0) assign.insert(std::make_pair(0, id));
    }

    if (tac.opd3.getType() == OpdType::Imme) {
        gen_mov32(prog, gpr(1), tac.opd3.getVal());
    } else {
        auto id = reg.at(tac.opd3);
        if (id != 1) assign.insert(std::make_pair(1, id));
    }

    while (true) {
        auto c = findCircle(assign);
        if (c.empty()) break;
        assert(temp != -1); // Dangerous!
        prog.push_back(gen_mov(false, "", gpr(temp), gpr(c.front().first)));
        c.pop_back();
        for (auto [des, ori] : c) {
            prog.push_back(gen_mov(false, "", gpr(des), gpr(ori)));
        }
        prog.push_back(gen_mov(false, "", gpr(c.back().second), gpr(temp)));
    }
    while (!assign.empty()) {
        genMoves(prog, assign, *assign.begin());
    }
    prog.push_back(gen_bl(funcName));
    if (offset) prog.push_back(gen_add(false, "", stack_pointer, stack_pointer, constant(offset)));
    if (!tac.opd2.empty()) {
        prog.push_back(gen_mov(false, "", toOperand2(tac.opd1, reg), gpr(0)));
    }
    offset = 0;
    for (auto id : save2stack) {
        offset += 4;
        prog.push_back(gen_ldr("", gpr(id), stack_pointer, -offset));
    }
    for (auto [des, ori] : save2reg) {
        prog.push_back(gen_mov(false, "", gpr(ori), gpr(des)));
    }
}
/*
void genMod(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
    genDivOrMod(prog, tac, reg, "__modsi3");
}

void genDiv(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
    genDivOrMod(prog, tac, reg, "__divsi3");
}
*/