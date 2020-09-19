#include "opt.h"
#include "help.h"
#include "gen_unit/gen_unit.h"

void opt_ldr_str_simple(Lines &prog) {
    for (auto inst = prog.head->succ; inst != prog.tail; inst = inst->succ) {
        if (get_inst_type(inst->content) == "str" && get_inst_type(inst->succ->content) == "ldr") {
            auto stropd = get_opd_list(inst->content);
            auto ldropd = get_opd_list(inst->succ->content);
            string strpos, ldrpos;
            for (int i = 1; i < stropd.size(); ++i) strpos += stropd[i];
            for (int i = 1; i < ldropd.size(); ++i) ldrpos += ldropd[i];
            if (strpos == ldrpos) {
                auto des = ldropd[0], ori = stropd[0];
                inst->succ->remove();
                if (des != ori) {
                    inst->insert(gen_mov(false, "", des, ori));
                }
            }
        }
    }
}