#include "flowgraph.h"
#include "pointer_analysis.h"
#include "work_iteration.h"

using std::vector;
using std::set;
using std::swap;
using std::map;

PointerAnalyzer::PointerAnalyzer(const TacProg &prog, const FlowGraph &flowgraph)
{ 
    params.clear();
    param_cond.clear();
    bool complex_func = false;
    auto blocks = flowgraph.getBlocks();
    // std::cerr << "Start Work Iteration" << std::endl;
    // std::cerr << prog << std::endl;
    work_iteration(prog, flowgraph);
    // std::cerr << "End Work Iteration" << std::endl;
    // step 1 : decide whether this function is complex
    for (auto block : blocks) {
        if (complex_func) break;
        for (auto inst : block->insts) {
            if (complex_func) break;
            if (inst.opr == TacOpr::Call) {
                if (inst.opd1.getType() == OpdType::Null)
                    continue;
                assert(inst.opd1.getType() == OpdType::Imme);
                int id = inst.opd1.getVal();
                if (id != flowgraph.func->id) 
                    continue;
                auto fp = flowgraph.func->paramId.begin();
                for (auto cur = inst.pred; 
                        cur->opr == TacOpr::Param; cur = cur->pred, fp++) {
                    if (cur->opd1.getType() == OpdType::Imme)
                        continue;
                    assert(cur->opd1.getType() == OpdType::Reg);
                    if (is_pointer(cur->opd1) && cur->opd1.getId() != *fp) {
                        complex_func = true;
                        break;
                    }
                }
            }
        }
    }
    if (complex_func || flowgraph.func->name == "main") {
        // if it is complex function, fall back to simple result
        map<TacOpd, set<int>> mp;
        for (int fp : flowgraph.func->paramId) {
            TacOpd opd = TacOpd(fp, OpdType::Reg);
            mp[opd] = get_space(opd);
        }
        param_cond.push_back(mp);
    } else {
        map<TacOpd, TacOpd> src_map;
        for (auto block : blocks) {
            for (auto inst : block->insts) {
                for (int i = 1; i <= 4; i++) {
                    auto opd = inst.getOpd(i);
                    if (opd.getType() != OpdType::Reg) continue;
                    if (!is_pointer(opd)) continue; 
                    auto src = get_source(opd);
                    if (src.getType() == OpdType::Null) continue;
                    src_map[opd] = src;
                }
            }
        }
        for (auto func : prog.funcs) {
            assert(flowgraph.func != nullptr);
            assert(func != nullptr);
            if (func->id == flowgraph.func->id)
                continue;
            for (auto inst : func->insts) {
                if (inst.opr != TacOpr::Call) continue;
                assert (inst.opd1.getType() == OpdType::Imme);
                if (inst.opd1.getVal() != flowgraph.func->id) 
                    continue;
                map<TacOpd, set<int>> mp;
                auto fp = flowgraph.func->paramId.begin();
                // std::cerr << "I\'m here " << flowgraph.func->name << " " << flowgraph.func->id << " " << inst.to_string() << std::endl;
                for (auto cur = inst.pred; cur->opr == TacOpr::Param; cur = cur->pred, fp++) {
                    TacOpd opd = cur->opd1;
                    assert(fp != flowgraph.func->paramId.end());
                    TacOpd pt = TacOpd(*fp, OpdType::Reg);
                    if (is_pointer(opd))
                        mp[pt] = get_space(opd);
                }
                for (auto [opd, src] : src_map) {
                    assert(mp.count(src));
                    mp[opd] = mp[src];
                }
                param_cond.push_back(mp);
            }
        }
    }
    // std::cerr << flowgraph.func->name << " init finished, size = " << param_cond.size() << std::endl;
}

set<int> PointerAnalyzer::getSpace(const std::map<TacOpd, std::set<int>> &sp, TacPtr inst) const
{
    assert(inst->opd3.getType() == OpdType::Imme);
    if (inst->opd3.getVal() == 0) {
        assert(inst->opd2.getType() == OpdType::Reg);
        // std::cerr << "get space of : " << inst->to_string() << " " << is_pointer(inst->opd2) << std::endl;
        auto res = getSpace(sp, inst->opd2);
        // std::cerr << "finish" << std::endl;
        return res;
    } else {
        return {inst->opd3.getVal()};
    }
}

bool PointerAnalyzer::independent(TacPtr str, TacPtr ldr) const
{
    for (const auto & sp : param_cond) {
        if (!independent(getSpace(sp, str), getSpace(sp, ldr)))
            return false;
    }
    return true;
}

set<TacPtr> PointerAnalyzer::getClean(const vector<TacPtr> &store_insts, const vector<TacPtr> &load_insts) const
{
    set<TacPtr> clean_insts;
    for (auto opd : load_insts)
        clean_insts.insert(opd); 
    for (const auto & sp : param_cond) {
        set<int> st;
        for (auto inst : store_insts)
            st.merge(getSpace(sp, inst));
        for (auto inst : load_insts)
            if (!independent(st, getSpace(sp, inst)) && clean_insts.count(inst)) 
                clean_insts.erase(inst);
    }
    return clean_insts;
}

bool PointerAnalyzer::independent(const vector<TacPtr> &store_opds, TacPtr opd) const
{
    vector<TacPtr> load_opds = {opd};
    /* std::cerr << "-- Stores --" << std::endl;
    for (auto inst : store_opds) 
        std::cerr << "\t" << inst->to_string() << std::endl;
    std::cerr << opd->to_string() << std::endl; */
    auto res = getClean(store_opds, load_opds);
    // std::cerr << "Result: " << !res.empty() << std::endl; 
    return !res.empty();
}

bool PointerAnalyzer::independent(const set<int> &s1, const set<int> &s2) const
{
    if (s1.size() > s2.size()) {
        for (auto u : s2)
            if (s1.count(u))
                return false;
    } else {
        for (auto u : s1)
            if (s2.count(u))
                return false;
    }
    return true;
}

set<int> PointerAnalyzer::getSpace(const map<TacOpd, set<int>> &sp, const TacOpd &opd) const
{
    if (sp.count(opd))
        return sp.at(opd);
    assert(is_pointer(opd));
    return get_space(opd);
}
