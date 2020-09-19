#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <set>
#include <vector>
/*
    operations of TAC instructions
*/
enum class TacOpr {
    Not,
    Neg,
    Mov,  // Not, Neg: Unary operator Mov: assignment, corresponding to
          // mov/movw/movt/LDR/STR
    Add,
    Sub,
    Mul,
    Div,
    Mod,  // Binary Arithmetic operator, Mod don't corresponds to instruction, should
          // be emulated by div, mul and sub
    Gt,
    Lt,
    Ge,
    Le,
    Eq,
    Ne,  // Binary comparison operator, corresponding to 'cmp'
    And,
    Or,  // binary logical operator, 'and', 'oor'
    Branch,
    Beqz,
    Bnez,  // corresponding to B, the flag should be prepared
    Param,
    Call,
    Ret,   // function call
    Labl,  // generate a label
    Load,
    Store,

    LoadSpASL,
    LoadSpASR,
    LoadSpLSR,

    LoadAdd,
    LoadAddASL,
    LoadAddASR,
    LoadAddLSR,

    Addr,  // array

    /* Special Instructions For Optimization */

    ASL,   // left shift, simulate x |-> y * 2^k 
    ASR,   // right shift, simulate x |-> y * 2^k
    LSR,   // logic right shift 

    MLA,   // a <- b * c + d
    MLS,   // a <- b * c - d

    CMP,

    AddLS,   // add a, b, c, #lsl d (imm)
    SubLS,   // sub a, b, c, #lsl d (imm)
    RsbLS,   // rsb a, b, c, #lsl d (imm)

    AddLSR,   // add a, b, c, logic right shift d (imm)
    RsbASR,   // rsb a, b, c, arithmetic right shift d (imm)

    BIC,      // bic a, b, c (in arm)
    
    Smmul,   // smmul a, b, c (in arm) (high 32-bit of multiplication) 
    _Head,
    _Tail,  // meaningless, only for tac lists
};

/*
    type of TAC operations
*/
enum class OpdType { Reg, Label, Imme, Null };

const int ORIGIN_BASE = 0;
const int SSA_SUB_BASE = 20;
const int SPILL_SUB_BASE = 25;
const int INTERPRETER_BASE = 28;

/*
    oprands of TAC instructions
*/
class TacOpd {
private:
    int id;  // the id of a register/label, or value of a immediate
    OpdType type;

public:
    TacOpd() : id(0), type(OpdType::Null) {}
    TacOpd(int id, OpdType type) : id(id), type(type) {}

    bool empty() const { return type == OpdType::Null; }

    bool operator==(const TacOpd &opd) const {
        if (empty() || opd.empty()) return false;
        return id == opd.id && type == opd.type;
    }

    bool operator!=(const TacOpd &opd) const { return !(*this == opd); }

    int getId() const {  // Id: for register/label
        assert(type == OpdType::Reg || type == OpdType::Label);
        return id;
    }

    int getVal() const {  // Val: for immediate
        assert(type == OpdType::Imme);
        return id;
    }

    void setId(int id) {
        this->id = id;
    }

    void addSubscript(int subscript) { id |= subscript << SSA_SUB_BASE; }

    TacOpd getOrigin() { return TacOpd(id & ((1 << SSA_SUB_BASE) - 1), OpdType::Reg); }

    OpdType getType() const { return type; }

    std::string name() const {
        std::string s;
        switch (type) {
            case OpdType::Reg:
                s = std::to_string(id & ((1 << SSA_SUB_BASE) - 1));
                if (id >> SSA_SUB_BASE) s += "_" + std::to_string((id & ((1 << SPILL_SUB_BASE) - 1)) >> SSA_SUB_BASE);
                if (id >> SPILL_SUB_BASE) s += "_" + std::to_string(id >> SPILL_SUB_BASE);
                return "R" + s;
            case OpdType::Label:
                return "L" + std::to_string(id);
            case OpdType::Imme:
                return std::to_string(id);
            default:
                return "";
        }
    }

    bool operator<(const TacOpd &opd) const {
        if (type != opd.type) return type < opd.type;
        return id < opd.id;
    }

    static TacOpd newReg() {
        static int regCounter = 0;
        ++regCounter;
        return TacOpd(regCounter, OpdType::Reg);
    }

    static TacOpd newLabel() {
        static int labelCounter = 0;
        ++labelCounter;
        return TacOpd(labelCounter, OpdType::Label);
    }

    static TacOpd newImme(int val) { return TacOpd(val, OpdType::Imme); }
};

/*
    TAC instructions
*/
struct Tac;
using TacPtr = std::shared_ptr<Tac>;
struct Tac {
    TacOpr opr;
    TacOpd opd1, opd2, opd3;
    TacOpd opd4;
    // opd4, only for MLA and MLS
    TacPtr pred, succ;
    TacOpr cond;

    Tac(TacOpr opr, TacOpd opd1 = TacOpd(), TacOpd opd2 = TacOpd(),
        TacOpd opd3 = TacOpd(), TacOpd opd4 = TacOpd())
        : opr(opr), opd1(opd1), opd2(opd2), opd3(opd3), opd4(opd4), cond(TacOpr::_Head) {
    }

    TacOpd &getOpd(int id) {
        switch (id) {
            case 1:
                return opd1;
            case 2:
                return opd2;
            case 3:
                return opd3;
            case 4:
                return opd4;
            default:
                assert(false);
        }
    }
    std::string to_string() const {
        switch (opr) {
            case TacOpr::Not:
                return "Not " + opd1.name() + ", " + opd2.name();
            case TacOpr::Neg:
                return "Neg " + opd1.name() + ", " + opd2.name();
            case TacOpr::Add:
                return "Add " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::Sub:
                return "Sub " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::Mul:
                return "Mul " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::Div:
                return "Div " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::Mod:
                return "Mod " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::Gt:
                return "Gt " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::Lt:
                return "Lt " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::Ge:
                return "Ge " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::Le:
                return "Le " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::Eq:
                return "Eq " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::Ne:
                return "Ne " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::And:
                return "And " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::Or:
                return "Or " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::Branch:
                return "Branch " + opd1.name();
            case TacOpr::Beqz:
                return "Beqz " + opd1.name() + ", " + opd2.name();
            case TacOpr::Bnez:
                return "Benz " + opd1.name() + ", " + opd2.name();
            case TacOpr::Param:
                return "Param " + opd1.name();
            case TacOpr::Call:
                return "Call " + opd1.name() + ", " +
                       opd2.name();  // opd1: func ID opd2: return value
            case TacOpr::Ret:
                return "Ret " +
                       opd1.name();  // put opd1 in r0  genrerate code: mov from
                                     // virtual register to real register, mov
                                     // from virtual register to virtual register
            case TacOpr::Labl:
                return "Labl " + opd1.name();
            case TacOpr::Mov:
                return "Mov " + opd1.name() + ", " + opd2.name();
            case TacOpr::Load:
                return "Load " + opd1.name() + ", " + opd2.name() + ", " +
                       opd3.name();  // opd3: space_id(imm), 0 stands for absolute
                                     // address opd2: offset opd1: virtual
                                     // register, must implement as a real
                                     // register
            case TacOpr::LoadSpASL:
                return "LoadSp " + opd1.name() + ", " + opd2.name() + ", #lsl " + opd3.name();
            case TacOpr::LoadSpASR:
                return "LoadSp " + opd1.name() + ", " + opd2.name() + ", #asr " + opd3.name();
            case TacOpr::LoadSpLSR:
                return "LoadSp " + opd1.name() + ", " + opd2.name() + ", #lsr " + opd3.name();
            case TacOpr::LoadAdd:
                return "Load " + opd1.name() + ", " + opd2.name() + " + " + opd3.name();
            case TacOpr::LoadAddLSR:
                return "Load " + opd1.name() + ", " + opd2.name() + " + " + opd3.name() + " #lsr " + opd4.name();
            case TacOpr::LoadAddASL:
                return "Load " + opd1.name() + ", " + opd2.name() + " + " + opd3.name() + " #lsl " + opd4.name();
            case TacOpr::LoadAddASR:
                return "Load " + opd1.name() + ", " + opd2.name() + " + " + opd3.name() + " #asr " + opd4.name();
            case TacOpr::Store:
                return "Store " + opd1.name() + ", " + opd2.name() + ", " +
                       opd3.name();
            case TacOpr::Addr:
                return "Addr " + opd1.name() + ", " + opd2.name() + ", " +
                       opd3.name();
            case TacOpr::ASL:
                return "ALShift " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::ASR:
                return "ARShift " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::LSR:
                return "LRShift " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::MLA:
                return "MLA " + opd1.name() + ", " + opd2.name() + ", " + opd3.name() + ", " + opd4.name();
            case TacOpr::MLS:
                return "MLS " + opd1.name() + ", " + opd2.name() + ", " + opd3.name() + ", " + opd4.name();
            case TacOpr::AddLS:
                return "Add " + opd1.name() + ", " + opd2.name() + ", " + opd3.name() + " #lsl " + opd4.name();
            case TacOpr::SubLS:
                return "Sub " + opd1.name() + ", " + opd2.name() + ", " + opd3.name() + " #lsl " + opd4.name();
            case TacOpr::RsbLS:
                return "Rsb " + opd1.name() + ", " + opd2.name() + ", " + opd3.name() + " #lsl " + opd4.name();
            case TacOpr::AddLSR:
                return "Add " + opd1.name() + ", " + opd2.name() + ", " + opd3.name() + " #lsr " + opd4.name();
            case TacOpr::RsbASR:
                return "Rsb " + opd1.name() + ", " + opd2.name() + ", " + opd3.name() + " #asr " + opd4.name();
            case TacOpr::Smmul:
                return "Smmul " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::BIC:
                return "Bic " + opd1.name() + ", " + opd2.name() + ", " + opd3.name();
            case TacOpr::CMP:
                return "Cmp " + opd1.name() + ", " + opd2.name();
            default:
                return "";
        }
    }

    bool isDefFast(int id) const {
        switch (id) {
            case 1:
                return _isDef1;
            case 2:
                return _isDef2;
            case 3:
                return _isDef3;
            case 4:
                return _isDef4;
            default:
                return false;
        }
    }

    bool isUseFast(int id) const {
        switch (id) {
            case 1:
                return _isUse1;
            case 2:
                return _isUse2;
            case 3:
                return _isUse3;
            case 4:
                return _isUse4;
            default:
                return false;
        }
    }

    bool isDef(int id) {
        computeDefUse();
        return isDefFast(id); 
    }

    bool isUse(int id) {
        computeDefUse();
        return isUseFast(id); 
    }

    void insert(TacPtr s) {
        auto ss = succ;
        s->pred = ss->pred;
        s->succ = ss;
        succ = ss->pred = s;
    }

    void remove() {
        auto p = pred, s = succ;
        pred = succ = nullptr;
        p->succ = s;
        s->pred = p;
    }

    void insert(const Tac &inst) { insert(std::make_shared<Tac>(inst)); }

    void insert(TacPtr head, TacPtr tail) {
        head->pred->succ = tail->succ;
        tail->succ->pred = head->pred;
        auto s = succ, p = succ->pred;
        p->succ = head;
        head->pred = p;
        s->pred = tail;
        tail->succ = s;
    }

    // compute liveness information using liveOut of the inst
    void computeLiveness(const std::set<TacOpd> &liveOut) {
        this->liveOut = liveOut; 
        this->liveIn = liveOut;
        computeDefUse(); 
        for (int i = 1; i <= 4; i++)
            if (isDefFast(i))
                this->liveIn.erase(getOpd(i)); 
        for (int i = 1; i <= 4; i++)
            if (isUseFast(i))
                this->liveIn.insert(getOpd(i));
    }

    void cleanLive()
    {
        liveIn.clear();
        liveOut.clear();
    }

    // compute variable defined in the inst
    std::set<TacOpd> getDefs() {
        computeDefUse();
        std::set<TacOpd> defs;  
        for (int i = 1; i <= 4; i++)
            if (isDefFast(i))
                defs.insert(getOpd(i));
        return defs; 
    }

    // compute variables used in the inst
    std::set<TacOpd> getUses() {
        computeDefUse(); 
        std::set<TacOpd> uses;
        for (int i = 1; i <= 4; i++)
            if (isUseFast(i))
                uses.insert(getOpd(i));
        return uses; 
    }

    std::set<TacOpd> getLiveIn() const {
        return liveIn; 
    }

    std::set<TacOpd> getLiveOut() const {
        return liveOut; 
    }
 
    bool isLiveIn(const TacOpd &opd) const {
        return liveIn.count(opd);
    }

    bool isLiveOut(const TacOpd &opd) const {
        return liveOut.count(opd); 
    }

private:
    bool _isDef1, _isDef2, _isDef3, _isDef4;
    bool _isUse1, _isUse2, _isUse3, _isUse4;

    std::set<TacOpd> liveIn;
    std::set<TacOpd> liveOut;

    void computeDefUse() {
        _isDef1 = _isDef2 = _isDef3 = _isDef4 = false; 
        _isUse1 = _isUse2 = _isUse3 = _isUse4 = false; 
        switch (opr) {
            case TacOpr::Not:
            case TacOpr::Neg:
            case TacOpr::Mov:
            case TacOpr::Addr:
            case TacOpr::Load:
            case TacOpr::LoadSpASL:
            case TacOpr::LoadSpASR:
            case TacOpr::LoadSpLSR:
                _isDef1 = opd1.getType() == OpdType::Reg;
                _isUse2 = opd2.getType() == OpdType::Reg;
                break;
            case TacOpr::Add:
            case TacOpr::Sub:
            case TacOpr::Mul:
            case TacOpr::Div:
            case TacOpr::Mod:
            case TacOpr::Gt:
            case TacOpr::Lt:
            case TacOpr::Ge:
            case TacOpr::Le:
            case TacOpr::Eq:
            case TacOpr::Ne:
            case TacOpr::And:
            case TacOpr::Or:
            case TacOpr::ASL:
            case TacOpr::ASR:
            case TacOpr::LSR:
            case TacOpr::Smmul:
            case TacOpr::BIC:
            case TacOpr::LoadAdd:
            case TacOpr::LoadAddLSR:
            case TacOpr::LoadAddASL:
            case TacOpr::LoadAddASR:
                _isDef1 = opd1.getType() == OpdType::Reg;
                _isUse2 = opd2.getType() == OpdType::Reg;
                _isUse3 = opd3.getType() == OpdType::Reg;
                break;
            case TacOpr::Beqz:
            case TacOpr::Bnez:
            case TacOpr::Param:
            case TacOpr::Ret:
                _isUse1 = opd1.getType() == OpdType::Reg;
                break;
            case TacOpr::Call:
                _isUse1 = opd1.getType() == OpdType::Reg;
                _isDef2 = opd2.getType() == OpdType::Reg;
                break;
            case TacOpr::Store:
                _isUse1 = opd1.getType() == OpdType::Reg;
                _isUse2 = opd2.getType() == OpdType::Reg;
                break;
            case TacOpr::MLA:
            case TacOpr::MLS:
            case TacOpr::AddLS:
            case TacOpr::SubLS:
            case TacOpr::RsbLS:
            case TacOpr::AddLSR:
            case TacOpr::RsbASR:
                _isDef1 = opd1.getType() == OpdType::Reg;
                _isUse2 = opd2.getType() == OpdType::Reg;
                _isUse3 = opd3.getType() == OpdType::Reg;
                _isUse4 = opd4.getType() == OpdType::Reg;
                break;
            case TacOpr::CMP:
                _isUse1 = opd1.getType() == OpdType::Reg;
                _isUse2 = opd2.getType() == OpdType::Reg;
                break;
            default:
                break;
        }
    }
};

/*
    a list of TAC instructions
*/
struct Insts {
    TacPtr head, tail;

    Insts() {
        head = std::make_shared<Tac>(TacOpr::_Head);
        tail = std::make_shared<Tac>(TacOpr::_Tail);
        head->succ = tail;
        tail->pred = head;
    }

    bool empty() const { return head->succ == tail; }

    void push_back(TacPtr inst) { tail->pred->insert(inst); }
    void push_front(TacPtr inst) { head->insert(inst); }
    void push_back(Insts insts) {
        if (insts.empty()) return;
        tail->pred->insert(insts.head->succ, insts.tail->pred);
    }
    void push_front(Insts insts) {
        if (insts.empty()) return;
        head->insert(insts.head->succ, insts.tail->pred);
    }

    static Insts cut(TacPtr first, TacPtr last) {
        Insts result;
        if (last->succ == first) return result;
        auto p = first->pred, s = last->succ;
        p->succ = s;
        s->pred = p;
        first->pred = result.head;
        last->succ = result.tail;
        result.head->succ = first;
        result.tail->pred = last;
        return result;
    }

    struct Iterator {
        Iterator(TacPtr ptr) : ptr(ptr) {}
        Iterator operator++() {
            ptr = ptr->succ;
            return *this;
        }
        bool operator!=(const Iterator &other) const { return ptr != other.ptr; }
        const Tac &operator*() const { return *ptr; }

    private:
        TacPtr ptr;
    };

    Iterator begin() { return Iterator(head->succ); }
    Iterator begin() const { return Iterator(head->succ); }
    Iterator end() { return Iterator(tail); }
    Iterator end() const { return Iterator(tail); }
};

/*
    functions of TAC
*/
struct TacFunc {
    std::string name;
    Insts insts;
    std::vector<int> paramId;
    bool is_pure;
    int id;
    int length;
    std::map<TacOpd, int> reg;
    bool hasLoop;

    TacFunc(const std::string &name, const std::vector<int> &paramId, int id,
            int length)
        : name(name), paramId(paramId), id(id), length(length), hasLoop(true) {
            is_pure = false;
        }
    
    std::shared_ptr<TacFunc> copyMyself() const;
};

struct TacData {
    std::vector<int> store;
    std::vector<std::pair<int, int>> initRegs;  // (regID, value)
    int length;
};

struct TacBSS {
    int length;  // maybe we need some extra labels in BSS to speed up addressing?
};

enum class SpaceType {
    Stack,
    Data,
    BSS,
    Abs,
};

using TBase = std::function<SpaceType(int)>;

/*
    a TAC program
*/
struct TacProg {
    std::list<std::shared_ptr<TacFunc>> funcs;
    std::map<int,int> space_length; // in words
    std::map<int,int> space_base;   // in words
    TacData data;
    TacBSS bss;
    TBase fbase;

    TacProg() {}
    TacProg(const std::list<std::shared_ptr<TacFunc>> &funcs, const TacData &data,
            const TacBSS &bss, const TBase &fbase)
        : funcs(funcs), data(data), bss(bss), fbase(fbase) {}
};

std::ostream &operator<<(std::ostream &os, const TacFunc &func);
std::ostream &operator<<(std::ostream &os, const TacData &data);
std::ostream &operator<<(std::ostream &os, const TacBSS &bss);
std::ostream &operator<<(std::ostream &os, const TacProg &prog);
std::ostream &operator<<(std::ostream &os, const TacOpd &opd);
/*
    interpreter error code
*/
const int OK = 0;
const int MISS_FUNCTION = 1;
const int STACK_ERROR = 2;
const int OPR_ERROR = 3;

/*
    interpret the TAC instructions
*/
int interpret(const TacProg &prog, std::istream &is, std::ostream &os);
int new_inte(const TacProg &prog, std::istream &is, std::ostream &os);

void name_discretization(TacProg &prog);