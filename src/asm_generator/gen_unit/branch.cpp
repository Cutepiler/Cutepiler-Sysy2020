#include "gen_unit.h"

LinePtr gen_b(const string &cond, const string &label) {
    return std::make_shared<Line>("b" + cond + " " + label, 32);
}