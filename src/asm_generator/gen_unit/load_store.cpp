#include "gen_unit.h"

LinePtr gen_ldr(const string &cond, const string &Rt, const string &label) {
    return std::make_shared<Line>("ldr" + cond + " " + Rt + ", " + label, 32);
}

LinePtr gen_ldr(const string &cond, const string &Rt, const string &Rn, int offset) {
    assert(-4096 < offset && offset < 4096);
    auto content = "ldr" + cond + " " + Rt + ", [" + Rn;
    if (offset) content = content + ", #" + std::to_string(offset);
    content = content + "]";
    return std::make_shared<Line>(content, 32);
}

LinePtr gen_ldr(const string &cond, const string &Rt, const string &Rn, const string &offset, const string &shift) {
    return std::make_shared<Line>("ldr" + cond + " " + Rt + ", [" + Rn + ", " + offset + ", " + shift + "]", 32);
}

LinePtr gen_ldr(const string &cond, const string &Rt, const string &Rn, const string &Rm) {
    return std::make_shared<Line>("ldr" + cond + " " + Rt + ", [" + Rn + ", " + Rm + "]", 32);
}


LinePtr gen_str(const string &cond, const string &Rt, const string &Rn, int offset) {
    assert(-4096 < offset && offset < 4096);
    auto content = "str" + cond + " " + Rt + ", [" + Rn;
    if (offset) content = content + ", #" + std::to_string(offset);
    content = content + "]";
    return std::make_shared<Line>(content, 32);
}

LinePtr gen_str(const string &cond, const string &Rt, const string &Rn, const string &Rm) {
    return std::make_shared<Line>("str" + cond + " " + Rt + ", [" + Rn + ", " + Rm + "]", 32);
}