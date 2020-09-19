#include "help.h"
#include "gen_unit/gen_unit.h"

string get_inst_type(const string &inst) {
    string result;
    for (auto ch : inst)
        if (ch != ' ') result.push_back(ch); else break;
    return result;
}

std::vector<string> get_opd_list(const string &inst) {
    std::vector<string> result;
    string str;
    for (int i = 0; i < inst.size(); ++i) {
        char ch = inst[i];
        if (ch == ' ') {
            str.clear();
        } else if (ch == ',') {
            result.push_back(str);
            str.clear();
            ++i;
        } else str.push_back(ch);
    }
    if (str != "") result.push_back(str);
    return result;
}

string toOperand2(const TacOpd &opd, const std::map<TacOpd, int> &reg) {
    if (opd.getType() == OpdType::Reg) return gpr(reg.at(opd));
    if (opd.getType() == OpdType::Imme) return constant(opd.getVal());
}

bool is_uncond_branch(const string &inst) {
    auto type = get_inst_type(inst);
    return type == "b" || type == "bx";
}

bool is_caller_save(int id) {
    return (0 <= id && id < 4) || id == 12;
}

string get_cond_code(TacOpr cond) {
    switch (cond) {
        case TacOpr::Ge: return "ge";
        case TacOpr::Le: return "le";
        case TacOpr::Gt: return "gt";
        case TacOpr::Lt: return "lt";
        case TacOpr::Ne: return "ne";
        case TacOpr::Eq: return "eq";
        case TacOpr::_Head: return "";
        default: assert(false);
    }
}