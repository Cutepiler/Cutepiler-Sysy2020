#include "work_iteration.h"
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <deque>
#include <cassert>

using std::set;
using std::map;
using std::pair;
using std::make_pair;
using std::vector;
using std::deque;
using std::shared_ptr;
using std::cout;
using std::endl;

class Space_Set{
  private:
	int id;
  public:
	set<int> sp;
	TacOpd sr;
	Space_Set(int _id){
		id = _id;
		sp.clear();
	}
	int get_id();
};

map<TacOpd,bool> po;
map<pair<int,int>,bool> func_param;
map<TacOpd,Space_Set*> ss;
map<pair<int,int>,Space_Set*> fps;
deque<int> q;
deque<Space_Set*> qs;

bool is_pointer(const TacOpd &opd){
	if (po.find(opd) != po.end()) return true;
	else return false;
}

void new_pointer(TacOpd opd){
	//cout << "Pointer : " << opd.name() << endl;
	po[opd] = true;
	Space_Set* s = new Space_Set(opd.getId());
	ss[opd] = s;
	//if (ss.find(opd) != ss.end()) cout << "Yes" << endl;
}

void clear_flow(const FlowGraph &flowgraph){
	auto blocks = flowgraph.getBlocks();
    for (auto bb : blocks){
		Insts insts = bb->insts;
    	for (auto inst = insts.head->succ;inst != insts.tail;inst = inst->succ){
			for(int i=1;i<=4;i++){
				TacOpd opd = inst->getOpd(i);
				if (opd.getType() == OpdType::Reg){
					if (po.find(opd) != po.end()) po.erase(po.find(opd));
					if (ss.find(opd) != ss.end()) ss.erase(ss.find(opd));
				}
			}
    	}
    }
}

void calc_opd_type(const FlowGraph &flowgraph){
	//cout << "== Calc Operand Type,function is " << flowgraph.func->name << ",id is " << flowgraph.func->id << " ==" << endl;
	int func_id = flowgraph.func->id;
	q.clear();
	qs.clear();
	for(int i=0;i<flowgraph.func->paramId.size();i++){
		if (func_param[make_pair(func_id,i)]){
			TacOpd opd = TacOpd(flowgraph.func->paramId[i],OpdType::Reg);
			//cout << "zhaodaole! " << opd.name() << endl;
			if (po.find(opd) == po.end()){
				new_pointer(opd);
				ss[opd]->sr = opd;
			}
		}
	}
	bool changed;
	int tot = -1;
	do{
		//cout << "== Loop Start ==" << endl;
		changed = false;
		auto blocks = flowgraph.getBlocks();
    	for (auto bb : blocks){
			for (auto pa : bb->phi){
				if (po.find(pa.first) != po.end()) continue;
				bool flag = false;
				for (auto pa2 : pa.second){
					if (po.find(pa2.second) != po.end()){
						flag = true;
						break;
					}
				}
				if (flag){
					changed = true;
					new_pointer(pa.first);
				}
			}
			Insts insts = bb->insts;
    		for (auto inst = insts.head->succ;inst != insts.tail;inst = inst->succ){
				//cout << inst->to_string() << endl;
				if (inst->opr == TacOpr::Param) tot++;
				switch (inst->opr){
					case TacOpr::Neg :
					case TacOpr::Mov :
						if (po.find(inst->opd1) == po.end()){
							if (po.find(inst->opd2) != po.end()){
								changed = true;
								new_pointer(inst->opd1);
								ss[inst->opd1]->sr = ss[inst->opd2]->sr;
							}
						}
						break;
					case TacOpr::Add :
					case TacOpr::Sub :
						if (po.find(inst->opd1) == po.end()){
							if (po.find(inst->opd2) != po.end()){
								changed = true;
								new_pointer(inst->opd1);
								ss[inst->opd1]->sr = ss[inst->opd2]->sr;
							}
							if (po.find(inst->opd3) != po.end()){
								changed = true;
								new_pointer(inst->opd1);
								ss[inst->opd1]->sr = ss[inst->opd3]->sr;
							}
						}
						break;
					case TacOpr::Addr :
						if (po.find(inst->opd1) == po.end()){
							changed = true;
							new_pointer(inst->opd1);
						}
						break;
					case TacOpr::Param :
						if (po.find(inst->opd1) != po.end()) q.push_front(tot);
						break;
					case TacOpr::Call :
						assert(inst->opd1.getType() == OpdType::Imme);
						for(auto wz : q){
							func_param[make_pair(inst->opd1.getVal(),tot-wz)] = true;
						}
						q.clear();
					default :
						break;
				}
				if (inst->opr != TacOpr::Param) tot=-1;
    		}
    	}
	}while (changed);
}

void initialize_calc_type(const TacProg &pg,const int now_id,const FlowGraph &now_flow){
	func_param.clear();
	vector<shared_ptr<TacFunc>> tf;
	tf.clear();
	for (auto func : pg.funcs) tf.push_back(func);
	while (!tf.empty()){
		auto func = tf.back();
		tf.pop_back();
		if (func->id != now_id){
			auto function = func->copyMyself();
        	auto flow = FlowGraph(function->insts, function, pg.fbase);
			calc_opd_type(flow);
		}
		else calc_opd_type(now_flow);
    }
}

set<int> get_space(const TacOpd &opd){
	assert(is_pointer(opd));
	return ss[opd]->sp;
}

TacOpd get_source(const TacOpd &opd){
	if (ss.find(opd) != ss.end()) return ss[opd]->sr;
	else return TacOpd();
}

bool merge(Space_Set* s2,Space_Set* s){
	bool changed = false;
	for(auto x : s->sp){
		if (s2->sp.find(x) == s2->sp.end()){
			changed = true;
			s2->sp.insert(x);
		}
	}
	return changed;
}

void calc_pointer_space(const FlowGraph &flowgraph){
	//calc_opd_type(flowgraph);
	//cout << "== Calc Pointer Space,function is " << flowgraph.func->name << " ==" << endl;
	int func_id = flowgraph.func->id;
	q.clear();
	qs.clear();
	for(int i=0;i<flowgraph.func->paramId.size();i++){
		if (fps.find(make_pair(func_id,i)) != fps.end()){
			TacOpd opd = TacOpd(flowgraph.func->paramId[i],OpdType::Reg);
			merge(ss[opd],fps[make_pair(func_id,i)]);
		}
	}
	bool changed;
	int tot = -1;
	do{
		//cout << "== Loop Start ==" << endl;
		changed = false;
		auto blocks = flowgraph.getBlocks();
    	for (auto bb : blocks){
			for (auto pa : bb->phi){
				if (po.find(pa.first) == po.end()) continue;
				Space_Set* ns = new Space_Set(-1);
				for (auto pa2 : pa.second) 
					if (ss.find(pa2.second) != ss.end()) merge(ns,ss[pa2.second]);
				changed = merge(ss[pa.first],ns) || changed;
			}
			Insts insts = bb->insts;
    		for (auto inst = insts.head->succ;inst != insts.tail;inst = inst->succ){
				//cout << inst->to_string() << endl;
				if (inst->opr == TacOpr::Param) tot++;
				int x;
				switch (inst->opr){
					case TacOpr::Neg :
					case TacOpr::Mov :
						if (ss.find(inst->opd2) != ss.end()){
							changed = merge(ss[inst->opd1],ss[inst->opd2]) || changed;
						}
						break;
					case TacOpr::Add :
					case TacOpr::Sub :
						if (ss.find(inst->opd2) != ss.end()){
							changed = merge(ss[inst->opd1],ss[inst->opd2]) || changed;
						}
						if (ss.find(inst->opd3) != ss.end()){
							changed = merge(ss[inst->opd1],ss[inst->opd3]) || changed;
						}
						break;
					case TacOpr::Addr :
						x = inst->opd3.getVal();
						if (ss[inst->opd1]->sp.find(x) == ss[inst->opd1]->sp.end()){
							ss[inst->opd1]->sp.insert(x);
							changed = true;
						}
						break;
					case TacOpr::Param :
						if (ss.find(inst->opd1) != ss.end()){
							Space_Set* nss = ss[inst->opd1];
							q.push_front(tot);
							qs.push_front(nss);
						}
						break;
					case TacOpr::Call :
						if (tot != -1){
							assert(inst->opd1.getType() == OpdType::Imme);
							for(auto wz : q){
								if (fps.find(make_pair(inst->opd1.getVal(),tot-wz)) == fps.end()) fps[make_pair(inst->opd1.getVal(),tot-wz)] = new Space_Set(-1);
								merge(fps[make_pair(inst->opd1.getVal(),tot-wz)],qs.front());
								qs.pop_front();
							}
							q.clear();
						}
					default :
						break;
				}
				if (inst->opr != TacOpr::Param) tot=-1;
    		}
    	}
	}while (changed);
}

void initialize_calc_space(const TacProg &pg,const int now_id,const FlowGraph &now_flow){
	fps.clear();
	vector<shared_ptr<TacFunc>> tf;
	tf.clear();
	for (auto func : pg.funcs) tf.push_back(func);
	while (!tf.empty()){
		auto func = tf.back();
		tf.pop_back();
		if (func->id != now_id){
			auto function = func->copyMyself();
        	auto flow = FlowGraph(function->insts, function, pg.fbase);
			calc_pointer_space(flow);
		}
		else calc_pointer_space(now_flow);
    }
}

void display_pointer(const FlowGraph &flowgraph){
	cout << "== Function : " << flowgraph.func->name << " ==" << endl;
	cout << "== Display Pointer Start ==" << endl;
	set<TacOpd> op;
	op.clear();
	for(int i=0;i<flowgraph.func->paramId.size();i++){
		TacOpd opd = TacOpd(flowgraph.func->paramId[i],OpdType::Reg);
		if (po.find(opd) != po.end()){
			if (op.find(opd) == op.end()) op.insert(opd);
		}
	}
	auto blocks = flowgraph.getBlocks();
	for (auto bb : blocks){
		for (auto pa : bb->phi){
			TacOpd opd = pa.first;
			if (po.find(opd) != po.end()){
				if (op.find(opd) == op.end()) op.insert(opd);
			}
		}
        auto insts = bb->insts;
        for (auto inst = insts.head->succ; inst != insts.tail; inst = inst->succ) {
			for(int i=1;i<=4;i++){
				TacOpd opd = inst->getOpd(i);
				if (po.find(opd) != po.end()){
					if (op.find(opd) == op.end()) op.insert(opd);
				}
			}
		}
	}
	set<int> sp;
	for (auto opd : op){
		cout << opd.name() << " : ";
		sp = get_space(opd);
		for (auto x : sp) cout << x << " ";
		cout << endl;
	}
	cout << "== Display Pointer End ==" << endl;
}

void display_pointer(const TacProg &pg,const int now_id,const FlowGraph &now_flow){
	//for (auto poi : po) cout << "Final Pointer : " << poi.first.name() << endl;
	for (auto func : pg.funcs){
		if (func->id != now_id){
			auto function = func->copyMyself();
			auto flow = FlowGraph(function->insts, function, pg.fbase);
			display_pointer(flow);
		}
		else display_pointer(now_flow);
	}
}

void work_iteration(const TacProg &pg,const FlowGraph &flowgraph){
	po.clear();
	ss.clear();
	initialize_calc_type(pg,flowgraph.func->id,flowgraph);
	initialize_calc_space(pg,flowgraph.func->id,flowgraph);
	clear_flow(flowgraph);
	calc_opd_type(flowgraph);
	calc_pointer_space(flowgraph);
	//display_pointer(flowgraph);
}

void work_iteration(const TacProg &pg){
	po.clear();
	ss.clear();
	//initialize_calc_type(pg);
	//initialize_calc_space(pg);
}