#include <algorithm>
#include <functional>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <vector>

#include "../tac/tac.h"
#include "../util/functional.h"
#include "flowgraph.h"

using std::pair;
using std::string;
using std::to_string;
using std::vector;

Position::Position(BBPtr bb, TacOpd phiN) : _isPhi(true), bb(bb), phiName(phiN) {}
Position::Position(BBPtr bb, TacPtr tac) : _isPhi(false), bb(bb), tac(tac) {}
bool Position::isPhi() const { return _isPhi; }
BBPtr Position::getBB() const { return bb; }
TacOpd Position::getPhiName() const { return phiName; }
void Position::setPhiName(TacOpd opd) { phiName = opd; }
TacPtr Position::getTac() const { return tac; }
string Position::name() const {
    if (isPhi())
        return "phi(id = " + to_string(bb->getId()) + ")";
    else
        return "tac(" + tac->to_string() + ")";
}

bool BasicBlock::dominate(BBPtr a, BBPtr b) {
    return b->doms.find(a) != b->doms.end();
}

void BasicBlock::computeDepth() {
    if (idom == nullptr)
        depth = 1;
    for (auto chl : child) {
        chl->depth = depth + 1;
        chl->computeDepth();
    }
}

bool BasicBlock::strictDominate(BBPtr a, BBPtr b) { return a != b && dominate(a, b); }

void BasicBlock::addPred(BBPtr pred) { this->pred.insert(pred); }
void BasicBlock::addSucc(BBPtr succ) { this->succ.insert(succ); }
void BasicBlock::delPred(BBPtr pred) {
    this->pred.erase(pred);
    if (phi.empty()) return;
    for (auto &p : phi) {
        p.second.erase(pred);
    }
}
void BasicBlock::delSucc(BBPtr succ) { this->succ.erase(succ); }
BasicBlock::BasicBlock() : reachedBySource(false) {
    static int id_counter;
    id = ++id_counter;
}
int BasicBlock::getId() const { return id; }
bool BasicBlock::isConditional() const { return succ.size() == 2; }

void BasicBlock::setTrueBranch(BBPtr self, BBPtr tb) {
    if (trueBranch) succ.erase(trueBranch);
    trueBranch = tb;
    succ.insert(trueBranch);
    trueBranch->pred.insert(self);
}

void BasicBlock::setFalseBranch(BBPtr self, BBPtr tf) {
    if (falseBranch) succ.erase(falseBranch);
    falseBranch = tf;
    succ.insert(falseBranch);
    falseBranch->pred.insert(self);
}

void BasicBlock::replaceSucc(BBPtr self, BBPtr pre, BBPtr now) {
    assert(succ.count(pre));
    if (succ.size() == 1) {
        succ.erase(pre); 
        succ.insert(now);
    } else if (succ.size() == 2) {
        if (pre == getTrueBranch())
            setTrueBranch(self, now);
        if (pre == getFalseBranch()) 
            setFalseBranch(self, now);
    } else assert(false);
}

void FlowGraph::addBlock(BBPtr block) {
    blocks.insert(block); 
}

BBPtr BasicBlock::getTrueBranch() const {
    assert(succ.size() > 1);
    return trueBranch;
}

BBPtr BasicBlock::getFalseBranch() const {
    assert(succ.size() > 1);
    return falseBranch;
}

BBPtr BasicBlock::getPred() const {
    assert(pred.size() == 1);
    return *pred.begin();
}

BBPtr BasicBlock::getSucc() const {
    assert(succ.size() == 1);
    return *succ.begin();
}

void BasicBlock::visitFromSource() {
    if (reachedBySource) return;
    reachedBySource = true;
    for (auto s : succ) s->visitFromSource();
}

void FlowGraph::addEdge(BBPtr fromEnd, BBPtr toEnd) {
    fromEnd->addSucc(toEnd);
    toEnd->addPred(fromEnd);
}

void FlowGraph::delEdge(BBPtr fromEnd, BBPtr toEnd) {
    fromEnd->delSucc(toEnd);
    toEnd->delPred(fromEnd);
}

void msgFlowGraphConstructor(const std::string &msg) {
//    logger << "[FlowGraph Constructor] " << msg << std::endl;
}

FlowGraph::FlowGraph(const Insts &insts, std::shared_ptr<TacFunc> func, std::function<SpaceType(int)> fbase) : func(func), fbase(fbase) {
    msgFlowGraphConstructor("Start.");

    std::map<int, BBPtr> labl2block;
    std::vector<BBPtr> blocks;

    auto inst = insts.head->succ;
    BBPtr lastBlock = nullptr;

    while (inst != insts.tail) {
        BBPtr curBlock = std::make_shared<BasicBlock>();
        if (startBlock == nullptr) startBlock = curBlock;
        if (lastBlock != nullptr) addEdge(lastBlock, curBlock);
        blocks.push_back(curBlock);
        while (inst != insts.tail) {
            if (inst->opr == TacOpr::Ret) {
                inst = inst->succ;
                curBlock->insts = Insts::cut(insts.head->succ, inst->pred);
                lastBlock = nullptr;
                break;
            } 
            if (inst->opr == TacOpr::Branch) {
                inst = inst->succ;
                curBlock->insts = Insts::cut(insts.head->succ, inst->pred);
                lastBlock = nullptr;
                break;
            }
            if (inst->opr == TacOpr::Beqz || inst->opr == TacOpr::Bnez) {
                inst = inst->succ;
                curBlock->insts = Insts::cut(insts.head->succ, inst->pred);
                lastBlock = curBlock;
                break;
            }
            if (inst->opr == TacOpr::Labl && inst != insts.head->succ) {
                if (inst->pred->opr == TacOpr::Ret)
                    lastBlock = nullptr;
                else
                    lastBlock = curBlock;
                curBlock->insts = Insts::cut(insts.head->succ, inst->pred);
                inst = inst->succ;
                break;
            }
            inst = inst->succ;
        }
        if (inst == insts.tail) {
            if (!curBlock->insts.empty()) {
                curBlock = std::make_shared<BasicBlock>();
                if (startBlock == nullptr) startBlock = curBlock;
                if (lastBlock != nullptr) addEdge(lastBlock, curBlock);
                blocks.push_back(curBlock);
            }
            curBlock->insts = insts;
        }
    }

    msgFlowGraphConstructor("Block cutting finished.");

    for (auto block : blocks) {
        auto inst = block->insts.head->succ;
        if (inst->opr == TacOpr::Labl) {
            labl2block[inst->opd1.getId()] = block;
        }
    }

    for (auto block : blocks) {
        if (block->insts.empty()) continue;
        auto inst = block->insts.tail->pred;
        if (inst->opr == TacOpr::Branch) {
            assert(labl2block[inst->opd1.getId()] != nullptr);
            addEdge(block, labl2block[inst->opd1.getId()]);
        } else if (inst->opr == TacOpr::Beqz || inst->opr == TacOpr::Bnez) {
            assert(labl2block[inst->opd2.getId()] != nullptr);
            block->falseBranch = *block->succ.begin();
            addEdge(block, labl2block[inst->opd2.getId()]);
            block->trueBranch = labl2block[inst->opd2.getId()];
        }
    }

    msgFlowGraphConstructor("Edge adding finished.");

    toSatisfyUniquePredOrSuccProp();

    msgFlowGraphConstructor(
        "Unique successor or predecessor property has been satisfied.");

    // delete blocks unreached by the start block
    startBlock->visitFromSource();
    msgFlowGraphConstructor("Finish visiting from source.");
    msgFlowGraphConstructor("#blocks = " + std::to_string(blocks.size()));
    for (auto block : blocks) {
        if (block->reachedBySource) {
            this->blocks.insert(block);
        } else {
            removeSingle(block);
        }
    }

    msgFlowGraphConstructor("Unreached blocks have been removed.");
    msgFlowGraphConstructor("All finished.");
}

void FlowGraph::toSatisfyUniquePredOrSuccProp() {
    // to satisfy unique successor or predecessor property
    std::set<std::pair<BBPtr, BBPtr>> toModify;
    for (auto from : blocks) {
        if (from->succ.size() <= 1) continue;
        for (auto to : from->succ) {
            if (to->pred.size() <= 1) continue;
            toModify.insert(std::make_pair(from, to));
        }
    }
    for (auto p : toModify) {
        auto from = p.first, to = p.second;
        from->succ.erase(to);
        to->pred.erase(from);
        auto blank = std::make_shared<BasicBlock>();
        from->succ.insert(blank);
        to->pred.insert(blank);
        blank->pred.insert(from);
        blank->succ.insert(to);
        if (from->trueBranch == to) from->trueBranch = blank;
        if (from->falseBranch == to) from->falseBranch = blank;
        blocks.insert(blank);
    }
}

BBPtr FlowGraph::getStartBlock() const { return startBlock; }
std::vector<BBPtr> FlowGraph::getBlocks() const {
    std::vector<BBPtr> result;
    std::transform(blocks.begin(), blocks.end(), std::back_inserter(result),
                   [](const BBPtr &val) -> BBPtr { return val; });
    return result;
}

void FlowGraph::setStartBlock(BBPtr block) {
    startBlock = block;
}

void FlowGraph::calcVars() {
    vars.clear();
    auto insertOpd = [&](TacOpd opd) {
        if (opd.getType() == OpdType::Reg) {
            vars.insert(opd);
        }
    };
    for (auto bb : blocks) {
        for (auto p : bb->phi) {
            vars.insert(p.first);
            for (auto q : p.second) vars.insert(q.second);
        }
        for (auto inst : bb->insts) {
            for (int i = 1; i <= 4; ++i)
                insertOpd(inst.getOpd(i));
        }
    }
}

void FlowGraph::calcDefUses() {
    def.clear();
    uses.clear();
    for (auto bb : blocks) {
        for (auto p : bb->phi) {
            auto var = p.first;
            auto pos = std::make_shared<Position>(bb, var);
            def[var] = pos;
            for (auto q : p.second) uses[q.second].insert(pos);
        }
        for (auto inst = bb->insts.head->succ; inst != bb->insts.tail;
             inst = inst->succ) {
            for (int i = 1; i <= 4; ++i) {
                if (inst->isDef(i)) {
                    def[inst->getOpd(i)] = std::make_shared<Position>(bb, inst);
                } else if (inst->isUse(i)) {
                    uses[inst->getOpd(i)].insert(
                        std::make_shared<Position>(bb, inst));
                }
            }
        }
    }
}

void FlowGraph::remove(BBPtr block) {
    std::queue<BBPtr> toRemove;
    toRemove.push(block);
    while (!toRemove.empty()) {
        auto b = toRemove.front();
        blocks.erase(block);
        toRemove.pop();
        for (auto s : b->succ) {
            if (s->pred.size() == 1) toRemove.push(s);
        }
        for (auto p : b->pred) p->delSucc(b);
        for (auto s : b->succ) s->delPred(b);
        b->pred.clear();
        b->succ.clear();
    }
}

void FlowGraph::removeSingle(BBPtr block) {
    blocks.erase(block);
    for (auto p : block->pred) p->delSucc(block);
    for (auto s : block->succ) s->delPred(block);
    block->pred.clear();
    block->succ.clear();
}

template <class TV, class TE>
Graph<TV, TE> *FlowGraph::getStruct(std::function<TV(BBPtr)> fV,
                                    std::function<TE(BBPtr, BBPtr)> fE) const {
    auto g = new Graph<TV, TE>();
    std::map<BBPtr, Vertex<TV, TE> *> block2vertex;
    for (auto block : blocks) {
        auto v = new Vertex<TV, TE>(fV(block));
        block2vertex[block] = v;
        g->vertices.insert(v);
    }
    for (auto bf : blocks) {
        for (auto bt : blocks) {
            auto from = block2vertex[bf], to = block2vertex[bt];
            auto e = new Edge<TV, TE>(fE(bf, bt), from, to);
            g->edges.insert(e);
            from->out.insert(e);
            to->in.insert(e);
        }
    }
    return g;
}

void pushBack(BBPtr block, TacPtr inst) {
    switch (block->insts.tail->pred->opr) {
        case TacOpr::Beqz:
        case TacOpr::Bnez:
            block->insts.tail->pred->pred->pred->insert(inst);
            return;
        case TacOpr::Branch:
            block->insts.tail->pred->pred->insert(inst);
            return;
        default:
            block->insts.push_back(inst);
    }
}

void FlowGraph::toTac() {
    for (auto block : blocks) {
        for (auto [des, oris] : block->phi) {
            for (auto [bb, ori] : oris) {
                pushBack(bb, std::make_shared<Tac>(TacOpr::Mov, des, ori));
            }
        }
        block->phi.clear();
    }
    calcVars();
    calcDefUses();
}

std::set<BBPtr> FlowGraph::getBlockSet() const {
    return blocks; 
}

Insts FlowGraph::toInsts() {
    /*
    std::set<BBPtr> toRemove;
    for (auto block : blocks) {
        if (block->insts.head->succ != block->insts.tail) continue;
        if (block->pred.size() == 1 && block->succ.size() == 1) {
            auto p = block->getPred(), s = block->getSucc();
            if (p->succ.size() == 1) {
                p->succ.erase(block);
                s->pred.erase(block);
                addEdge(p, s);
                block->pred.clear();
                block->succ.clear();
                toRemove.insert(block);
            } else if (s->pred.size() == 1) {
                if (p->getTrueBranch() == block) p->setTrueBranch(p, s);
                if (p->getFalseBranch() == block) p->setFalseBranch(p, s);
                s->pred.erase(block);
                block->succ.clear();
                toRemove.insert(block);
            } 
        }
    }
    for (auto block : toRemove) blocks.erase(block);
    */
    std::map<BBPtr, TacOpd> labl;
    static int lablCounter = 0;
    for (auto block : blocks) {
        auto first = block->insts.head->succ;
        if (first->opr == TacOpr::Labl) first->remove();
        if (block->succ.size() == 1) {
            auto last = block->insts.tail->pred;
            if (last->opr == TacOpr::Branch || last->opr == TacOpr::Beqz ||
                last->opr == TacOpr::Bnez)
                last->remove();
        }
    }

    for (auto block : blocks) {
        if ((block->pred.size() + (block == startBlock)) > 1) {
            auto nlb = TacOpd(++lablCounter, OpdType::Label);
            block->insts.push_front(std::make_shared<Tac>(TacOpr::Labl, nlb));
            labl[block] = nlb;
        }
        if (block->succ.size() > 1) {
            auto nlb = TacOpd(++lablCounter, OpdType::Label);
            block->getTrueBranch()->insts.push_front(
                std::make_shared<Tac>(TacOpr::Labl, nlb));
            labl[block->getTrueBranch()] = nlb;
            block->insts.tail->pred->opd2 = nlb;
        }
    }

    std::set<BBPtr> visit;
    std::queue<std::tuple<BBPtr, Insts, bool>> q;
    q.push(std::make_tuple(startBlock, Insts(), true));
    visit.insert(startBlock);
    Insts prog;
    while (!q.empty()) {
        auto [b, insts, isStart] = q.front();
        q.pop();
        insts.push_back(b->insts);
        if (b->succ.empty()) {
            if (isStart) {
                prog.push_front(insts);
            } else {
                prog.push_back(insts);
            }
            continue;
        }
        if (b->succ.size() == 1) {
            auto s = b->getSucc();
            if (visit.find(s) == visit.end()) {
                visit.insert(s);
                q.push(std::make_tuple(s, insts, isStart));
            } else {
                insts.push_back(std::make_shared<Tac>(TacOpr::Branch, labl.at(s)));
                if (isStart)
                    prog.push_front(insts);
                else
                    prog.push_back(insts);
            }
        } else {
            auto tb = b->getTrueBranch(), fb = b->getFalseBranch();
            assert(visit.find(tb) == visit.end());
            assert(visit.find(fb) == visit.end());
            visit.insert(tb);
            visit.insert(fb);
            q.push(std::make_tuple(fb, insts, isStart));
            q.push(std::make_tuple(tb, Insts(), false));
        }
    }
    return prog;
}

void FlowGraph::cleanBB()
{
    for (auto bb : blocks) {
        while (bb->insts.head->succ != bb->insts.tail) {
            auto fst = bb->insts.head->succ;
            if (fst->opr == TacOpr::Labl) {
                fst->remove();
            } else {
                break;
            }
        }
        while (bb->insts.head->succ != bb->insts.tail) {
            auto fst = bb->insts.tail->pred;
            if (fst->opr == TacOpr::Branch) {
                fst->remove();
            } else {
                break;
            }
        }
    }
}

void FlowGraph::removeMov() {
    for (auto bb : blocks) {
        for (auto inst = bb->insts.head->succ; inst != bb->insts.tail;
            inst = inst->succ) {
            if (inst->opr == TacOpr::Mov) {
                if (inst->opd2.getType() != OpdType::Reg) continue;
                if (func->reg.at(inst->opd1) == func->reg.at(inst->opd2)) {
                    inst = inst->pred;
                    inst->succ->remove();
                }
            }
        }
    }
}

static TacOpr negOpr(TacOpr opr) {
    switch (opr) {
        case TacOpr::Ge: return TacOpr::Lt;
        case TacOpr::Le: return TacOpr::Gt;
        case TacOpr::Gt: return TacOpr::Le;
        case TacOpr::Lt: return TacOpr::Ge;
        case TacOpr::Ne: return TacOpr::Eq;
        case TacOpr::Eq: return TacOpr::Ne;
        default: assert(false);
    }
}

void BasicBlock::swapBranches() {
    if (succ.size() < 2) return;
    std::swap(trueBranch, falseBranch);
    auto &bopr = insts.tail->pred->opr;
    switch (bopr) {
        case TacOpr::Beqz:
            bopr = TacOpr::Bnez;
            break;
        case TacOpr::Bnez:
            bopr = TacOpr::Beqz;
            break;
        default:
            assert(false);
    }
}