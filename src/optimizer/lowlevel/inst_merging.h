#pragma once 

#include <memory>
#include <cassert>
#include <vector>
#include <list>
#include "../../tac/tac.h"
#include "../../flowgraph/flowgraph.h"

struct TreeNode;
using TreePtr = std::shared_ptr<TreeNode>;
using TreeDPInfo = std::tuple<std::vector<TreePtr>, TacPtr, int>;
class Matcher {
public:
    virtual TreeDPInfo match(TreePtr) = 0;
    virtual ~Matcher() {}
};
using MatcherPtr = std::shared_ptr<Matcher>;
static const auto Single = TacOpr::_Head;

struct TreeNode {
    TacOpd des;
    TacOpr opr;
    TreePtr lc, rc;
    int cost;
    MatcherPtr matcher;    
    TreeNode() { lc = rc = nullptr, matcher = nullptr; } 
    TreeNode(TacOpd des, TacOpr opr) : des(des), opr(opr)
    { lc = rc = nullptr, matcher = nullptr; }  
};

std::vector<TacPtr> gen_code(TreePtr tree);
void register_matchers(std::function<SpaceType(int)> fbase);
void inst_merge(FlowGraph &FlowGraph, std::function<SpaceType(int)> fbase);

class SingleMatcher : public Matcher {
public:
    virtual TreeDPInfo match(TreePtr);
    virtual ~SingleMatcher() {}
};

class BinaryMatcher : public Matcher {
    TacOpr opr;
    int delay;
public:
    BinaryMatcher(TacOpr opr, int delay): opr(opr), delay(delay) 
    { }
    virtual TreeDPInfo match(TreePtr);
    virtual ~BinaryMatcher() {}
};

class MLAMatcher : public Matcher {
    bool left;
public:
    MLAMatcher(bool left) : left(left)
    { }
    virtual TreeDPInfo match(TreePtr);
    virtual ~MLAMatcher() {}
};

class MLSMatcher : public Matcher {
public:
    virtual TreeDPInfo match(TreePtr);
    virtual ~MLSMatcher() {}
};

class AddLSMatcher : public Matcher {
    bool left;
public:
    AddLSMatcher(bool left) : left(left)
    { }
    virtual TreeDPInfo match(TreePtr);
    virtual ~AddLSMatcher() {}
};

class LoadSpMatcher : public Matcher {
    std::function<SpaceType(int)> fbase;
public:
    LoadSpMatcher(std::function<SpaceType(int)> fbase) : fbase(fbase) 
    { }
    virtual TreeDPInfo match(TreePtr);
    virtual ~LoadSpMatcher() {}
};

class LoadAddMatcher : public Matcher {
    std::function<SpaceType(int)> fbase;
public:
    LoadAddMatcher(std::function<SpaceType(int)> fbase) : fbase(fbase) 
    { }
    virtual TreeDPInfo match(TreePtr);
    virtual ~LoadAddMatcher() {}
};

class LoadAddShiftMatcher : public Matcher {
    std::function<SpaceType(int)> fbase;
    bool left;
public:
    LoadAddShiftMatcher(std::function<SpaceType(int)> fbase, bool left) 
        : fbase(fbase), left(left) { }
    virtual TreeDPInfo match(TreePtr);
    virtual ~LoadAddShiftMatcher() {}
};
