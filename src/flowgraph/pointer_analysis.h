#pragma once

#include "flowgraph.h"
#include "../tac/tac.h"
#include <vector>

using ProgPtr = std::shared_ptr<TacProg>;

class PointerAnalyzer {
private:
    std::set<TacOpd> params; 
    std::vector<std::map<TacOpd, std::set<int>>> param_cond;
    bool independent(const std::set<int> &a, const std::set<int> &b) const;
    std::set<int> getSpace(const std::map<TacOpd, std::set<int>> &sp, const TacOpd &opd) const;
    std::set<int> getSpace(const std::map<TacOpd, std::set<int>> &sp, TacPtr inst) const;
public:
    PointerAnalyzer(const TacProg &prog, const FlowGraph &flowgraph);
    // @return: true if opd1 and opd2 never points to the same space
    bool independent(TacPtr str, TacPtr ldr) const;
    // @return: get a vector containing all clean opds in @load_opds 
    std::set<TacPtr> getClean(const std::vector<TacPtr> &store_insts, 
                              const std::vector<TacPtr> &load_insts) const;
    bool independent(const std::vector<TacPtr> &store_opds, TacPtr opd2) const;
};
