#include <cassert>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "tac.h"


std::ostream &operator<<(std::ostream &os, const TacFunc &func) {
    os << "Function" << func.id << " " << func.name << "(";
    if (!func.paramId.empty()) {
        os << func.paramId[0];
        for (int i = 1; i < func.paramId.size(); ++i) {
            os << ", " << func.paramId[i];
        }
    }
    os << "):" << std::endl;
    for (auto inst : func.insts) {
        os << "  " << inst.to_string() << std::endl;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const TacData &data) {
    os << "Data:" << std::endl << " ";
    for (auto d : data.store) {
        os << " " << d;
    }
    os << std::endl;
    for (auto p : data.initRegs) {
        os << "  R" << p.first << " = " << p.second << std::endl;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const TacBSS &bss) {
    return os << "BSS length:" << bss.length << std::endl;
}

std::ostream &operator<<(std::ostream &os, const TacOpd &opd){
    return os << opd.name() << std::endl;
}
std::ostream &operator<<(std::ostream &os, const TacProg &prog) {
    os << "== Start of TAC ==" << std::endl;
    for (auto func : prog.funcs) {
        os << *func << std::endl;
    }
    os << prog.bss << std::endl;
    os << prog.data << std::endl;
    os << "== End of TAC ==" << std::endl;
    return os;
}

void name_discretization(TacProg &prog) {
    int idCounter = 0;
    std::map<int, int> newId;
    std::set<int> preAlloc;
    for (auto func : prog.funcs) {
        for (auto param : func->paramId) {
            newId[param] = param;
            preAlloc.insert(param);
        }
    }
    for (auto func : prog.funcs) {
        for (auto inst = func->insts.head->succ; inst != func->insts.tail;
            inst = inst->succ) {
            for (int i = 1; i <= 4; ++i) {
                auto &opd = inst->getOpd(i);
                if (opd.getType() != OpdType::Reg) continue;
                if (newId.count(opd.getId())) {
                    opd = TacOpd(newId[opd.getId()], OpdType::Reg);
                } else {
                    do {
                        ++idCounter;
                    } while (preAlloc.count(idCounter));
                    newId[opd.getId()] = idCounter;
                    opd = TacOpd(idCounter, OpdType::Reg);
                }
            }
        }
    }
}

std::shared_ptr<TacFunc> TacFunc::copyMyself() const {
    auto result = std::make_shared<TacFunc>(name, paramId, id, length);
    result->is_pure = is_pure;
    result->reg = reg;
    result->hasLoop = hasLoop;
    for (auto inst : insts) {
        result->insts.push_back(std::make_shared<Tac>(inst));
    }
    return result;
}