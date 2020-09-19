#include "gen_unit.h"
LinePtr gen_label(const string &label_name, int label_id){
    return std::make_shared<Line>(label_name + std::to_string(label_id),  32);
}
