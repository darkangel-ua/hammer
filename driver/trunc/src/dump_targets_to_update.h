#if !defined(h_f2593db8_2e4f_41a5_aa99_d77e3a8706e6)
#define h_f2593db8_2e4f_41a5_aa99_d77e3a8706e6

#include <iosfwd>
#include <hammer/core/build_node.h>
#include <hammer/core/build_environment.h>

void dump_targets_to_update(std::ostream& os, 
                            const hammer::build_node::nodes_t& nodes,
                            hammer::build_environment& environment);

#endif //h_f2593db8_2e4f_41a5_aa99_d77e3a8706e6
