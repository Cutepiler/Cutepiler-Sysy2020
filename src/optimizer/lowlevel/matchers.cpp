#include "inst_merging.h"
#include <cassert>

using std::make_tuple;
using std::vector;
using std::make_shared;

static TreeDPInfo not_matched()
{
    return make_tuple(vector<TreePtr>(), make_shared<Tac>(TacOpr::_Head), -1);
}

TreeDPInfo SingleMatcher::match(TreePtr tree)
{
    if (tree->opr != Single) return not_matched();
    assert(tree->lc == nullptr);
    assert(tree->rc == nullptr);
    return make_tuple(vector<TreePtr>(), nullptr, 0);
}

TreeDPInfo BinaryMatcher::match(TreePtr tree)
{
    if (tree->opr != this->opr) return not_matched();
    vector<TreePtr> sub = {tree->lc, tree->rc};
    assert(tree->lc != nullptr);
    assert(tree->rc != nullptr);
    auto inst = make_shared<Tac>(this->opr, tree->des, tree->lc->des, tree->rc->des);
    return make_tuple(sub, inst, delay);
}

TreeDPInfo MLAMatcher::match(TreePtr tree)
{
    if (tree->opr != TacOpr::Add) return not_matched();
    if (left) {
        assert(tree->lc != nullptr);
        if (tree->lc->opr != TacOpr::Mul) return not_matched();
        assert(tree->lc->lc != nullptr);
        assert(tree->lc->rc != nullptr);
        auto inst = make_shared<Tac>(TacOpr::MLA, tree->des, tree->lc->lc->des, tree->lc->rc->des, tree->rc->des);
        vector<TreePtr> sub = {tree->lc->lc, tree->lc->rc, tree->rc};
        return make_tuple(sub, inst, 3);
    } else {
        assert(tree->rc != nullptr);
        if (tree->rc->opr != TacOpr::Mul) return not_matched();
        assert(tree->rc->lc != nullptr);
        assert(tree->rc->rc != nullptr);
        auto inst = make_shared<Tac>(TacOpr::MLA, tree->des, tree->rc->lc->des, tree->rc->rc->des, tree->lc->des);
        vector<TreePtr> sub = {tree->rc->lc, tree->rc->rc, tree->lc};
        return make_tuple(sub, inst, 3);       
    }
}

TreeDPInfo MLSMatcher::match(TreePtr tree)
{
    if (tree->opr != TacOpr::Sub) return not_matched();
    assert(tree->rc != nullptr);
    if (tree->rc->opr != TacOpr::Mul) return not_matched();
    assert(tree->rc->lc != nullptr);
    assert(tree->rc->rc != nullptr);
    auto inst = make_shared<Tac>(TacOpr::MLS, tree->des, tree->rc->lc->des, 
                                tree->rc->rc->des, tree->lc->des);
    vector<TreePtr> sub = {tree->lc, tree->rc->lc, tree->rc->rc};
    return make_tuple(sub, inst, 3);
}

TreeDPInfo AddLSMatcher::match(TreePtr tree)
{
    if (tree->opr != TacOpr::Add) return not_matched();
    TreePtr lsl_tree, single_tree;
    if (left) {
        lsl_tree = tree->lc;
        single_tree = tree->rc;
    } else {
        lsl_tree = tree->rc;
        single_tree = tree->lc;
    }
    if (lsl_tree->opr != TacOpr::ASL) return not_matched();
    assert(lsl_tree->lc != nullptr);
    assert(lsl_tree->rc != nullptr); 
    auto inst = make_shared<Tac>(TacOpr::AddLS, tree->des, single_tree->des, 
                                lsl_tree->lc->des, lsl_tree->rc->des);
    vector<TreePtr> sub = {lsl_tree->lc, lsl_tree->rc, single_tree};
    return make_tuple(sub, inst, 1);
}

TreeDPInfo LoadSpMatcher::match(TreePtr tree)
{
    if (tree->opr != TacOpr::Load) return not_matched();
    assert(tree->rc != nullptr);
    assert(tree->rc->des.getType() == OpdType::Imme);
    if (fbase(tree->rc->des.getVal()) != SpaceType::Stack) return not_matched();
    TacOpr loadsp;
    switch (tree->lc->opr) {
        case TacOpr::ASL:
            loadsp = TacOpr::LoadSpASL; 
            break;
        case TacOpr::LSR:
            loadsp = TacOpr::LoadSpLSR;
            break;
        case TacOpr::ASR:
            loadsp = TacOpr::LoadSpASR;
            break;
        default: return not_matched(); 
    }
    assert(tree->lc->lc != nullptr);
    assert(tree->lc->rc != nullptr);
    assert(tree->lc->rc->des.getType() == OpdType::Imme);
    auto inst = make_shared<Tac>(loadsp, tree->des, tree->lc->lc->des, tree->lc->rc->des);
    vector<TreePtr> sub = {tree->lc->lc, tree->lc->rc};
    return make_tuple(sub, inst, 4);
}

TreeDPInfo LoadAddMatcher::match(TreePtr tree)
{
    if (tree->opr != TacOpr::Load) return not_matched();
    assert(tree->rc != nullptr);
    assert(tree->rc->des.getType() == OpdType::Imme);
    if (fbase(tree->rc->des.getVal()) != SpaceType::Abs) return not_matched();
    if (tree->lc->opr != TacOpr::Add) return not_matched();
    assert(tree->lc->lc != nullptr);
    assert(tree->lc->rc != nullptr);
    auto inst = make_shared<Tac>(TacOpr::LoadAdd, tree->des, tree->lc->lc->des, tree->lc->rc->des);
    vector<TreePtr> sub = {tree->lc->lc, tree->lc->rc};
    return make_tuple(sub, inst, 4);    
}

TreeDPInfo LoadAddShiftMatcher::match(TreePtr tree)
{
    if (tree->opr != TacOpr::Load) return not_matched();
    assert(tree->rc != nullptr);
    assert(tree->rc->des.getType() == OpdType::Imme);
    if (fbase(tree->rc->des.getVal()) != SpaceType::Abs) return not_matched();
    if (tree->lc->opr != TacOpr::Add) return not_matched();
    TreePtr shift_tree, single_tree;
    if (left) shift_tree = tree->lc->lc, single_tree = tree->lc->rc;
    else shift_tree = tree->lc->rc, single_tree = tree->lc->lc;
    assert(shift_tree != nullptr);
    assert(single_tree != nullptr);
    TacOpr load;
    switch (shift_tree->opr) {
        case TacOpr::ASL:
            load = TacOpr::LoadAddASL; 
            break;
        case TacOpr::LSR:
            load = TacOpr::LoadAddLSR;
            break;
        case TacOpr::ASR:
            load = TacOpr::LoadAddASR;
            break;
        default: return not_matched(); 
    }
    auto inst = make_shared<Tac>(load, tree->des, single_tree->des, 
                                shift_tree->lc->des, shift_tree->rc->des);
    vector<TreePtr> sub = {shift_tree->lc, shift_tree->rc, single_tree};
    return make_tuple(sub, inst, 4);
}
