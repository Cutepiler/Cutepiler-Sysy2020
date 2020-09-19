#include "convert.h"
#include "../env/env.h"

using std::make_shared;
using std::function;
using std::endl;

void test_branch_requirement(FlowGraph &flowgraph){
	for (auto bb : flowgraph.getBlocks()) {
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ) {
			auto pre = inst->pred;
            switch (inst->opr){
				case TacOpr::Beqz :
				case TacOpr::Bnez :
					assert(inst->opd1.getType() == OpdType::Reg);
					assert(inst != insts.head->succ);
					switch (pre->opr){
						case TacOpr::Gt :
						case TacOpr::Lt :
						case TacOpr::Ge :
						case TacOpr::Le :
						case TacOpr::Eq :
						case TacOpr::Ne :
						case TacOpr::And :
						case TacOpr::Or :
							assert(pre->opd1 == inst->opd1);
							break;
						default :
							assert(false);
							break;
					}
					break;
				default :
					break;
			}
        }
    }
}

void convert_store_opd1_to_register(FlowGraph &flowgraph){
	for (auto bb : flowgraph.getBlocks()) {
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ){
			auto pre = inst->pred;
			auto nxt = inst->succ;
			if (inst->opr == TacOpr::Store && inst->opd1.getType() == OpdType::Imme) {
				if (nxt->opr == TacOpr::Store && nxt->opd1.getType() == OpdType::Imme) {
					TacOpd reg1 = TacOpd::newReg(), reg2 = TacOpd::newReg();
					auto p = make_shared<Tac>(TacOpr::Mov, reg1, inst->opd1);
					auto q = make_shared<Tac>(TacOpr::Mov, reg2, nxt->opd1);
					pre->insert(q), pre->insert(p);
					inst->opd1 = reg1;
					nxt->opd1 = reg2;
				} else {
					TacOpd reg = TacOpd::newReg();
					auto p = make_shared<Tac>(TacOpr::Mov, reg, inst->opd1);
					pre->insert(p);
					inst->opd1 = reg;
				}
			}
        }
    }
}

void merge_stores(FlowGraph &flowgraph)
{
	auto blocks = flowgraph.getBlocks();
	for (auto block : blocks) {
		for (auto inst = block->insts.head->succ; inst != block->insts.tail; inst = inst->succ) {
			if (inst->opr != TacOpr::Store) continue;
			auto &opd = inst->opd1;
			if (opd.getType() == OpdType::Reg) continue;
			auto reg = TacOpd::newReg();
			inst->pred->insert(make_shared<Tac>(TacOpr::Mov, reg, opd));
			auto cur = inst->succ;
			while (cur->opr == TacOpr::Store && cur->opd1 == opd) {
				cur->opd1 = reg;
				cur = cur->succ;
			}
			opd = reg;
			inst = cur->pred;
		}
	}
}

void convert_param_opd1_to_register(FlowGraph &flowgraph){
	for (auto bb : flowgraph.getBlocks()) {
        auto insts = bb->insts;
		auto last = insts.head;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ){
			auto pre = inst->pred;
            switch (inst->opr){
				case TacOpr::Param :
					if (/* inst->opd2.getType() == OpdType::Imme &&  inst->opd2.getVal() >= 5 && */ inst->opd1.getType() == OpdType::Imme){
						TacOpd reg = TacOpd::newReg();
						auto p = make_shared<Tac>(TacOpr::Mov, reg, inst->opd1);
						last->insert(p);
						inst->opd1 = reg;
					}
					break;
				default :
					last = inst;
					break;
			}
        }
    }
}

void convert_mul_opd_to_register(FlowGraph &flowgraph){
	for (auto bb : flowgraph.getBlocks()) {
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ){
			auto pre = inst->pred;
            switch (inst->opr){
				case TacOpr::Mul:
				case TacOpr::MLA:
				case TacOpr::MLS:
					for (int i = 2; i <= 4; i++) {
						auto &opd = inst->getOpd(i);
						if (opd.getType() == OpdType::Imme){
							TacOpd reg = TacOpd::newReg();
							auto p = make_shared<Tac>(TacOpr::Mov, reg, opd);
							pre->insert(p);
							opd = reg;
						}
					}
					break;
				case TacOpr::AddLS:
					for (int i = 2; i <= 3; i++) {
						auto &opd = inst->getOpd(i);
						if (opd.getType() == OpdType::Imme){
							TacOpd reg = TacOpd::newReg();
							auto p = make_shared<Tac>(TacOpr::Mov, reg, opd);
							pre->insert(p);
							opd = reg;
						}
					}
					assert(inst->opd4.getType() == OpdType::Imme);
					break;
				default :
					break;
			}
        }
    }
}

void limit_offset(FlowGraph &flowgraph){
	for (auto bb : flowgraph.getBlocks()) {
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ){
			auto pre = inst->pred;
            switch (inst->opr){
				case TacOpr::Load :
				case TacOpr::Store :
				case TacOpr::Addr :
					if (inst->opd2.getType() == OpdType::Imme){
						if (inst->opd2.getVal() >= 4096 || inst->opd2.getVal() <= -4096){
							TacOpd reg = TacOpd::newReg();
							auto p = make_shared<Tac>(TacOpr::Mov, reg, inst->opd2);
							pre->insert(p);
							inst->opd2 = reg;
						}
					}
					break;
				case TacOpr::LoadAddLSR:
				case TacOpr::LoadAddASR:
				case TacOpr::LoadAddASL:
					assert(inst->opd3.getType() == OpdType::Reg);
					break;
				default :
					break;
			}
        }
    }
}

void call_transformation(FlowGraph &flowgraph){
	for (auto bb : flowgraph.getBlocks()) {
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ){
            switch (inst->opr){
				case TacOpr::Call :
					if (inst->opd2.getType() == OpdType::Reg){
						TacOpd reg = TacOpd::newReg();
						auto p = make_shared<Tac>(TacOpr::Mov, inst->opd2, reg);
						inst->opd2 = reg;
						inst->insert(p);
					}
					break;
				default :
					break;
			}
        }
    }
}

bool is_valid_imme(unsigned int val){
	for(int i = 0;i < 16;i++){
		unsigned int num;
		int t;
		num = val << (2 * i);
		if (i == 0) t = 0;else t = 32 - 2 * i;
		num |= (val & (0xffffffff << t)) >> t;
		if (num <= 0x000000ff) return true;
	}
	return false;
}

void make_immediate_valid(FlowGraph &flowgraph){
	for (auto bb : flowgraph.getBlocks()) {
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ){
			auto pre = inst->pred;
            switch (inst->opr){
				case TacOpr::Ret :
				case TacOpr::Param :
				case TacOpr::Call :
				case TacOpr::Beqz :
				case TacOpr::Bnez :
					if (inst->opd1.getType() == OpdType::Imme && !is_valid_imme(inst->opd1.getVal())){
						TacOpd reg = TacOpd::newReg();
						auto p = make_shared<Tac>(TacOpr::Mov, reg, inst->opd1);
						pre->insert(p);
						inst->opd1 = reg;
					}
					break;
				case TacOpr::Not :
				case TacOpr::Neg :
				case TacOpr::Load :
				case TacOpr::Store :
				case TacOpr::Addr :
					if (inst->opd2.getType() == OpdType::Imme && !is_valid_imme(inst->opd2.getVal())){
						TacOpd reg = TacOpd::newReg();
						auto p = make_shared<Tac>(TacOpr::Mov, reg, inst->opd2);
						pre->insert(p);
						inst->opd2 = reg;
					}
					break;
				case TacOpr::LoadAdd:
					if (inst->opd3.getType() == OpdType::Imme && 
						(!is_valid_imme(inst->opd3.getVal()) || inst->opd3.getVal() < -4095 || inst->opd3.getVal() > 4095)) {
						TacOpd reg = TacOpd::newReg();
						auto p = make_shared<Tac>(TacOpr::Mov, reg, inst->opd3);
						pre->insert(p);
						inst->opd3 = reg;
					}
					break;
				case TacOpr::Add :
				case TacOpr::Sub :
				case TacOpr::Mul :
				case TacOpr::Div :
				case TacOpr::Mod :
				case TacOpr::Gt :
				case TacOpr::Lt :
				case TacOpr::Ge :
				case TacOpr::Le :
				case TacOpr::Eq :
				case TacOpr::Ne :
				case TacOpr::And :
				case TacOpr::Or :
				case TacOpr::BIC:
					if (inst->opd2.getType() == OpdType::Imme && !is_valid_imme(inst->opd2.getVal())){
						TacOpd reg = TacOpd::newReg();
						auto p = make_shared<Tac>(TacOpr::Mov, reg, inst->opd2);
						pre->insert(p);
						inst->opd2 = reg;
					}
					if (inst->opd3.getType() == OpdType::Imme && !is_valid_imme(inst->opd3.getVal())){
						TacOpd reg = TacOpd::newReg();
						auto p = make_shared<Tac>(TacOpr::Mov, reg, inst->opd3);
						pre->insert(p);
						inst->opd3 = reg;
					}
					break;
				default :
					break;
			}
        }
    }
}

void load_store_limit(FlowGraph &flowgraph){
	for (auto bb : flowgraph.getBlocks()) {
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ){
			auto pre = inst->pred;
            switch (inst->opr){
				case TacOpr::Load :
				case TacOpr::Store :
					if (flowgraph.fbase(inst->opd3.getVal()) == SpaceType::Data || flowgraph.fbase(inst->opd3.getVal()) == SpaceType::BSS){
						TacOpd reg = TacOpd::newReg();
						auto p = make_shared<Tac>(TacOpr::Addr, reg, inst->opd2, inst->opd3);
						pre->insert(p);
						inst->opd2 = reg;
						inst->opd3 = TacOpd::newImme(0);
					}
					break;
				default :
					break;
			}
        }
    }
}

void addr_transformation(FlowGraph &flowgraph){
	auto bb = flowgraph.getStartBlock();
	auto inst = bb->insts.head;
	TacOpd data_start = TacOpd::newReg();
	auto p = make_shared<Tac>(TacOpr::Addr, data_start, TacOpd::newImme(0), TacOpd::newImme(DATA_START));
	TacOpd bss_start = TacOpd::newReg();
	auto q = make_shared<Tac>(TacOpr::Addr, bss_start, TacOpd::newImme(0), TacOpd::newImme(BSS_START));
	bool has_bss = false, has_data = false;
	for (auto bb : flowgraph.getBlocks()) {
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ){
			auto pre = inst->pred;
            switch (inst->opr){
				case TacOpr::Addr :
					if (flowgraph.fbase(inst->opd3.getVal()) == SpaceType::BSS) has_bss = true;
					else if (flowgraph.fbase(inst->opd3.getVal()) == SpaceType::Data) has_data = true;
					break;
				default :
					break;
			}
        }
    }
	for (auto bb : flowgraph.getBlocks()) {
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ){
			auto pre = inst->pred;
            switch (inst->opr){
				case TacOpr::Addr :
					if (flowgraph.fbase(inst->opd3.getVal()) == SpaceType::BSS){
						inst->opd3 = bss_start;
						inst->opr = TacOpr::Add;
					}
					else if (flowgraph.fbase(inst->opd3.getVal()) == SpaceType::Data){
						inst->opd3 = data_start;
						inst->opr = TacOpr::Add;
					}
					break;
				default :
					break;
			}
        }
    }
	if (has_bss) inst->insert(q);
	if (has_data) inst->insert(p);
}

void convert_abs_offset_to_register(FlowGraph &flowgraph){
	for (auto bb : flowgraph.getBlocks()) {
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ){
			auto pre = inst->pred;
            switch (inst->opr){
				case TacOpr::Load :
				case TacOpr::Store :
					if (flowgraph.fbase(inst->opd3.getVal()) == SpaceType::Abs && inst->opd2.getType() == OpdType::Imme){
						TacOpd reg = TacOpd::newReg();
						auto p = make_shared<Tac>(TacOpr::Mov, reg, inst->opd2);
						pre->insert(p);
						inst->opd2 = reg;
					}
					break;
				default :
					break;
			}
        }
    }
}

void convert_tac(FlowGraph &flowgraph){
	merge_stores(flowgraph);
	test_branch_requirement(flowgraph);
	convert_store_opd1_to_register(flowgraph);
	convert_param_opd1_to_register(flowgraph);
	convert_mul_opd_to_register(flowgraph);
	limit_offset(flowgraph);
	//call_transformation(flowgraph);
	make_immediate_valid(flowgraph);
	load_store_limit(flowgraph);
	addr_transformation(flowgraph);
	convert_abs_offset_to_register(flowgraph);
}