#include "peephole.h"
#include "../../tac/tac.h"
#include <vector>
#include <climits>

using std::vector;
using std::swap;
using std::function;

//algebraic simplification and strength reduction
bool algebraic_optimization(TacPtr ptr, function<SpaceType(int)> fbase){
	switch (ptr->opr){
		case TacOpr::Add:
			if (ptr->opd2.getType() == OpdType::Imme && ptr->opd3.getType() == OpdType::Reg)
				swap(ptr->opd2, ptr->opd3);
			if (ptr->opd2.getType() == OpdType::Reg && ptr->opd3.getType() == OpdType::Imme && ptr->opd3.getVal() == 0){
				ptr->opr = TacOpr::Mov;
				ptr->opd3 = TacOpd();
				if (ptr->opd1 == ptr->opd2) ptr->remove();
				return true;
			}
			return false;
		case TacOpr::Sub:
			if (ptr->opd2.getType() == OpdType::Reg && ptr->opd3.getType() == OpdType::Imme && ptr->opd3.getVal() == 0){
				ptr->opr = TacOpr::Mov;
				ptr->opd3 = TacOpd();
				if (ptr->opd1 == ptr->opd2) ptr->remove();
				return true;
			}
			if (ptr->opd2 == ptr->opd3) {
				ptr->opr = TacOpr::Mov;
				ptr->opd3 = TacOpd();
				ptr->opd2 = TacOpd::newImme(0);
				return true;
			}
			return false;
		case TacOpr::Mul:
			if (ptr->opd2.getType() == OpdType::Imme && ptr->opd3.getType() == OpdType::Reg)
				swap(ptr->opd2, ptr->opd3);
			if (ptr->opd2.getType() == OpdType::Reg && ptr->opd3.getType() == OpdType::Imme && ptr->opd3.getVal() == 0){
				ptr->opr = TacOpr::Mov;
				ptr->opd2 = TacOpd::newImme(0);
				ptr->opd3 = TacOpd();
				return true;
			}
			if (ptr->opd2.getType() == OpdType::Reg && ptr->opd3.getType() == OpdType::Imme && ptr->opd3.getVal() == 1){
				ptr->opr = TacOpr::Mov;
				ptr->opd3 = TacOpd();
				if (ptr->opd1 == ptr->opd2) ptr->remove();
				return true;
			}
			if (ptr->opd2.getType() == OpdType::Reg && ptr->opd3.getType() == OpdType::Imme && ptr->opd3.getVal() == -1){
				ptr->opr = TacOpr::Neg;
				ptr->opd3 = TacOpd();
				return true;
			}
			return false;
		case TacOpr::Div:
			if (ptr->opd2.getType() == OpdType::Reg && ptr->opd3.getType() == OpdType::Imme && ptr->opd3.getVal() == 1){
				ptr->opr = TacOpr::Mov;
				ptr->opd3 = TacOpd();
				if (ptr->opd1 == ptr->opd2) ptr->remove();
				return true;
			}
			if (ptr->opd2.getType() == OpdType::Reg && ptr->opd3.getType() == OpdType::Imme && ptr->opd3.getVal() == -1){
				ptr->opr = TacOpr::Neg;
				ptr->opd3 = TacOpd();
				return true;
			}
			return false;
		case TacOpr::Mod:
			if (ptr->opd2.getType() == OpdType::Reg && ptr->opd3.getType() == OpdType::Imme) {
				int imm = ptr->opd3.getVal();
				if (imm < 0) {
					ptr->opd3 = TacOpd::newImme(-imm);
					imm = -imm;
				}
				if (imm == 1) {
					ptr->opr = TacOpr::Mov;
					ptr->opd2 = TacOpd::newImme(0);
					ptr->opd3 = TacOpd();
					return true;
				} 
				if (imm == (1 << 31)) {
					ptr->opr = TacOpr::Mov;
					ptr->opd3 = TacOpd();
					return true;
				}
			}
			return false;
		case TacOpr::Addr:
			assert(ptr->opd3.getType() == OpdType::Imme);
			if (fbase(ptr->opd3.getVal()) == SpaceType::Abs) {
				ptr->opr = TacOpr::Mov;
				ptr->opd3 = TacOpd();
				return true;
			} 
			return false;
		default:
			return false;
	}
}

bool algebraic_optimization(FlowGraph &flowgraph){
	bool changed = false;
    for (auto bb : flowgraph.getBlocks()) {
        auto insts = bb->insts;
		vector<TacPtr> vec;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ) 
            vec.push_back(inst);
		for (auto inst : vec) 
			changed = algebraic_optimization(inst, flowgraph.fbase) || changed; 
    }
    return changed; 
}

bool load_store_elim(BBPtr &block){
	bool changed = false;
    Insts insts = block->insts;
	auto inst = insts.head->succ;
	if (inst == insts.tail) return false;
    while (inst->succ != insts.tail){
		if (inst->opr == TacOpr::Load && inst->succ->opr == TacOpr::Store){
			if (inst->opd1 == inst->succ->opd1 && inst->opd2 == inst->succ->opd2 && inst->opd3 == inst->succ->opd3){
				changed = true;
				inst->succ->remove();
			}
			else inst = inst->succ;
		}
		else if (inst->opr == TacOpr::Store && inst->succ->opr == TacOpr::Load){
			if (inst->opd2 == inst->succ->opd2 && inst->opd3 == inst->succ->opd3){
				changed = true;
				if (inst->opd1 == inst->succ->opd1) inst->succ->remove();
				else{
					inst->succ->opr = TacOpr::Mov;
					inst->succ->opd2 = inst->opd1;
					inst->succ->opd3 = TacOpd();
				}
			}
			inst = inst->succ;
		}
		else inst = inst->succ;
	}
	return changed;
}

bool load_store_elim(FlowGraph &flowgraph){
	bool changed = false;
    for (auto bb : flowgraph.getBlocks()){
        changed = load_store_elim(bb) || changed;
    }
    return changed;
}

bool peephole(FlowGraph &flowgraph){
    bool changed, this_changed = false;
    do{
        changed = false;
        changed = algebraic_optimization(flowgraph) || changed;
		changed = load_store_elim(flowgraph) || changed;
        this_changed = this_changed || changed;
    }while (changed);
    return this_changed; 
}